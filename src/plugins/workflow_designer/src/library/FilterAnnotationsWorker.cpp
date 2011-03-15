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

#include "FilterAnnotationsWorker.h"

#include <U2Core/TaskSignalMapper.h>

#include <U2Lang/ConfigurationEditor.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>

#include <U2Designer/DelegateEditors.h>


namespace U2 {
namespace LocalWorkflow {

const QString FilterAnnotationsWorkerFactory::ACTOR_ID("filter-annotations");

const static QString FILTER_NAMES_ATTR( "annotation-names" );
const static QString WHICH_FILTER_ATTR( "accept-or-filter" );

QString FilterAnnotationsPrompter::composeRichDoc() {
    ////extract names. use max 2 names from the list
    //bool useAccepted = getParameter( WHICH_FILTER_ATTR ).toBool();
    //QString filterText;
    //QString names;
    //if( useAccepted ) {
    //    filterText += tr( " which name is " );
    //    names = getParameter( FILTER_NAMES_ATTR ).toString();
    //} else {
    //    filterText += tr( ", which name is not " );
    //    names = getParameter( FILTER_NAMES_ATTR ).toString();
    //}
    //QStringList annNamesLit = names.split( QRegExp("\\s+"), QString::SkipEmptyParts );
    //const int maxNamesMention = 2;
    //for( int i = 0, end = qMin(annNamesLit.size(), maxNamesMention); i < end; ++i ) {
    //    filterText += tr( "\"<u>%1</u>\"" ).arg(annNamesLit.at(i));
    //    filterText += ", ";
    //}
    //if( annNamesLit.size() > maxNamesMention ) {
    //    filterText.remove( filterText.size()-1, 2 ); //remove ", "
    //    filterText += tr(" or others, ");
    //} else if( annNamesLit.empty() ) {
    //    filterText = ",";
    //}
    return tr("Filters annotations by name.");
}

void FilterAnnotationsWorker::init() {
    input = ports.value(BasePorts::IN_ANNOTATIONS_PORT_ID());
    output = ports.value(BasePorts::OUT_ANNOTATIONS_PORT_ID());
}

bool FilterAnnotationsWorker::isReady() {
    return input->hasMessage();
}

Task* FilterAnnotationsWorker::tick() {
    Message inputMessage = getMessageAndSetupScriptValues(input);
    QVariantMap qm = inputMessage.getData().toMap();
    inputAnns = qVariantValue<QList<SharedAnnotationData> >( qm.value(BaseSlots::ANNOTATION_TABLE_SLOT().getId()) );

    bool accept = actor->getParameter( WHICH_FILTER_ATTR )->getAttributeValue<bool>();
    QString namesStr = actor->getParameter( FILTER_NAMES_ATTR )->getAttributeValue<QString>();
    QStringList names = namesStr.split( QRegExp("\\s+"), QString::SkipEmptyParts ); //split by whitespace

    Task* t = new FilterAnnotationsTask(inputAnns, names, accept);
    connect(new TaskSignalMapper(t), SIGNAL(si_taskFinished(Task*)), SLOT(sl_taskFinished()));
    return t;
}

void FilterAnnotationsWorker::sl_taskFinished() {
    output->put( Message(BaseTypes::ANNOTATION_TABLE_TYPE(), qVariantFromValue(inputAnns)) );
    if (input->isEnded()) {
        output->setEnded();
    }
}

bool FilterAnnotationsWorker::isDone() {
    return input->isEnded();
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
            FilterAnnotationsWorker::tr("List of annotation names which will be accepted or filtered. Use space as the separator.") );
        Descriptor whichFilterDesc( WHICH_FILTER_ATTR,
            FilterAnnotationsWorker::tr("Accept or filter"),
            FilterAnnotationsWorker::tr("Selects the name filter: accept specified names or accept all except specified") );

        attribs << new Attribute( filterNamesDesc, BaseTypes::STRING_TYPE(), /*required*/true );
        attribs << new Attribute( whichFilterDesc, BaseTypes::BOOL_TYPE(), /*required*/ false, QVariant(true) );
    }

    Descriptor desc( FilterAnnotationsWorkerFactory::ACTOR_ID,
        FilterAnnotationsWorker::tr("Filter annotations by name"), 
        FilterAnnotationsWorker::tr("Filters annotations by name") );
    ActorPrototype * proto = new IntegralBusActorPrototype( desc, portDescs, attribs );

    proto->setPrompter( new FilterAnnotationsPrompter() );
    proto->setEditor(new DelegateEditor(QMap<QString, PropertyDelegate*>()));
    WorkflowEnv::getProtoRegistry()->registerProto( BaseActorCategories::CATEGORY_BASIC(), proto );

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById( LocalDomainFactory::ID );
    localDomain->registerEntry( new FilterAnnotationsWorkerFactory() );
}


void FilterAnnotationsTask::run() {
    QMutableListIterator<SharedAnnotationData> i(annotations_);
    while (i.hasNext()) {
        SharedAnnotationData ad = i.next();
        if (accept_) {
            if (!names_.contains(ad->name)) {
                i.remove();
            }
        } else {
            if (names_.contains(ad->name)) {
                i.remove();
            }
        }
    }
}

} // U2 namespace
} // LocalWorkflow namespace
