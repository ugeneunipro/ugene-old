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

#ifndef _U2_BOWTIE_TASK_H_
#define _U2_BOWTIE_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/TLSTask.h>
#include <U2Algorithm/DnaAssemblyTask.h>
#include <U2Core/MAlignmentObject.h>

#include <QtCore/QPointer>
#include <QtCore/QMutex>
#include <QtCore/QSemaphore>

namespace U2 {

#define BOWTIE_ALG_NAME "Bowtie"

class StateLock;
class MAlignmentObject;
class LoadDocumentTask;
class BowtieTLSTask;
class BowtieTask;
class BowtieBuildTask;


class BowtieTask : public DnaAssemblyToReferenceTask {
    Q_OBJECT
	DNA_ASSEMBLEY_TO_REF_TASK_FACTORY(BowtieTask)
	friend class BowtieTLSTask;
public:

	static const QString OPTION_READS_READER; 
	static const QString OPTION_READS_WRITER;

	static const QString OPTION_PREBUILT_INDEX; 
	static const QString OPTION_N_MISMATCHES; 
	static const QString OPTION_V_MISMATCHES; 
	static const QString OPTION_MAQERR; // -e ( e>=1, default 70)
	static const QString OPTION_SEED_LEN; // -l ( l>=5, default 28)
	static const QString OPTION_NOMAQROUND; // --nomaqround
	static const QString OPTION_NOFW; // --nofw
	static const QString OPTION_NORC; // --norc
	static const QString OPTION_MAXBTS; //--maxbts >=0
	static const QString OPTION_TRYHARD; //-y/--tryhard
	static const QString OPTION_CHUNKMBS; //--chunkmbs ( >=1, default 64)
	static const QString OPTION_SEED; //--seed 
    static const QString OPTION_BEST; //--best
    static const QString OPTION_ALL; //--all
	static const QString OPTION_SORT_ALIGNMENT_BY_OFFSET;

    BowtieTask(const DnaAssemblyToRefTaskSettings & config, bool justBuildIndex = false);
	QList<Task*> onSubTaskFinished(Task* subTask);
	void prepare();
    ReportResult report();
    int numHits;
private:
	int fileSize;
    BowtieTLSTask* tlsTask;
	BowtieBuildTask* buildTask;
	QString indexPath;
	bool justBuildIndex;
};

class BowtieTLSTask : public TLSTask {
	Q_OBJECT
public:
	BowtieTLSTask();	
	void prepare();
	Task::ReportResult report();
protected:
	void _run();
	TLSContext* createContextInstance();
	int nThreads;
};

class BowtieWorkerTask : public TLSTask {
	Q_OBJECT
	friend class BowtieTLSTask;
public:
	BowtieWorkerTask(int _id, TLSContext* ctx): TLSTask("Bowtie Task Worker", TaskFlags_FOSCOE, false), id(_id) { taskContext = ctx; }
protected:
	void _run();
	TLSContext* createContextInstance() {return taskContext;};
private:
	int id;
};

class BowtieBuildTask : public TLSTask {
	Q_OBJECT
public:
	BowtieBuildTask(QString refPath, QString outEbwtPath);
	
	Task::ReportResult report();
	QString getEbwtPath() const { return outEbwtPath; }
protected:
	void _run();
	TLSContext* createContextInstance();
private:
	static QMutex mutex;
	QString refPath;
	QString outEbwtPath;
};

}//namespace
#endif
