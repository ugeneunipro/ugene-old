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

//////////////////////////////////////////////////
//QualityTrim
class QualityTrimPrompter;
typedef PrompterBase<QualityTrimPrompter> QualityTrimBase;
class QualityTrimPrompter : public QualityTrimBase {
    Q_OBJECT
public:
    QualityTrimPrompter(Actor* p = 0) : QualityTrimBase(p) {}
protected:
    QString composeRichDoc();
}; //QualityTrimPrompter

class QualityTrimWorker: public BaseNGSWorker {
    Q_OBJECT
public:
    QualityTrimWorker(Actor *a);
protected:
    QVariantMap getCustomParameters() const;
    QString getDefaultFileName() const;
    Task *getTask(const BaseNGSSetting &settings) const;

}; //QualityTrimWorker

class QualityTrimWorkerFactory : public DomainFactory {
    static const QString ACTOR_ID;
public:
    static void init();
    QualityTrimWorkerFactory() : DomainFactory(ACTOR_ID) {}
    Worker* createWorker(Actor* a) { return new QualityTrimWorker(a); }
}; //QualityTrimWorkerFactory

class QualityTrimTask : public BaseNGSTask{
    Q_OBJECT
public:
    QualityTrimTask (const BaseNGSSetting &settings);

protected:
    void runStep();
    QStringList getParameters(U2OpStatus& os);
};

//////////////////////////////////////////////////
//MergeFastq
class MergeFastqPrompter;
typedef PrompterBase<MergeFastqPrompter> MergeFastqBase;
class MergeFastqPrompter : public MergeFastqBase {
    Q_OBJECT
public:
    MergeFastqPrompter(Actor* p = 0) : MergeFastqBase(p) {}
protected:
    QString composeRichDoc();
}; //MergeFastqPrompter

class MergeFastqWorker: public BaseNGSWorker {
    Q_OBJECT
public:
    MergeFastqWorker(Actor *a);
    Task * tick();
protected:
    QVariantMap getCustomParameters() const;
    QString getDefaultFileName() const;
    Task *getTask(const BaseNGSSetting &settings) const;

    QStringList inputUrls;

}; //MergeFastqWorker

class MergeFastqWorkerFactory : public DomainFactory {
    static const QString ACTOR_ID;
public:
    static void init();
    MergeFastqWorkerFactory() : DomainFactory(ACTOR_ID) {}
    Worker* createWorker(Actor* a) { return new MergeFastqWorker(a); }
}; //MergeFastqWorkerFactory

class MergeFastqTask : public BaseNGSTask{
    Q_OBJECT
public:
    MergeFastqTask (const BaseNGSSetting &settings);

protected:
    void runStep();
    QStringList getParameters(U2OpStatus& os);
};



} //LocalWorkflow
} //U2

#endif //_U2_FASTQ_WORKERS_LIBRARY_
