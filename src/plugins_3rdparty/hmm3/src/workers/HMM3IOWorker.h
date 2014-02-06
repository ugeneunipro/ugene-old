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

#ifndef _U2_HMM3_IO_WORKER_H_
#define _U2_HMM3_IO_WORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowMonitor.h>


#include <U2Core/SaveDocumentTask.h>

#include <hmmer3/hmmer.h>

struct msa_struct;

Q_DECLARE_METATYPE(const P7_HMM*);

namespace U2 {
namespace LocalWorkflow {

class HMM3Lib : public QObject {
    Q_OBJECT
public:
    static const Descriptor HMM3_CATEGORY();
    static const Descriptor HMM3_SLOT;
    static DataTypePtr HMM3_PROFILE_TYPE();
    static const QString HMM3_PROFILE_TYPE_ID;
    static void init();
    static void cleanup();
};

class HMM3IOProto : public IntegralBusActorPrototype {
public:
    HMM3IOProto(const Descriptor& desc, const QList<PortDescriptor*>& ports, 
        const QList<Attribute*>& attrs = QList<Attribute*>());
    virtual bool isAcceptableDrop(const QMimeData*, QVariantMap*) const = 0;
    bool isAcceptableDrop(const QMimeData*, QVariantMap*, const QString&) const;
};

class ReadHMM3Proto : public HMM3IOProto {
public:
    ReadHMM3Proto(const Descriptor& _desc, const QList<PortDescriptor*>& _ports, 
        const QList<Attribute*>& _attrs = QList<Attribute*>());
    virtual bool isAcceptableDrop(const QMimeData*, QVariantMap*) const;
};

class WriteHMM3Proto : public HMM3IOProto {
public:
    WriteHMM3Proto(const Descriptor& _desc, const QList<PortDescriptor*>& _ports, 
        const QList<Attribute*>& _attrs = QList<Attribute*>());
    virtual bool isAcceptableDrop(const QMimeData*, QVariantMap*) const;
};

class HMM3ReadPrompter : public PrompterBase<HMM3ReadPrompter> {
    Q_OBJECT
public:
    HMM3ReadPrompter(Actor* p = 0) : PrompterBase<HMM3ReadPrompter>(p) {}
protected:
    QString composeRichDoc();
};

class HMM3WritePrompter : public PrompterBase<HMM3WritePrompter> {
    Q_OBJECT
public:
    HMM3WritePrompter(Actor* p = 0) : PrompterBase<HMM3WritePrompter>(p) {}
protected:
    QString composeRichDoc();
};


class HMM3Reader : public BaseWorker {
    Q_OBJECT
public:
    static const QString ACTOR;
    HMM3Reader(Actor* a) : BaseWorker(a), output(NULL) {}
    virtual void init();
    virtual Task* tick();
    virtual void cleanup() {}
private slots:
    void sl_taskFinished();

protected:
    CommunicationChannel *output;
    QStringList urls;
}; 

class HMM3Writer : public BaseWorker {
    Q_OBJECT
public:
    static const QString ACTOR;
    HMM3Writer(Actor* a) : BaseWorker(a), input(NULL), done(false), fileMode(SaveDoc_Overwrite) {}
    virtual void init();
    virtual Task* tick();
    virtual void cleanup() {}

protected:
    CommunicationChannel *input;
    QString url;
    QMap<QString,int> counter;
    bool done;
    uint fileMode;
}; 

class HMM3IOWorkerFactory : public DomainFactory {
public:
    static void init();
    static void cleanup();
    HMM3IOWorkerFactory(const Descriptor& d) : DomainFactory(d) {}
    virtual Worker* createWorker(Actor* a);
};

} // Workflow namespace
} // U2 namespace

#endif

