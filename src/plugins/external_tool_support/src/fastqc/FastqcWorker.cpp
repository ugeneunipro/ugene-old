/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentImport.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/FailTask.h>
#include <U2Core/GObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/DataPathRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Formats/BAMUtils.h>
#include <U2Core/FileAndDirectoryUtils.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowMonitor.h>

#include "java/JavaSupport.h"
#include "perl/PerlSupport.h"

#include "FastqcSupport.h"
#include "FastqcTask.h"

#include "FastqcWorker.h"

namespace U2 {
namespace LocalWorkflow {


const QString FastQCWorker::BASE_FASTQC_SUBDIR = "FastQC";
const QString FastQCWorker::INPUT_PORT = "in-file";
const QString FastQCWorker::OUT_MODE_ID = "out-mode";
const QString FastQCWorker::CUSTOM_DIR_ID = "custom-dir";

const QString FastQCWorker::ADAPTERS = "adapter";
const QString FastQCWorker::CONTAMINANTS = "contaminants";

const QString FastQCFactory::ACTOR_ID("fastqc");

////////////////////////////////////////////////
// FastQCPrompter

QString FastQCPrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(FastQCWorker::INPUT_PORT));
    const Actor* producer = input->getProducer(BaseSlots::URL_SLOT().getId());
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString producerName = tr(" from <u>%1</u>").arg(producer ? producer->getLabel() : unsetStr);

    QString doc = tr("Builds FastQC report for file(s) %1.").arg(producerName);
    return doc;
}

////////////////////////////////////////
//FastQCFactory
void FastQCFactory::init() {
    Descriptor desc( ACTOR_ID, FastQCWorker::tr("FastQC Quality Control"),
        FastQCWorker::tr("Builds quality control reports.") );

    QList<PortDescriptor*> p;
    {
        Descriptor inD(FastQCWorker::INPUT_PORT, FastQCWorker::tr("Short reads"),
            FastQCWorker::tr("Short read data"));

        QMap<Descriptor, DataTypePtr> inM;
        inM[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        p << new PortDescriptor(inD, DataTypePtr(new MapDataType("fastqc.input-url", inM)), true);

    }

    QList<Attribute*> a;
    {

        Descriptor outDir(FastQCWorker::OUT_MODE_ID, FastQCWorker::tr("Output directory"),
            FastQCWorker::tr("Select an output directory. <b>Custom</b> - specify the output directory in the 'Custom directory' parameter. "
            "<b>Workflow</b> - internal workflow directory. "
            "<b>Input file</b> - the directory of the input file."));

        Descriptor customDir(FastQCWorker::CUSTOM_DIR_ID, FastQCWorker::tr("Custom directory"),
            FastQCWorker::tr("Select the custom output directory."));

        Descriptor adapters(FastQCWorker::ADAPTERS, FastQCWorker::tr("List of adapters"),
            FastQCWorker::tr("Specifies a non-default file which contains the list of adapter sequences which will be explicity searched against "
                                             "the library. The file must contain sets of named adapters "
                                             "in the form name[tab]sequence.  Lines prefixed with a hash "
                                             "will be ignored."));

        Descriptor conts(FastQCWorker::CONTAMINANTS, FastQCWorker::tr("List of contaminants"),
            FastQCWorker::tr("Specifies a non-default file which contains the list of "
                             "contaminants to screen overrepresented sequences against. "
                                             "The file must contain sets of named contaminants in the "
                                             "form name[tab]sequence.  Lines prefixed with a hash will "
                                             "be ignored."));


        a << new Attribute(outDir, BaseTypes::NUM_TYPE(), false, QVariant(FileAndDirectoryUtils::WORKFLOW_INTERNAL));
        Attribute* customDirAttr = new Attribute(customDir, BaseTypes::STRING_TYPE(), false, QVariant(""));
        customDirAttr->addRelation(new VisibilityRelation(FastQCWorker::OUT_MODE_ID, FileAndDirectoryUtils::CUSTOM));
        a << customDirAttr;

        a << new Attribute( adapters, BaseTypes::STRING_TYPE(), false, "");
        a << new Attribute( conts, BaseTypes::STRING_TYPE(), false, "");
    }

    QMap<QString, PropertyDelegate*> delegates;
    {

        QVariantMap directoryMap;
        QString fileDir = FastQCWorker::tr("Input file");
        QString workflowDir = FastQCWorker::tr("Workflow");
        QString customD = FastQCWorker::tr("Custom");
        directoryMap[fileDir] = FileAndDirectoryUtils::FILE_DIRECTORY;
        directoryMap[workflowDir] = FileAndDirectoryUtils::WORKFLOW_INTERNAL;
        directoryMap[customD] = FileAndDirectoryUtils::CUSTOM;
        delegates[FastQCWorker::OUT_MODE_ID] = new ComboBoxDelegate(directoryMap);

        delegates[FastQCWorker::CUSTOM_DIR_ID] = new URLDelegate("", "", false, true);

        delegates[FastQCWorker::ADAPTERS] = new URLDelegate("", "", false, false, false);
        delegates[FastQCWorker::CONTAMINANTS] = new URLDelegate("", "", false, false, false);
    }

    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new FastQCPrompter());
    proto->addExternalTool(ET_JAVA);
    proto->addExternalTool(ET_FASTQC);
    proto->addExternalTool(ET_PERL);

    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_NGS_BASIC(), proto);
    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new FastQCFactory());
}

//////////////////////////////////////////////////////////////////////////
//FastQCWorker
FastQCWorker::FastQCWorker(Actor *a)
:BaseWorker(a)
,inputUrlPort(NULL)
{

}

void FastQCWorker::init() {
    inputUrlPort = ports.value(INPUT_PORT);
}

Task * FastQCWorker::tick() {
    if (inputUrlPort->hasMessage()) {
        const QString url = takeUrl();
        CHECK(!url.isEmpty(), NULL);

        QString outputDir = FileAndDirectoryUtils::createWorkingDir(url, getValue<int>(OUT_MODE_ID), getValue<QString>(CUSTOM_DIR_ID), context->workingDir());
        U2OpStatusImpl os;
        outputDir = GUrlUtils::createDirectory(outputDir + FastQCWorker::BASE_FASTQC_SUBDIR, "_", os);

        FastQCSetting setting;
        setting.inputUrl = url;
        setting.outDir = outputDir;
        setting.adapters = getValue<QString>(ADAPTERS);
        setting.conts = getValue<QString>(CONTAMINANTS);

        FastQCTask *t = new FastQCTask (setting);
        t->addListeners(createLogListeners());
        connect(new TaskSignalMapper(t), SIGNAL(si_taskFinished(Task*)), SLOT(sl_taskFinished(Task*)));
        return t;
    }

    if (inputUrlPort->isEnded()) {
        setDone();
    }
    return NULL;
}

void FastQCWorker::cleanup(){

}

namespace {
    QString getTargetTaskUrl(Task * task) {
        FastQCTask *curtask = dynamic_cast<FastQCTask*>(task);
        if (NULL != curtask) {
            return curtask->getResult();
        }
        return "";
    }

}

void FastQCWorker::sl_taskFinished(Task *task) {
    CHECK(!task->hasError(), );
    CHECK(!task->isCanceled(), );

    QString url = getTargetTaskUrl(task);
    CHECK(!url.isEmpty(), );

    monitor()->addOutputFile(url, getActorId(), true);
}

QString FastQCWorker::takeUrl() {
    const Message inputMessage = getMessageAndSetupScriptValues(inputUrlPort);
    if (inputMessage.isEmpty()) {
        return "";
    }

    const QVariantMap data = inputMessage.getData().toMap();
    return data[BaseSlots::URL_SLOT().getId()].toString();
}

} //LocalWorkflow
} //U2

