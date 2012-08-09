/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <U2Algorithm/BitsTable.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AppResources.h>
#include <U2Core/Timer.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/GAutoDeleteList.h>

#include "GenomeAlignerIndex.h"
#include "GenomeAlignerTask.h"
#include "SuffixSearchCUDA.h"

#include <time.h>

#include "GenomeAlignerFindTask.h"

namespace U2 {

template <typename T>
QString numArrToStr(T* arr, int size, boolean hex = false) {
	if(NULL == arr) {
		return QString("null");
	}

	QString ret("");
	ret += '[';
	for(int i = 0; i < size; i++) {
		if(hex) {
			ret += QString("0x%1").arg(arr[i], sizeof(T) * 2, 16, QChar('0'));
		} else {
			ret += QString::number(arr[i]);
		}
		if(i < size - 1) {
			ret += ", ";
		}
	}
	ret += ']';

	return ret;
}

const int GenomeAlignerFindTask::ALIGN_DATA_SIZE = 100000;

GenomeAlignerFindTask::GenomeAlignerFindTask(U2::GenomeAlignerIndex *i, AlignContext *s, GenomeAlignerWriteTask *w)
: Task("GenomeAlignerFindTask", TaskFlag_None),
index(i), writeTask(w), alignContext(s), binarySearchResults(NULL)
{
    partLoaded = false;
    openCLFinished = false;
    nextElementToGive = 0;
    indexLoadTime = 0;
    waiterCount = 0;
    alignerTaskCount = 0;
}

void GenomeAlignerFindTask::prepare() {
    if (alignContext->useCUDA) {
        //proceed to run function
        return;
    }

	if(alignContext->openCL) {
		// no reason to have several parallel subtasks using openCL since they'll be waiting on the same mutex anyway
		waiterCount = 0;
		nextElementToGive = 0;
		alignerTaskCount = 1;
		Task *subTask = new ShortReadAlignerOpenCL(0, index, alignContext, writeTask);
		subTask->setSubtaskProgressWeight(1.0f);
		addSubTask(subTask);
	} else {
		alignerTaskCount = AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount();
		setMaxParallelSubtasks(alignerTaskCount);
		for (int i = 0; i < alignerTaskCount; i++) {
			waiterCount = 0;
			nextElementToGive = 0;
			Task *subTask = new ShortReadAlignerCPU(i, index, alignContext, writeTask);
			subTask->setSubtaskProgressWeight(1.0f / alignerTaskCount);
			addSubTask(subTask);
		}
	}
}

void GenomeAlignerFindTask::run() {
    if (alignContext->useCUDA) {
        
        GenomeAlignerCUDAHelper cudaHelper;
        
        cudaHelper.loadShortReads(alignContext->queries, stateInfo);
        if (hasError()) {
            return;
        }

        for (int part = 0; part < index->getPartCount(); ++part) {
            if (!index->loadPart(part)) {
                setError("Incorrect index file. Please, try to create a new index file.");
            }
            cudaHelper.alignReads(index->getLoadedPart(),alignContext, stateInfo);
            if (hasError()) {
                return;
            }
        }
    }
}

void GenomeAlignerFindTask::loadPartForAligning(int part) {
    waitMutex.lock();
    waiterCount++;
    if (waiterCount != alignerTaskCount) {
        waiter.wait(&waitMutex);
        waiter.wakeOne();
    } else {
        waiterCount = 0;
        partLoaded = false;
        waiter.wakeOne();
    }
    waitMutex.unlock();

    QMutexLocker lock(&loadPartMutex);
    if (!partLoaded) {
        taskLog.details(QString("loading part %1").arg(part));
        if (!index->loadPart(part)) {
            setError("Incorrect index file. Please, try to create a new index file.");
        }
        partLoaded = true;
        openCLFinished = false;
        nextElementToGive = 0;
        taskLog.details(QString("finish to load part %1").arg(part));
    }
}

void GenomeAlignerFindTask::unsafeGetData(int &first, int &length) {
    int bitValuesCount = alignContext->bitValuesV.size();

    first = nextElementToGive;

    if (first >= bitValuesCount) {
        length = 0;
    } else if (first + ALIGN_DATA_SIZE > bitValuesCount) {
        length = bitValuesCount - first;
    } else {
        length = ALIGN_DATA_SIZE;
    }

    QVector<int> &rn = alignContext->readNumbersV;
    int it = first + length;
    for (int last=it-1; it<bitValuesCount; it++) {
        if (rn[last] == rn[it]) {
            length++;
        } else {
            SearchQuery *lastQu = alignContext->queries.at(rn[last]);
            SearchQuery *qu = alignContext->queries.at(rn[it]);
            if (lastQu->getRevCompl() == qu) {
                last = it;
                length++;
            } else {
                break;
            }
        }
    }

    nextElementToGive += length;
}

void GenomeAlignerFindTask::getDataForAligning(int &first, int &length) {
    QMutexLocker lock(&shortReadsMutex);
    unsafeGetData(first, length);
}

void GenomeAlignerFindTask::waitDataForAligning(int &first, int &length) {
    QMutexLocker lock(&shortReadsMutex);
    if (alignContext->openCL) {
        while (!alignContext->isReadingFinished) {
            alignContext->alignerWait.wait(&shortReadsMutex);
        }
    } else {
        while ((!alignContext->isReadingFinished && alignContext->bitValuesV.size() - nextElementToGive < ALIGN_DATA_SIZE)
            || !alignContext->isReadingStarted) { //while (not enough read) wait
            alignContext->alignerWait.wait(&shortReadsMutex);
        }
    }
    unsafeGetData(first, length);
    if (alignContext->isReadingFinished) {
        alignContext->alignerWait.wakeAll();
    }
}

#ifdef OPENCL_SUPPORT
bool GenomeAlignerFindTask::runOpenCLBinarySearch() {
    QMutexLocker lock(&openCLMutex);
    if (!openCLFinished) {
        openCLFinished = true;
        delete[] binarySearchResults;
        if (0 == alignContext->bitValuesV.size()) {
            binarySearchResults = NULL;
            return false;
        }
        binarySearchResults = index->bitMaskBinarySearchOpenCL(alignContext->bitValuesV.constData(), alignContext->bitValuesV.size(), 
			alignContext->windowSizes.constData());
        if (NULL == binarySearchResults) {
            setError("OpenCL binary find error");
            return false;
        }
    }

    if (NULL == binarySearchResults) {
        return false;
    }

    return true;
}
#endif

GenomeAlignerFindTask::~GenomeAlignerFindTask() {
	if(alignContext->openCL) {
		// otherwise it's a pointer to QVector data, which will be deleted automatically
		delete[] binarySearchResults;
	}
}

ShortReadAlignerCPU::ShortReadAlignerCPU(int taskNo, GenomeAlignerIndex *i, AlignContext *s, GenomeAlignerWriteTask *w)
: Task("ShortReadAlignerCPU", TaskFlag_None), taskNo(taskNo), index(i), alignContext(s), writeTask(w)
{
}

void ShortReadAlignerCPU::run() {
	assert(!alignContext->openCL);
    GenomeAlignerFindTask *parent = static_cast<GenomeAlignerFindTask*>(getParentTask());
    SearchQuery *shortRead = NULL;
    SearchQuery *revCompl = NULL;
    int first = 0;
    int last = 0;
    int length = 0;
    BinarySearchResult bmr = 0;

    //for thread safe:
    BMType bv = 0;
    int rn = 0;
    int rn1 = 0;
    int pos = 0;

    for (int part = 0; part < index->getPartCount(); part++) {
        stateInfo.setProgress(100*part/index->getPartCount());
        parent->loadPartForAligning(part);
        if (parent->hasError()) {
            return;
        }
        stateInfo.setProgress(stateInfo.getProgress() + 25/index->getPartCount());
		if(0 == parent->index->getLoadedPart().getLoadedPartSize()) {
			algoLog.info(tr("Index size for part %1/%2 is zero, skipping it.").arg(part + 1).arg(index->getPartCount()));
			continue;
		}

		do {
			// fetch a batch of reads
			if (part > 0) {
				SAFE_POINT(alignContext->isReadingFinished, "Synchronization error", );
				parent->getDataForAligning(first, length);
			} else {
				parent->waitDataForAligning(first, length);
			}

			if(0 == length) {
				break;
			}
// 			algoLog.details(QString("ShortReadAlignerCPU subtask %1: fetched %3 needles starting at offset %2, next element=%4, queries=%5, bitValues=%6")
// 				.arg(taskNo).arg(first).arg(length).arg(parent->nextElementToGive).arg(alignContext->queries.size()).arg(alignContext->bitValuesV.size()));
// 
// 			algoLog.details(QString("needles: %1").arg(numArrToStr(alignContext->bitValuesV.constData(), alignContext->bitValuesV.size(), true)));
// 			algoLog.details(QString("window sizes: %1").arg(numArrToStr(alignContext->windowSizes.constData(), alignContext->windowSizes.size())));
// 			algoLog.details(QString("haystack: %1").arg(numArrToStr(index->getLoadedPart().bitMask, (int)(index->getLoadedPart().getLoadedPartSize()), true)));

			SearchQuery **q = const_cast<SearchQuery**>(alignContext->queries.constData());
			const BMType *bitValues = alignContext->bitValuesV.constData();
			const int *readNumbers = alignContext->readNumbersV.constData();
			const int *par = alignContext->positionsAtReadV.constData();
			const int *windowSizes = alignContext->windowSizes.constData();

			last = first + length;
			for (int i = first; i < last; i++) {
				int currentW = alignContext->windowSizes.at(i);
				if(0 == currentW) {
					continue;
				}
				BMType currentBitFilter = ((quint64)0 - 1) << (62 - currentW * 2);

				if (part > 0 || alignContext->openCL) { //for avoiding a QVector deep copy
					SAFE_POINT(alignContext->isReadingFinished, "Synchronization error", );
					bv = bitValues[i];
					rn = readNumbers[i];
					pos = par[i];
					if (i < last - 1) {
						rn1 = readNumbers[i+1];
					}
					shortRead = q[rn];
				} else {
					QMutexLocker lock(&alignContext->listM);
					bv = alignContext->bitValuesV.at(i);
					rn = alignContext->readNumbersV.at(i);
					pos = alignContext->positionsAtReadV.at(i);
					if (i < last - 1) {
						rn1 = alignContext->readNumbersV.at(i + 1);
					}
					shortRead = alignContext->queries.at(rn);
				}
				revCompl = shortRead->getRevCompl();
				if (alignContext->bestMode) {
					if (0 == shortRead->firstMCount()) {
						continue;
					}
					if (NULL != revCompl && 0 == revCompl->firstMCount()) {
						continue;
					}
				}

				bmr = index->bitMaskBinarySearch(bv, currentBitFilter);
// 				algoLog.details(QString("needle %1 found at offset %2").arg(i).arg(bmr));
				index->alignShortRead(shortRead, bv, pos, bmr, alignContext, currentBitFilter, currentW);

				if (!alignContext->bestMode) {
					if ((i == last - 1) || (rn1 != rn)) {
						if (shortRead->haveResult()) {
							writeTask->addResult(shortRead);
						}
						shortRead->onPartChanged();
					}
				}
			}
		} while(true);
    }
}


ShortReadAlignerOpenCL::ShortReadAlignerOpenCL(int taskNo, GenomeAlignerIndex *i, AlignContext *s, GenomeAlignerWriteTask *w)
: Task("ShortReadAlignerOpenCL", TaskFlag_None), taskNo(taskNo), index(i), alignContext(s), writeTask(w)
{
}

void ShortReadAlignerOpenCL::run() {
#ifdef OPENCL_SUPPORT
	assert(alignContext->openCL);

	GenomeAlignerFindTask *parent = static_cast<GenomeAlignerFindTask*>(getParentTask());
	SearchQuery *shortRead = NULL;
	SearchQuery *revCompl = NULL;
	int first = 0;
	int length = 0;
	BinarySearchResult bmr = 0;

	//for thread safe:
	BMType bv = 0;
	int rn = 0;
	int rn1 = 0;
	int pos = 0;

	for (int part = 0; part < index->getPartCount(); part++) {
		stateInfo.setProgress(100 * part / index->getPartCount());
		parent->loadPartForAligning(part);
		if (parent->hasError()) {
			return;
		}
		stateInfo.setProgress(stateInfo.getProgress() + 25 / index->getPartCount());
		if(0 == parent->index->getLoadedPart().getLoadedPartSize()) {
			algoLog.info(tr("Index size for part %1/%2 is zero, skipping it.").arg(part + 1).arg(index->getPartCount()));
			continue;
		}

		// wait until all short reads are loaded
		do {
			if (part > 0) {
				SAFE_POINT(alignContext->isReadingFinished, "Synchronization error", );
				parent->getDataForAligning(first, length);
			} else { //if (0 == part) then wait for reading shortreads
				parent->waitDataForAligning(first, length);
			}
// 			algoLog.details(QString("ShortReadAlignerOpenCL subtask %1: fetched %3 needles starting at offset %2, next element=%4, queries=%5, bitValues=%6")
// 				.arg(taskNo).arg(first).arg(length).arg(parent->nextElementToGive).arg(alignContext->queries.size()).arg(alignContext->bitValuesV.size()));
		} while(length > 0);

// 		algoLog.details(QString("needles: %1").arg(numArrToStr(alignContext->bitValuesV.constData(), alignContext->bitValuesV.size(), true)));
// 		algoLog.details(QString("window sizes: %1").arg(numArrToStr(alignContext->windowSizes.constData(), alignContext->windowSizes.size())));
// 		algoLog.details(QString("haystack: %1").arg(numArrToStr(index->getLoadedPart().bitMask, (int)(index->getLoadedPart().getLoadedPartSize()), true)));

		if (!parent->runOpenCLBinarySearch()) {
			return;
		}
//  		algoLog.details(QString("search results: %1").arg(numArrToStr(parent->binarySearchResults, alignContext->bitValuesV.size())));
		stateInfo.setProgress(stateInfo.getProgress() + 50/index->getPartCount());

		SearchQuery **q = const_cast<SearchQuery**>(alignContext->queries.constData());
		const BMType *bitValues = alignContext->bitValuesV.constData();
		const int *readNumbers = alignContext->readNumbersV.constData();
		const int *par = alignContext->positionsAtReadV.constData();
		const int *windowSizes = alignContext->windowSizes.constData();
		const int totalResults = alignContext->bitValuesV.size();

		for (int i = 0; i < totalResults; i++) {
			int currentW = alignContext->windowSizes.at(i);
			if(0 == currentW) {
				continue;
			}
			BMType currentBitFilter = ((quint64)0 - 1) << (62 - currentW * 2);
// 			algoLog.details(QString("bitFilter value for needle #%1 is 0x%2").arg(i).arg(currentBitFilter, sizeof(BMType) * 2, 16, QChar('0')));

			if (part > 0 || alignContext->openCL) { //for avoiding a QVector deep copy
				SAFE_POINT(alignContext->isReadingFinished, "Synchronization error", );
				bv = bitValues[i];
				rn = readNumbers[i];
				pos = par[i];
				if (i < totalResults - 1) {
					rn1 = readNumbers[i + 1];
				}
				shortRead = q[rn];
			} else {
				QMutexLocker lock(&alignContext->listM);
				bv = alignContext->bitValuesV.at(i);
				rn = alignContext->readNumbersV.at(i);
				pos = alignContext->positionsAtReadV.at(i);
				if (i < totalResults - 1) {
					rn1 = alignContext->readNumbersV.at(i + 1);
				}
				shortRead = alignContext->queries.at(rn);
			}
			revCompl = shortRead->getRevCompl();
			if (alignContext->bestMode) {
				if (0 == shortRead->firstMCount()) {
					continue;
				}
				if (NULL != revCompl && 0 == revCompl->firstMCount()) {
					continue;
				}
			}

			bmr = parent->binarySearchResults[i];
			index->alignShortRead(shortRead, bv, pos, bmr, alignContext, currentBitFilter, currentW);

			if (!alignContext->bestMode) {
				if ((i == totalResults - 1) || (rn1 != rn)) {
					if (shortRead->haveResult()) {
						writeTask->addResult(shortRead);
					}
					shortRead->onPartChanged();
				}
			}
		}
	}
#endif
}

} // U2
