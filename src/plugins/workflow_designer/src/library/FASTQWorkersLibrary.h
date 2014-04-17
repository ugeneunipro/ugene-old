/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_FASTQ_WORKERS_LIBRARY_
#define _U2_FASTQ_WORKERS_LIBRARY_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include <U2Lang/BaseNGSWorker.h>
#include <U2Core/GUrl.h>

namespace U2 {
namespace LocalWorkflow {

//////////////////////////////////////////////////
//CASAVAFilter
class CASAVAFilterPrompter;
typedef PrompterBase<CASAVAFilterPrompter> CASAVAFilterBase;
class CASAVAFilterPrompter : public CASAVAFilterBase {
    Q_OBJECT
public:
    CASAVAFilterPrompter(Actor* p = 0) : CASAVAFilterBase(p) {}
protected:
    QString composeRichDoc();
}; //CASAVAFilterPrompter

class CASAVAFilterWorker: public BaseNGSWorker {
    Q_OBJECT
public:
    CASAVAFilterWorker(Actor *a);
protected:
    QVariantMap getCustomParameters() const;
    QString getDefaultFileName() const;
    Task *getTask(const BaseNGSSetting &settings) const;

}; //CASAVAFilterWorker

class CASAVAFilterWorkerFactory : public DomainFactory {
    static const QString ACTOR_ID;
public:
    static void init();
    CASAVAFilterWorkerFactory() : DomainFactory(ACTOR_ID) {}
    Worker* createWorker(Actor* a) { return new CASAVAFilterWorker(a); }
}; //CASAVAFilterWorkerFactory

class CASAVAFilterTask : public BaseNGSTask{
    Q_OBJECT
public:
    CASAVAFilterTask (const BaseNGSSetting &settings);

protected:
    void runStep();
    QStringList getParameters(U2OpStatus& os);
};

} //LocalWorkflow
} //U2

#endif //_U2_FASTQ_WORKERS_LIBRARY_
