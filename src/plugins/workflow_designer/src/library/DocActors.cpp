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

#include "DocActors.h"
#include "CoreLib.h"

#include <U2Designer/DelegateEditors.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Gui/DialogUtils.h>

namespace U2 {
namespace Workflow {

/****************************
 * DocActorProto
 *****************************/
DocActorProto::DocActorProto(const DocumentFormatId& _fid, const Descriptor& _desc, const QList<PortDescriptor*>& _ports, 
                             const QList<Attribute*>& _attrs ) : IntegralBusActorPrototype(_desc, _ports, _attrs), fid(_fid) {
}

DocActorProto::DocActorProto(const Descriptor& _desc, const GObjectType& t, const QList<PortDescriptor*>& _ports,
                             const QList<Attribute*>& _attrs ) : IntegralBusActorPrototype(_desc, _ports, _attrs), type(t) {
}

bool DocActorProto::isAcceptableDrop(const QMimeData * md, QVariantMap * params, const QString & urlAttrId ) const {
    QList<DocumentFormat*> fs;
    QString url = WorkflowUtils::getDropUrl(fs, md);
    foreach(DocumentFormat* df, fs) {
        if (fid == df->getFormatId()) {
            if (params) {
                params->insert( urlAttrId, url );
            }
            return true;
        }
    }
    return false;
}

QString DocActorProto::prepareDocumentFilter() {
    if( !fid.isEmpty() ) {
        return DialogUtils::prepareDocumentsFileFilter( fid, true );
    } else {
        assert( !type.isEmpty() );
        return DialogUtils::prepareDocumentsFileFilterByObjType( type, true );
    }
}

/****************************
 * ReadDocActorProto
 *****************************/
ReadDocActorProto::ReadDocActorProto(const DocumentFormatId& _fid, const Descriptor& _desc, const QList<PortDescriptor*>& _ports, 
                                     const QList<Attribute*>& _attrs ) : DocActorProto( _fid, _desc, _ports, _attrs ) {
    attrs << new Attribute( BaseAttributes::URL_IN_ATTRIBUTE(), BaseTypes::STRING_TYPE(), true );
    QMap< QString, PropertyDelegate* > delegateMap;
    delegateMap[BaseAttributes::URL_IN_ATTRIBUTE().getId()] = new URLDelegate( prepareDocumentFilter(), QString(), true );
    setEditor( new DelegateEditor( delegateMap ) );
}

bool ReadDocActorProto::isAcceptableDrop(const QMimeData * md, QVariantMap * params ) const {
    return DocActorProto::isAcceptableDrop( md, params, BaseAttributes::URL_IN_ATTRIBUTE().getId() );
}

/****************************
 * WriteDocActorProto
 *****************************/
WriteDocActorProto::WriteDocActorProto(const DocumentFormatId& _fid, const Descriptor& _desc, const QList<PortDescriptor*>& _ports, 
                                       const QString & portId, const QList<Attribute*>& _attrs)
: DocActorProto( _fid, _desc, _ports, _attrs ), outPortId(portId), urlDelegate(NULL) {
    construct();
}

WriteDocActorProto::WriteDocActorProto(const Descriptor& _desc, const GObjectType & t, const QList<PortDescriptor*>& _ports, 
                                       const QString & portId, const QList<Attribute*>& _attrs)
: DocActorProto(_desc, t, _ports, _attrs), outPortId(portId), urlDelegate(NULL) {
    construct();
}

bool WriteDocActorProto::isAcceptableDrop(const QMimeData * md, QVariantMap * params ) const {
    return DocActorProto::isAcceptableDrop( md, params, BaseAttributes::URL_OUT_ATTRIBUTE().getId() );
}

void WriteDocActorProto::construct() {
    attrs << new Attribute(BaseAttributes::URL_OUT_ATTRIBUTE(), BaseTypes::STRING_TYPE(), false );
    attrs << new Attribute(BaseAttributes::FILE_MODE_ATTRIBUTE(), BaseTypes::NUM_TYPE(), false, SaveDoc_Roll);

    QMap< QString, PropertyDelegate* > delegateMap;
    urlDelegate = new URLDelegate(prepareDocumentFilter(), QString(), false, false, true, 0, fid);
    delegateMap[BaseAttributes::URL_OUT_ATTRIBUTE().getId()] = urlDelegate;
    delegateMap[BaseAttributes::FILE_MODE_ATTRIBUTE().getId()] = new FileModeDelegate(attrs.size() > 2);

    setEditor(new DelegateEditor(delegateMap));
    setValidator(new ScreenedParamValidator(BaseAttributes::URL_OUT_ATTRIBUTE().getId(), ports.first()->getId(), BaseSlots::URL_SLOT().getId()));
    setPortValidator(outPortId, new ScreenedSlotValidator(BaseSlots::URL_SLOT().getId()));
}

URLDelegate *WriteDocActorProto::getUrlDelegate() {
    return urlDelegate;
}

/****************************
 * WriteGenbankPrompter
 *****************************/
QString WriteGenbankPrompter::composeRichDoc() {
    QString outPortId = target->getInputPorts().first()->getId();
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(outPortId));
    Actor* seqProducer = input->getProducer(BaseSlots::DNA_SEQUENCE_SLOT().getId());
    QString seqName = seqProducer ? tr(" sequence from <u>%1</u>").arg(seqProducer->getLabel()) : "";
    QString annName = getProducers(outPortId, BaseSlots::ANNOTATION_TABLE_SLOT().getId());
    if (!annName.isEmpty()) {
        annName = tr(" set of annotations from <u>%1</u>").arg(annName);
    }
    
    QString url = getScreenedURL( qobject_cast<IntegralBusPort*>(target->getPort(outPortId)), 
        BaseAttributes::URL_OUT_ATTRIBUTE().getId(), BaseSlots::URL_SLOT().getId() );
    url = getHyperlink(BaseAttributes::URL_OUT_ATTRIBUTE().getId(), url);
    
    QString data;
    if (seqName.isEmpty() && annName.isEmpty()) {
        QString doc = tr("Write sequence(s) in Genbank format to <u>%1</u>.").arg(url);
        return doc;
    } else if (!seqName.isEmpty() && !annName.isEmpty()) {
        data = tr("each %1 and %2").arg(seqName).arg(annName);
    } else {
        data = tr("each ") + seqName + annName;
    }
    
    return tr("Write %1 in Genbank format, to <u>%2</u>.")
        .arg(data)
        .arg(url);
}

/****************************
 * WriteFastaPrompter
 *****************************/
QString WriteFastaPrompter::composeRichDoc() {
    QString outPortId = target->getInputPorts().first()->getId();
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(outPortId));
    QString url = getScreenedURL( qobject_cast<IntegralBusPort*>(target->getPort(outPortId)),
        BaseAttributes::URL_OUT_ATTRIBUTE().getId(), BaseSlots::URL_SLOT().getId() );
    url = getHyperlink(BaseAttributes::URL_OUT_ATTRIBUTE().getId(), url);

    Actor* seqProducer = input->getProducer(BaseSlots::DNA_SEQUENCE_SLOT().getId());
    if (!seqProducer) {
        QString doc = tr("Writes sequence(s) in %1 format to <u>%2</u>.").arg(format).arg(url);
        return doc;
    }

    QString doc = tr("Writes sequence(s) from <u>%1</u> in %2 format to  <u>%3</u>.")
        .arg(seqProducer->getLabel())
        .arg(format)
        .arg(url);

    return doc;
}

ActorDocument* WriteFastaPrompter::createDescription(Actor* a) {
    WriteFastaPrompter* doc = new WriteFastaPrompter(format, a);
    doc->connect(a, SIGNAL(si_labelChanged()), SLOT(sl_actorModified()));
    doc->connect(a, SIGNAL(si_modified()), SLOT(sl_actorModified()));
    foreach(Workflow::Port* input, a->getInputPorts()) {
        doc->connect(input, SIGNAL(bindingChanged()), SLOT(sl_actorModified()));
    }
    return doc;
}

/****************************
 * WriteDocPrompter
 *****************************/
QString WriteDocPrompter::composeRichDoc() {
    QString outPortId = target->getInputPorts().first()->getId();
    QString url = getScreenedURL( qobject_cast<IntegralBusPort*>(target->getPort(outPortId)), 
        BaseAttributes::URL_OUT_ATTRIBUTE().getId(), BaseSlots::URL_SLOT().getId() );
    url = getHyperlink(BaseAttributes::URL_OUT_ATTRIBUTE().getId(), url);
    
    QString producers = getProducers(outPortId, slot);
    if (producers.isEmpty()) {
        QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
        return spec.arg(unsetStr).arg(url);
    }
    return spec.arg(producers).arg(url);
}

/****************************
 * ReadDocPrompter
 *****************************/
QString ReadDocPrompter::composeRichDoc() {
    const QString& id = BaseAttributes::URL_IN_ATTRIBUTE().getId();
    return spec.arg(getHyperlink(id, getURL(id)));
}

}//namespace Workflow
}//namespace U2
