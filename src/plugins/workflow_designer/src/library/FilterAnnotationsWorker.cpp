/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "FilterAnnotationsWorker.h"

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/TaskSignalMapper.h>

#include <U2Lang/ConfigurationEditor.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Gui/DialogUtils.h>

#include <U2Designer/DelegateEditors.h>

namespace U2 {
namespace LocalWorkflow {

const QString FilterAnnotationsWorkerFactory::ACTOR_ID("filter-annotations");

const static QString FILTER_NAMES_ATTR( "annotation-names" );
const static QString WHICH_FILTER_ATTR( "accept-or-filter" );

QString FilterAnnotationsPrompter::composeRichDoc() {
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString annName = getProducers(BasePorts::IN_ANNOTATIONS_PORT_ID(), BaseSlots::ANNOTATION_TABLE_SLOT().getId());
    annName = annName.isEmpty() ? unsetStr : annName;
    return tr("Filter annotations from <u>%1</u> by supplied names.").arg(annName);
}

void FilterAnnotationsWorker::init() {
    input = ports.value(BasePorts::IN_ANNOTATIONS_PORT_ID());
    output = ports.value(BasePorts::OUT_ANNOTATIONS_PORT_ID());
}

Task* FilterAnnotationsWorker::tick() {
    if (input->hasMessage()) {
        Message inputMessage = getMessageAndSetupScriptValues(input);
        if (inputMessage.isEmpty()) {
            output->transit();
            return NULL;
        }

        QVariantMap qm = inputMessage.getData().toMap();
        const QVariant annsVar = qm[BaseSlots::ANNOTATION_TABLE_SLOT().getId()];
        inputAnns = StorageUtils::getAnnotationTable( context->getDataStorage( ), annsVar );

        bool accept = actor->getParameter( WHICH_FILTER_ATTR )->getAttributeValue<bool>(context);
        QString namesStr = actor->getParameter( FILTER_NAMES_ATTR )->getAttributeValue<QString>(context);

        Task* t = new FilterAnnotationsTask(inputAnns, namesStr, accept);
        connect(new TaskSignalMapper(t), SIGNAL(si_taskFinished(Task*)), SLOT(sl_taskFinished(Task*)));
        return t;
    } else if (input->isEnded()) {
        setDone();
        output->setEnded();
    }
    return NULL;
}

void FilterAnnotationsWorker::sl_taskFinished(Task *t) {
    if(t->isCanceled() || t->hasError() || t->hasError()){
        return;
    }
    const SharedDbiDataHandler tableId = context->getDataStorage( )->putAnnotationTable( inputAnns );
    output->put( Message( BaseTypes::ANNOTATION_TABLE_TYPE( ),
        qVariantFromValue<SharedDbiDataHandler>( tableId ) ) );
}

void FilterAnnotationsWorker::cleanup() {
}


void FilterAnnotationsWorkerFactory::init() {
    QList<PortDescriptor*> portDescs; 
    QList<Attribute*> attribs;

    //accept sequence and annotated regions as input
    QMap<Descriptor, DataTypePtr> inputMap;
    inputMap[ BaseSlots::ANNOTATION_TABLE_SLOT() ] = BaseTypes::ANNOTATION_TABLE_TYPE();

    { //Create input port descriptors
        Descriptor inDesc( BasePorts::IN_ANNOTATIONS_PORT_ID(), FilterAnnotationsWorker::tr("Input annotations"), 
            FilterAnnotationsWorker::tr("Annotations to be filtered by name.") );
        Descriptor outDesc( BasePorts::OUT_ANNOTATIONS_PORT_ID(), FilterAnnotationsWorker::tr("Result annotations"), 
            FilterAnnotationsWorker::tr("Resulted annotations, filtered by name.") );

        portDescs << new PortDescriptor( inDesc, DataTypePtr(new MapDataType("filter.anns", inputMap)), /*input*/ true );
        portDescs << new PortDescriptor( outDesc, DataTypePtr(new MapDataType("filter.anns", inputMap)), /*input*/false, /*multi*/true );
    }

    { //Create attributes descriptors
        Descriptor filterNamesDesc( FILTER_NAMES_ATTR,
            FilterAnnotationsWorker::tr("Annotation names"), 
            FilterAnnotationsWorker::tr("File with annotation names, separated with whitespaces or list of annotation names " 
                                        "which will be accepted or filtered. Use space as the separator.") );
        Descriptor whichFilterDesc( WHICH_FILTER_ATTR,
            FilterAnnotationsWorker::tr("Accept or filter"),
            FilterAnnotationsWorker::tr("Selects the name filter: accept specified names or accept all except specified.") );

        attribs << new Attribute( filterNamesDesc, BaseTypes::STRING_TYPE(), /*required*/true );
        attribs << new Attribute( whichFilterDesc, BaseTypes::BOOL_TYPE(), /*required*/ false, QVariant(true) );
    }

    Descriptor desc( FilterAnnotationsWorkerFactory::ACTOR_ID,
        FilterAnnotationsWorker::tr("Filter Annotations by Name"), 
        FilterAnnotationsWorker::tr("Filters annotations by name.") );
    ActorPrototype * proto = new IntegralBusActorPrototype( desc, portDescs, attribs );

    proto->setPrompter( new FilterAnnotationsPrompter() );
    {
        QMap<QString, PropertyDelegate*> delegateMap;
        delegateMap[FILTER_NAMES_ATTR] = new URLDelegate(DialogUtils::prepareDocumentsFileFilter(true), QString(), true, false, false);
        proto->setEditor(new DelegateEditor(delegateMap));
    }
    WorkflowEnv::getProtoRegistry()->registerProto( BaseActorCategories::CATEGORY_BASIC(), proto );
    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById( LocalDomainFactory::ID );
    localDomain->registerEntry( new FilterAnnotationsWorkerFactory() );
}

void FilterAnnotationsTask::run() {
    QStringList names;
    if(QFileInfo(names_).exists()) { // annotation names are listed in file
        names = readAnnotationNames();
    } else { // annotation names are listed in a string
        names = names_.split( QRegExp("\\s+"), QString::SkipEmptyParts ); //split by whitespace
    }

    QMutableListIterator<AnnotationData> i(annotations_);
    while (i.hasNext()) {
        AnnotationData ad = i.next();
        if (accept_) {
            if (!names.contains(ad.name)) {
                i.remove();
            }
        } else {
            if (names.contains(ad.name)) {
                i.remove();
            }
        }
    }
}

QStringList FilterAnnotationsTask::readAnnotationNames() {
    QStringList res;
    QFile f(names_);
    if(!f.open(QFile::ReadOnly)) {
        return res;
    }
    QString data = f.readAll();
    return data.split(QRegExp("\\s+"), QString::SkipEmptyParts);
}

} // U2 namespace
} // LocalWorkflow namespace
