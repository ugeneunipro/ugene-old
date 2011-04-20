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

#ifndef _U2_BOWTIE_WORKER_H_
#define _U2_BOWTIE_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include "BowtieTask.h"
#include "BowtieReadsIOUtils.h"

namespace U2 {
namespace LocalWorkflow {

class BowtieCommunicationChanelReader : public BowtieReadsReader {
public:
	BowtieCommunicationChanelReader(CommunicationChannel* reads);
	DNASequence *read();
	bool isEnd();
private:
	CommunicationChannel* reads;
};

class BowtieMAlignmentWriter : public BowtieReadsWriter {
public:
	BowtieMAlignmentWriter();
	void write(const DNASequence& seq, int offset);
	void close();
	MAlignment& getResult();
private:
	MAlignment result;
};

class BowtiePrompter : public PrompterBase<BowtiePrompter> {
	Q_OBJECT
public:
	BowtiePrompter(Actor* p = 0) : PrompterBase<BowtiePrompter>(p) {}
protected:
	QString composeRichDoc();
};

class BowtieWorker : public BaseWorker {
	Q_OBJECT
public:
	BowtieWorker(Actor* a) : BaseWorker(a, false), reads(NULL), ebwt(NULL), output(NULL), reader(NULL), writer(NULL) {}
	virtual void init() ;
	virtual bool isReady();
	virtual Task* tick() ;
	virtual bool isDone() ;
	virtual void cleanup();
	private slots:
		void sl_taskFinished();

protected:
	CommunicationChannel *reads, *ebwt, *output;
	BowtieReadsReader* reader;
	BowtieReadsWriter* writer;
	QString resultName,transId;
	DnaAssemblyToRefTaskSettings settings;
}; 

class BowtieWorkerFactory : public DomainFactory {
public:
	static const QString ACTOR_ID;
	static void init();
	BowtieWorkerFactory() : DomainFactory(ACTOR_ID) {}
	virtual Worker* createWorker(Actor* a) {return new BowtieWorker(a);}
};

/************************************************************************/
/* Bowtie build                                                         */
/************************************************************************/

class BowtieBuildPrompter : public PrompterBase<BowtieBuildPrompter> {
	Q_OBJECT
public:
	BowtieBuildPrompter(Actor* p = 0) : PrompterBase<BowtieBuildPrompter>(p) {}
protected:
	QString composeRichDoc();
};

class BowtieBuildWorker : public BaseWorker {
	Q_OBJECT
public:
	BowtieBuildWorker(Actor* a) : BaseWorker(a), output(NULL), done(false) {}
	virtual void init() ;
	virtual bool isReady();
	virtual Task* tick() ;
	virtual bool isDone() ;
	virtual void cleanup() {}
	private slots:
		void sl_taskFinished();

protected:
	CommunicationChannel *output;
	QString resultName,transId;
	GUrl refSeqUrl;
	GUrl ebwtUrl;
	bool done;
}; 

class BowtieBuildWorkerFactory : public DomainFactory {
public:
	static const QString ACTOR_ID;
	static void init();
	BowtieBuildWorkerFactory() : DomainFactory(ACTOR_ID) {}
	virtual Worker* createWorker(Actor* a) {return new BowtieBuildWorker(a);}
};

/************************************************************************/
/* Bowtie index reader                                                  */
/************************************************************************/

class BowtieIndexReaderPrompter : public PrompterBase<BowtieIndexReaderPrompter> {
	Q_OBJECT
public:
	BowtieIndexReaderPrompter(Actor* p = 0) : PrompterBase<BowtieIndexReaderPrompter>(p) {}
protected:
	QString composeRichDoc();
};

class BowtieIndexReaderWorker : public BaseWorker {
	Q_OBJECT
public:
	BowtieIndexReaderWorker(Actor* a) : BaseWorker(a), output(NULL), done(false) {}
	virtual void init() ;
	virtual bool isReady();
	virtual Task* tick() ;
	virtual bool isDone() ;
	virtual void cleanup() {}
	private slots:
		void sl_taskFinished();

protected:
	CommunicationChannel *output;
	QString resultName,transId;
	GUrl ebwtUrl;
	bool done;
}; 

class BowtieIndexReaderWorkerFactory : public DomainFactory {
public:
	static const QString ACTOR_ID;
	static void init();
	BowtieIndexReaderWorkerFactory() : DomainFactory(ACTOR_ID) {}
	virtual Worker* createWorker(Actor* a) {return new BowtieIndexReaderWorker(a);}
};

} // Workflow namespace
} // U2 namespace


#endif // BOWTIE_WORKER
