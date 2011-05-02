/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include <QtCore/QDir>

#include <U2Core/AppContext.h>
#include <U2Core/CMDLineRegistry.h>

#include "GenomeAlignerTask.h"
#include "GenomeAlignerCMDLineTask.h"

namespace U2 {

#define OPTION_INDEX_PATH   "index"
#define OPTION_SHORTREADS   "short-reads"
#define OPTION_RESULT       "result"
#define OPTION_BUILD_INDEX  "build-index"
#define OPTION_REFERENCE    "reference"
#define OPTION_MEMSIZE      "memsize"
#define OPTION_CUDA         "use-cuda"
#define OPTION_USE_OPENCL   "use-opencl"
#define OPTION_REF_FRAG     "ref-size"


 GenomeAlignerCMDLineTask::GenomeAlignerCMDLineTask()
:Task( tr( "Run genome aligner from command line" ), TaskFlags_NR_FOSCOE), onlyBuildIndex(false)
{
    memSize = 1000;
    mismatchCount = 0;
    refSize = 10;
    alignRevCompl = false;
    useCuda = false;
    useOpenCL = false;

    // parse options
       
    QList<StringPair> options = AppContext::getCMDLineRegistry()->getParameters();
    
    foreach (const StringPair& opt, options ) {
        if (opt.first == OPTION_INDEX_PATH  ) {
            indexPath = opt.second;
        }else if (opt.first == OPTION_BUILD_INDEX ) {
            onlyBuildIndex = true;    
        } else if (opt.first == OPTION_REFERENCE) {
            refPath = opt.second;
        } else if (opt.first == OPTION_RESULT) {
            resultPath = opt.second;
        } else if (opt.first == OPTION_SHORTREADS) {
            QStringList urls = opt.second.split(";");
            foreach(const QString& url, urls) {
                shortReadUrls.append(url);
            }
        } else if (opt.first == OPTION_CUDA) {
            useCuda = true;
            useOpenCL = false;
        } else if (opt.first == OPTION_USE_OPENCL) {
            useOpenCL = true;
            useCuda = false;
        } else if (opt.first == OPTION_REF_FRAG) {
            refSize = opt.second.toInt();
            assert(refSize != 0);
            if (refSize == 0) {
                refSize = 10;
            }
        }
    }
        
    coreLog.info( tr( "Finished parsing genome aligner options." ) );

}

GenomeAlignerCMDLineTask::~GenomeAlignerCMDLineTask()
{
    // clean up resources
}

void GenomeAlignerCMDLineTask::prepare()
{
    if (onlyBuildIndex && refPath.isEmpty()) {
        setError(tr("Path to reference sequence is not set."));
        return;
    }
        
    if (!onlyBuildIndex) {
        if ( shortReadUrls.isEmpty() ) {
            setError(tr("Short reads list is empty."));
            return;
        } else {
            if (refPath.isEmpty() && indexPath.isEmpty()) {
                setError(tr("Reference (index or sequence) is not set."));
                return;
            }
        }
    }
     
    if (resultPath.isEmpty()) {
        resultPath =  QDir::current().path() + "/output.sam";
    }
    
    settings.resultFileName = resultPath;
    settings.shortReadUrls = shortReadUrls;
    settings.refSeqUrl = refPath;
    settings.indexFileName = indexPath;
    settings.loadResultDocument = false;
    if (onlyBuildIndex) {
        settings.prebuiltIndex = false;
    }

    settings.setCustomValue(GenomeAlignerTask::OPTION_MISMATCHES, mismatchCount);
    settings.setCustomValue(GenomeAlignerTask::OPTION_READS_MEMORY_SIZE, memSize);
    settings.setCustomValue(GenomeAlignerTask::OPTION_ALIGN_REVERSED, alignRevCompl);
    settings.setCustomValue(GenomeAlignerTask::OPTION_USE_CUDA, useCuda);
    settings.setCustomValue(GenomeAlignerTask::OPTION_OPENCL, useOpenCL);
    settings.setCustomValue(GenomeAlignerTask::OPTION_SEQ_PART_SIZE, refSize);


    GenomeAlignerTask* task = new GenomeAlignerTask(settings, onlyBuildIndex);
    addSubTask(task);

}

QString GenomeAlignerCMDLineTask::getArgumentsDescritption()
{
    QString desc;
    desc += tr("  --%1    Use this flag to only build index for reference sequence.\n\n").arg(OPTION_BUILD_INDEX);
    desc += tr("  --%1    Path to reference genome sequence\n\n").arg(OPTION_REFERENCE);
    desc += tr("  --%1    Path to short-reads data in FASTA or FASTQ format\n\n").arg(OPTION_SHORTREADS);
    desc += tr("  --%1    Path to prebuilt index. If not set, index is searched in system temporary directory.\n\n").arg(OPTION_INDEX_PATH);
    desc += tr("  --%1    Path to output alignment in SAM format\n\n").arg(OPTION_RESULT);
    desc += tr("  --%1    Memory size (in Mbs) reserved for short-reads. The bigger value the faster algorithm works. Default value depends on available system memory.\n\n").arg(OPTION_MEMSIZE);
    desc += tr("  --%1    Index fragmentation size (in Mbs). Small fragments better fit into RAM, allowing to load more short reads. Default value is 10.\n").arg(OPTION_REF_FRAG);


    return desc;
}


}//namespace

