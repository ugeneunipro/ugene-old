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

#include "BowtieConstants.h"
#include "BowtieAdapter.h"
#include "BowtieIOAdapter.h"
#include "BowtieContext.h"
#include "BowtieReadsIOUtils.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/AppContext.h>
#include <U2Core/Task.h>
#include <U2Core/Log.h>
#include <U2Core/MAlignment.h>
#include <U2Core/DNASequence.h>

#include <QtCore/QVector>

#include <stdlib.h>
#include <stdexcept>
#include <vector>

using namespace std;
using namespace seqan;

extern void prepareSearchOptions();

void driverAdapter(U2::BowtieReadsReader* reader,
				   U2::BowtieReadsWriter* writer,
				   const U2::GUrl& resultMA,
				   const char * type,
				   const string& ebwtFileBase,
				   const string& query,
				   const vector<string>& queries,
				   const vector<string>& qualities,
				   const string& outfile);

int bowtieBuildAdapter(const string& infile, const string& outfile);

namespace U2 {

void BowtieAdapter::doBowtie(const QString& ebwtFileName, BowtieReadsReader* reader, U2::BowtieReadsWriter* writer, const U2::GUrl& resultUrl, TaskStateInfo& ti) {
	BowtieContext *ctx = BowtieContext::getContext();
	try {
		prepareSearchOptions();
		//driver<String<Dna, Alloc<> > >("DNA", ebwtFileName, query, queries, qualities, outfile);
		vector<string> queries;
		queries.push_back("reads/reads");
		driverAdapter(reader, writer, resultUrl, "DNA", ebwtFileName.toAscii().constData(), "", queries, vector<string>(), "");
// 		if(output.getRows().count() == 0) {
// 			ti.setError(tr("Reference assembly failed - no possible alignment found"));
// 			return;
// 		}
//         output.setAlphabet(AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT));
		//CHUD_STOP();
	}catch(exception& e) {
		ti.setError(QString("Bowtie exits with error \"%1\"").arg(e.what()));
		ctx->hasError = true;
	} catch(int e) {
		if(e != 0) {
			ti.setError(QString("Bowtie exits with error code %1").arg(e));
			ctx->hasError = true;
		}
	}
	if(ctx->hasError) {
		for(int i=0;i<ctx->search.nthreads-1;i++) {
			ctx->workerList[i]->start.release();
		}
	}
}



void BowtieAdapter::doBowtieBuild(const QString& infile, const QString& outfile, TaskStateInfo& ti )
{
	try{
		bowtieBuildAdapter(infile.toLocal8Bit().constData(), outfile.toLocal8Bit().constData());
	} catch(std::exception& e) {
		if(!(ti.cancelFlag && QString("Canceled") == e.what()))
		ti.setError(QString("Bowtie-build exits with error: %1").arg(e.what()));
	} catch(int e) {
		ti.setError(QString("Bowtie-build exits with error code %1").arg(e));
	}
}

void BowtieAdapter::doBowtieWorker(int id, TaskStateInfo& ti)
{
	BowtieContext* ctx = BowtieContext::getContext();
	ctx->workerList[id]->start.acquire();
	if(ctx->hasError || ctx->isCanceled())
	{
		return;
	}
	BowtieContext::BowtieWorkerArg arg = ctx->jobList[id];
	try {
		arg.start_routine(arg.arg);
	} 
	catch(int e) {
		if(e == 2) {
			algoLog.trace(QString("Exiting worker %1").arg(id));
		} else {
			ti.setError("Bowtie worker throws known exception");
			ctx->hasError = true;
		}
	}
	catch(...) {
		ti.setError("Bowtie worker throws unknown exception");
		ctx->hasError = true;
	}
	ctx->workerList[id]->stop.release();
}
}
