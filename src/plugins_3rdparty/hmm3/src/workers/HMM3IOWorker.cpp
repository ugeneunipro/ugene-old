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

#include "HMM3IOWorker.h"
#include "HMM3BuildWorker.h"
#include "HMM3SearchWorker.h"

#include <U2Lang/Datatype.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseAttributes.h>

#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/Log.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/FailTask.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/LoadDocumentTask.h>

#include <util/uhmm3Utilities.h>
#include <format/uHMMFormat.h>
#include <gobject/uHMMObject.h>
#include <build/uHMM3BuildTask.h>

#include <U2Gui/DialogUtils.h>


/* TRANSLATOR U2::LocalWorkflow::HMM3Lib */

namespace U2 {
namespace LocalWorkflow {

const QString HMM3_ID("hmm3");
const QString HMM3_EXT("hmm");
const QString HMM3Reader::ACTOR("hmm3-read-profile");
const QString HMM3Writer::ACTOR("hmm3-write-profile");
static const QString HMM3_OUT_PORT_ID("out-hmm3");
static const QString HMM3_IN_PORT_ID("in-hmm3");
const QString HMM3Lib::HMM3_PROFILE_TYPE_ID("hmm3.profile");

DataTypePtr HMM3Lib::HMM3_PROFILE_TYPE() {
    DataTypeRegistry* dtr = WorkflowEnv::getDataTypeRegistry();
    assert(dtr);
    static bool startup = true;
    if (startup)
    {
        dtr->registerEntry(DataTypePtr(new DataType(HMM3_PROFILE_TYPE_ID, tr("HMM3 Profile"), "")));
        startup = false;
    }
    return dtr->getById(HMM3_PROFILE_TYPE_ID);
}

const Descriptor HMM3Lib::HMM3_SLOT("hmm3-profile", HMM3Lib::tr("HMM3 Profile"), "");

const Descriptor HMM3Lib::HMM3_CATEGORY() {return Descriptor("hmmer3", tr("HMMER3 Tools"), "");}

HMM3IOProto::HMM3IOProto(const Descriptor& _desc, const QList<PortDescriptor*>& _ports, const QList<Attribute*>& _attrs ) 
: IntegralBusActorPrototype(_desc, _ports, _attrs) {
}

bool HMM3IOProto::isAcceptableDrop(const QMimeData * md, QVariantMap * params, const QString & urlAttrId ) const {
    if (md->hasUrls()) {
        QList<QUrl> urls = md->urls();
        if (urls.size() == 1)
        {
            QString url = urls.at(0).toLocalFile();
            QString ext = GUrlUtils::getUncompressedExtension(url);
            if (ext == HMM3_EXT) {
                if (params) {
                    params->insert(urlAttrId, url);
                }
                return true;
            }
        }
    }
    return false;
}

ReadHMM3Proto::ReadHMM3Proto(const Descriptor& _desc, const QList<PortDescriptor*>& _ports, const QList<Attribute*>& _attrs ) 
: HMM3IOProto( _desc, _ports, _attrs ) {
    
    attrs << new Attribute(BaseAttributes::URL_IN_ATTRIBUTE(), BaseTypes::STRING_TYPE(), true);
    QMap<QString, PropertyDelegate*> delegateMap;
    delegateMap[BaseAttributes::URL_IN_ATTRIBUTE().getId()] = new URLDelegate( DialogUtils::prepareDocumentsFileFilterByObjType( UHMMObject::UHMM_OT, true), HMM3_ID, true, false, false );
    setEditor(new DelegateEditor(delegateMap));
    setIconPath( ":/hmm3/images/hmmer_16.png" );
}

bool ReadHMM3Proto::isAcceptableDrop(const QMimeData * md, QVariantMap * params ) const {
    return HMM3IOProto::isAcceptableDrop( md, params, BaseAttributes::URL_IN_ATTRIBUTE().getId() );
}

WriteHMM3Proto::WriteHMM3Proto(const Descriptor& _desc, const QList<PortDescriptor*>& _ports, const QList<Attribute*>& _attrs ) 
: HMM3IOProto( _desc, _ports, _attrs ) {
    attrs << new Attribute(BaseAttributes::URL_OUT_ATTRIBUTE(), BaseTypes::STRING_TYPE(), true );
    attrs << new Attribute(BaseAttributes::FILE_MODE_ATTRIBUTE(), BaseTypes::NUM_TYPE(), false, SaveDoc_Roll);
    
    QMap<QString, PropertyDelegate*> delegateMap;
    delegateMap[BaseAttributes::URL_OUT_ATTRIBUTE().getId()] = new URLDelegate(DialogUtils::prepareDocumentsFileFilterByObjType( UHMMObject::UHMM_OT, true), HMM3_ID, false );
    delegateMap[BaseAttributes::FILE_MODE_ATTRIBUTE().getId()] = new FileModeDelegate(false);
    
    setEditor(new DelegateEditor(delegateMap));
    setIconPath( ":/hmm3/images/hmmer_16.png" );
    setValidator(new ScreenedParamValidator(BaseAttributes::URL_OUT_ATTRIBUTE().getId(), ports.first()->getId(), BaseSlots::URL_SLOT().getId()));
    setPortValidator(HMM3_IN_PORT_ID, new ScreenedSlotValidator(BaseSlots::URL_SLOT().getId()));
}

bool WriteHMM3Proto::isAcceptableDrop(const QMimeData * md, QVariantMap * params ) const {
    return HMM3IOProto::isAcceptableDrop( md, params, BaseAttributes::URL_OUT_ATTRIBUTE().getId() );
}

void HMM3IOWorkerFactory::init() {
 
     ActorPrototypeRegistry* r = WorkflowEnv::getProtoRegistry();
     assert(r);
     {
         Descriptor id(HMM3_IN_PORT_ID, HMM3Lib::tr("HMM3 profile"), HMM3Lib::tr("Input HMM3 profile"));
         Descriptor ud(BaseSlots::URL_SLOT().getId(), HMM3Lib::tr("Location"), HMM3Lib::tr("Location hint for the target file."));

         QMap<Descriptor, DataTypePtr> m;
         m[ud] = BaseTypes::STRING_TYPE();
         m[HMM3Lib::HMM3_SLOT] = HMM3Lib::HMM3_PROFILE_TYPE();
         DataTypePtr t(new MapDataType(Descriptor("write.hmm.content"), m));
        
         QList<PortDescriptor*> p; QList<Attribute*> a;
         p << new PortDescriptor(id, t, true /*input*/);
         
         Descriptor desc(HMM3Writer::ACTOR, HMM3Lib::tr("Write HMM3 Profile"), HMM3Lib::tr("Saves all input HMM3 profiles to specified location."));
         IntegralBusActorPrototype* proto = new WriteHMM3Proto(desc, p, a);
         proto->setPrompter(new HMM3WritePrompter());
         r->registerProto(HMM3Lib::HMM3_CATEGORY(), proto);
     }
     {
         Descriptor od(HMM3_OUT_PORT_ID, HMM3Lib::tr("HMM3 profile"), HMM3Lib::tr("Loaded HMM3 profile"));

         QList<PortDescriptor*> p; QList<Attribute*> a;
         QMap<Descriptor, DataTypePtr> outM;
         outM[HMM3Lib::HMM3_SLOT] = HMM3Lib::HMM3_PROFILE_TYPE();
         p << new PortDescriptor(od, DataTypePtr(new MapDataType("hmm.read.out", outM)), false /*output*/, true);
         
         Descriptor desc(HMM3Reader::ACTOR, HMM3Lib::tr("Read HMM3 Profile"), HMM3Lib::tr("Reads HMM3 profiles from file(s). The files can be local or Internet URLs."));
         IntegralBusActorPrototype* proto = new ReadHMM3Proto(desc, p, a);
         proto->setPrompter(new HMM3ReadPrompter());
         r->registerProto(HMM3Lib::HMM3_CATEGORY(), proto);
     }
 
     DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
     localDomain->registerEntry(new HMM3IOWorkerFactory(HMM3Reader::ACTOR));
     localDomain->registerEntry(new HMM3IOWorkerFactory(HMM3Writer::ACTOR));
}

QString HMM3ReadPrompter::composeRichDoc() {
    return tr("Read HMM3 profile(s) from %1").arg(getHyperlink(BaseAttributes::URL_IN_ATTRIBUTE().getId(), getURL(BaseAttributes::URL_IN_ATTRIBUTE().getId())));
}

QString HMM3WritePrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(HMM3_IN_PORT_ID));
    Actor* producer = input->getProducer(HMM3Lib::HMM3_SLOT.getId());
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString producerStr = producer ? producer->getLabel() : unsetStr;
    QString url = getScreenedURL(input, BaseAttributes::URL_OUT_ATTRIBUTE().getId(), BaseSlots::URL_SLOT().getId()); 
    url = getHyperlink(BaseAttributes::URL_OUT_ATTRIBUTE().getId(), url);
    QString doc = tr("Save HMM3 profile(s) from <u>%1</u> to <u>%2</u>.")
        .arg(producerStr)
        .arg(url);
    return doc;
}

void HMM3IOWorkerFactory::cleanup() {
     DomainFactory* ld = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
     DomainFactory* f = ld->unregisterEntry(HMM3Reader::ACTOR); 
     delete f;
     f = ld->unregisterEntry(HMM3Writer::ACTOR);
     delete f;
 
     ActorPrototypeRegistry* r = WorkflowEnv::getProtoRegistry();
     ActorPrototype* p = r->unregisterProto(HMM3Reader::ACTOR);
     assert(p); delete p;
     p = r->unregisterProto(HMM3Writer::ACTOR);
     assert(p); delete p;
}

Worker* HMM3IOWorkerFactory::createWorker(Actor* a) {
    BaseWorker* w = NULL;
     if (HMM3Reader::ACTOR == a->getProto()->getId()) {
         w = new HMM3Reader(a);
     } 
     else if (HMM3Writer::ACTOR == a->getProto()->getId()) {
         w = new HMM3Writer(a);
     }

    return w;    
}

void HMM3Reader::init() {
    output = ports.value(HMM3_OUT_PORT_ID);
    urls = WorkflowUtils::expandToUrls(actor->getParameter(BaseAttributes::URL_IN_ATTRIBUTE().getId())->getAttributeValue<QString>(context));
}

Task* HMM3Reader::tick() {
    if (urls.isEmpty()) {
        setDone();
        output->setEnded();
    } else {
        const QString& filename = urls.takeFirst();
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( IOAdapterUtils::url2io( filename  ));
        assert( NULL != iof );
        Task* t = new LoadDocumentTask( UHMMFormat::UHHMER_FORMAT_ID, filename, iof, QVariantMap() );
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
        
    }
    return NULL;
}

void HMM3Reader::sl_taskFinished() {
    LoadDocumentTask* t = qobject_cast<LoadDocumentTask*>(sender());
    if (t->getState() != Task::State_Finished) return;
    if (output) {
        if (!t->hasError()) {
            TaskStateInfo os;
            QList<const P7_HMM*> hmms = UHMM3Utilities::getHmmsFromDocument( t->getDocument(), os );
            for(int i = 0; i<hmms.size(); i++){
                QVariant v = qVariantFromValue<const P7_HMM*>(hmms[i]);
                output->put(Message(HMM3Lib::HMM3_PROFILE_TYPE(), v));
            }
        }
        ioLog.info(tr("Loaded HMM3 profile(s) from %1").arg(t->getURLString()));
    }
}

void HMM3Writer::init() {
    input = ports.value(HMM3_IN_PORT_ID);
}

Task* HMM3Writer::tick() {
    if (input->hasMessage()) {
        Message inputMessage = getMessageAndSetupScriptValues(input);
        if (inputMessage.isEmpty()) {
            return NULL;
        }
        url = getValue<QString>(BaseAttributes::URL_OUT_ATTRIBUTE().getId());
        fileMode = actor->getParameter(BaseAttributes::FILE_MODE_ATTRIBUTE().getId())->getAttributeValue<uint>(context);
        QVariantMap data = inputMessage.getData().toMap();
        
        const P7_HMM* hmm = data.value(HMM3Lib::HMM3_SLOT.getId()).value<const P7_HMM*>();
        QString anUrl = url;
        if (anUrl.isEmpty()) {
            anUrl = data.value(BaseSlots::URL_SLOT().getId()).toString();
        }
        if (anUrl.isEmpty() || hmm == NULL) {
            QString err = (hmm == NULL) ? tr("Empty HMM3 passed for writing to %1").arg(anUrl) : tr("Unspecified URL for writing HMM3");
            //if (failFast) {
                return new FailTask(err);
            /*} else {
                ioLog.error(err);
                return NULL;
            }*/
        }
        assert(!anUrl.isEmpty());
        anUrl = context->absolutePath(anUrl);
        int count = ++counter[anUrl];
        if (count != 1) {
            anUrl = GUrlUtils::prepareFileName(anUrl, count, QStringList(HMM3_EXT));
        } else {
            anUrl = GUrlUtils::ensureFileExt( anUrl, QStringList(HMM3_EXT)).getURLString();
        }
        ioLog.info(tr("Writing HMM3 profile to %1").arg(anUrl));
        Document* savingDocument = UHMM3Utilities::getSavingDocument( QList<const P7_HMM*>()<<hmm, anUrl );
        context->getMonitor()->addOutputFile(anUrl, getActor()->getId());
        //return new HMM3WriteTask(anUrl, hmm, fileMode);
        return new SaveDocumentTask( savingDocument );    
    } else if (input->isEnded()) {
        setDone();
    }
    return NULL;
}


void HMM3Lib::init() {
    HMM3IOWorkerFactory::init();
    HMM3BuildWorkerFactory::init();
    HMM3SearchWorkerFactory::init();
}

void HMM3Lib::cleanup() {
    //FIXME need locking
    //HMM3IOWorkerFactory::cleanup();
    //HMM3BuildWorkerFactory::cleanup();
    //HMM3SearchWorkerFactory::cleanup();

    //DataTypeRegistry* dr = WorkflowEnv::getDataTypeRegistry();
    //dr->unregisterEntry(HMM3_PROFILE_TYPE->getId());
}

} //namespace LocalWorkflow
} //namespace U2
