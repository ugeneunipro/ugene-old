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

#include "SnpEffSupport.h"
#include "SnpEffTask.h"

#include "SnpEffWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString SnpEffWorker::INPUT_PORT = "in-file";
const QString SnpEffWorker::OUTPUT_PORT = "out-file";
const QString SnpEffWorker::OUT_MODE_ID = "out-mode";
const QString SnpEffWorker::CUSTOM_DIR_ID = "custom-dir";

const QString SnpEffWorker::INPUT_FORMAT = "inp-format";
const QString SnpEffWorker::OUTPUT_FORMAT = "out-format";
const QString SnpEffWorker::GENOME = "genome";
const QString SnpEffWorker::UPDOWN_LENGTH = "updown-length";
const QString SnpEffWorker::HOMOHETERO_CHANGES = "homoheter-changes";
const QString SnpEffWorker::SEQ_CHANGES = "seq-changes";
const QString SnpEffWorker::FILTER_OUTPUT = "filter-out";
const QString SnpEffWorker::CHR_POS = "chr-pos";

const QString SnpEffFactory::ACTOR_ID("seff");


////////////////////////////////////////////////
// SnpEffPrompter

QString SnpEffPrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(SnpEffWorker::INPUT_PORT));
    const Actor* producer = input->getProducer(BaseSlots::URL_SLOT().getId());
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString producerName = tr(" from <u>%1</u>").arg(producer ? producer->getLabel() : unsetStr);

    QString doc = tr("Annotates and filters variations %1 with SnpEff.").arg(producerName);
    return doc;
}

////////////////////////////////////////
//SnpEffFactory
void SnpEffFactory::init() {
    //init data path
    /*
    U2DataPath* dataPath = NULL;
    U2DataPathRegistry* dpr =  AppContext::getDataPathRegistry();
    if (dpr){
        U2DataPath* dp = dpr->getDataPathByName(GENOMES_DATA_NAME);
        if (dp && dp->isValid()){
            dataPath = dp;
        }
    }
    */
    Descriptor desc( ACTOR_ID, SnpEffWorker::tr("SnpEff annotation and filtration"),
        SnpEffWorker::tr("Annotates and filters variations with SnpEff.") );

    QList<PortDescriptor*> p;
    {
        Descriptor inD(SnpEffWorker::INPUT_PORT, SnpEffWorker::tr("Variations"),
            SnpEffWorker::tr("Set of variations"));
        Descriptor outD(SnpEffWorker::OUTPUT_PORT, SnpEffWorker::tr("Annotated variations"),
            SnpEffWorker::tr("Annotated variations"));

        QMap<Descriptor, DataTypePtr> inM;
        inM[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        p << new PortDescriptor(inD, DataTypePtr(new MapDataType("seff.input-url", inM)), true);

        QMap<Descriptor, DataTypePtr> outM;
        outM[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        p << new PortDescriptor(outD, DataTypePtr(new MapDataType("seff.output-url", outM)), false, true);
    }

    QList<Attribute*> a;
    {

        Descriptor outDir(SnpEffWorker::OUT_MODE_ID, SnpEffWorker::tr("Output directory"),
            SnpEffWorker::tr("Select an output directory. <b>Custom</b> - specify the output directory in the 'Custom directory' parameter. "
            "<b>Workflow</b> - internal workflow directory. "
            "<b>Input file</b> - the directory of the input file."));

        Descriptor customDir(SnpEffWorker::CUSTOM_DIR_ID, SnpEffWorker::tr("Custom directory"),
            SnpEffWorker::tr("Select the custom output directory."));

        /*
        Descriptor genomeAttrDesc(SnpEffWorker::GENOME, SnpEffWorker::tr("Genome"),
            SnpEffWorker::tr("File with genome length."));

        a << new Attribute( outDir, BaseTypes::NUM_TYPE(), false, QVariant(FileAndDirectoryUtils::FILE_DIRECTORY));
        Attribute* customDirAttr = new Attribute(customDir, BaseTypes::STRING_TYPE(), false, QVariant(""));
        customDirAttr->addRelation(new VisibilityRelation(SnpEffWorker::OUT_MODE_ID, FileAndDirectoryUtils::CUSTOM));
        a << customDirAttr;
        Attribute* genomeAttr = NULL;
        if (dataPath){
            const QList<QString>& dataNames = dataPath->getDataNames();
            if (!dataNames.isEmpty()){
                genomeAttr = new Attribute(genomeAttrDesc, BaseTypes::STRING_TYPE(), true, dataPath->getPathByName(dataNames.first()));
            }else{
                genomeAttr = new Attribute(genomeAttrDesc, BaseTypes::STRING_TYPE(), true);
            }
        }else{
            genomeAttr = new Attribute(genomeAttrDesc, BaseTypes::STRING_TYPE(), true);
        }
        a << genomeAttr;*/

        Descriptor inpFormat(SnpEffWorker::INPUT_FORMAT, SnpEffWorker::tr("Input format"),
            SnpEffWorker::tr("Select the input format of variations."));

        Descriptor outFormat(SnpEffWorker::OUTPUT_FORMAT, SnpEffWorker::tr("Output format"),
            SnpEffWorker::tr("Select the format of annotated output files."));

        Descriptor chrPos(SnpEffWorker::CHR_POS, SnpEffWorker::tr("Chromosome positions"),
            SnpEffWorker::tr("Select indexing type."));

        Descriptor genome(SnpEffWorker::GENOME, SnpEffWorker::tr("Genome"),
            SnpEffWorker::tr("Select the target genome. Genome data will be downloaded if it is not found."));

        Descriptor updownLength(SnpEffWorker::UPDOWN_LENGTH, SnpEffWorker::tr("Upstream/downstream length"),
            SnpEffWorker::tr("Upstream and downstream interval size. Eliminate any upstream and downstream effect by using 0 length"));

        Descriptor honoheteroFilter(SnpEffWorker::HOMOHETERO_CHANGES, SnpEffWorker::tr("Filter by homozygous/heterozygous type"),
            SnpEffWorker::tr("Select if filter homozygous / heterozygous changes"));

        Descriptor outFilter(SnpEffWorker::FILTER_OUTPUT, SnpEffWorker::tr("Post-annotation filter"),
            SnpEffWorker::tr("Select if filter after annotation"));

        a << new Attribute( outDir, BaseTypes::NUM_TYPE(), false, QVariant(FileAndDirectoryUtils::FILE_DIRECTORY));
        Attribute* customDirAttr = new Attribute(customDir, BaseTypes::STRING_TYPE(), false, QVariant(""));
        customDirAttr->addRelation(new VisibilityRelation(SnpEffWorker::OUT_MODE_ID, FileAndDirectoryUtils::CUSTOM));
        a << customDirAttr;

        a << new Attribute( inpFormat, BaseTypes::STRING_TYPE(), false, "VCF");
        a << new Attribute( outFormat, BaseTypes::STRING_TYPE(), false, "VCF");
        a << new Attribute( chrPos, BaseTypes::STRING_TYPE(), false, "");
        a << new Attribute( genome, BaseTypes::STRING_TYPE(), false, "");
        a << new Attribute( updownLength, BaseTypes::STRING_TYPE(), false, "");
        a << new Attribute( honoheteroFilter, BaseTypes::STRING_TYPE(), false, "");
        a << new Attribute( outFilter, BaseTypes::STRING_TYPE(), false, "");
    }

    QMap<QString, PropertyDelegate*> delegates;
    {

        QVariantMap directoryMap;
        QString fileDir = SnpEffWorker::tr("Input file");
        QString workflowDir = SnpEffWorker::tr("Workflow");
        QString customD = SnpEffWorker::tr("Custom");
        directoryMap[fileDir] = FileAndDirectoryUtils::FILE_DIRECTORY;
        directoryMap[workflowDir] = FileAndDirectoryUtils::WORKFLOW_INTERNAL;
        directoryMap[customD] = FileAndDirectoryUtils::CUSTOM;
        delegates[SnpEffWorker::OUT_MODE_ID] = new ComboBoxDelegate(directoryMap);

        delegates[SnpEffWorker::CUSTOM_DIR_ID] = new URLDelegate("", "", false, true);

        {
            QVariantMap inFMap;
            inFMap["VCF"] = "VCF";
            inFMap["Tabular"] = "Tabular";
            inFMap["Pileup"] = "Pileup";
            inFMap["BED"] = "BED";
            delegates[SnpEffWorker::INPUT_FORMAT] = new ComboBoxDelegate(inFMap);
        }
        {
            QVariantMap outFMap;
            outFMap["VCF (only if VCF input)"] = "VCF";
            outFMap["Tabular"] = "Tabular";
            outFMap["BED"] = "BED";
            outFMap["BED Annotations"] = "BED Annotations";
            delegates[SnpEffWorker::OUTPUT_FORMAT] = new ComboBoxDelegate(outFMap);
        }
        {
            QVariantMap dataMap;
            dataMap["Use default (based on input type)"] = "Use default (based on input type)";
            dataMap["Force zero-based positions (both input and output)"] = "Force zero-based positions (both input and output)";
            dataMap["Force one-based positions (both input and output)"] = "Force one-based positions (both input and output)";
            delegates[SnpEffWorker::CHR_POS] = new ComboBoxDelegate(dataMap);
        }
        {
            QVariantMap dataMap;
            dataMap["No filter (analyze everything)"] = "No filter (analyze everything)";
            dataMap["Analyze homozygous sequence changes only"] = "Analyze homozygous sequence changes only";
            dataMap["Analyze heterozygous sequence changes only"] = "Analyze heterozygous sequence changes only";
            delegates[SnpEffWorker::HOMOHETERO_CHANGES] = new ComboBoxDelegate(dataMap);
        }
        {
            QVariantMap dataMap;
            dataMap["No upstream/downstream interval (0 bases)"] = "No upstream/downstream interval (0 bases)";
            dataMap["200 bases"] = "200 bases";
            dataMap["500 bases"] = "500 bases";
            dataMap["1000 bases"] = "1000 bases";
            dataMap["5000 bases"] = "5000 bases";
            dataMap["10000 bases"] = "10000 bases";
            dataMap["20000 bases"] = "20000 bases";
            delegates[SnpEffWorker::UPDOWN_LENGTH] = new ComboBoxDelegate(dataMap);
        }

        {
            QVariantMap dataMap;
            dataMap["No filter (analyze everything)"] = "No filter (analyze everything)";
            dataMap["Analyze deletions only"] = "Analyze deletions only";
            dataMap["Analyze insertions only"] = "Analyze insertions only";
            dataMap["Only MNPs (multiple nucleotide polymorphisms)"] = "Only MNPs (multiple nucleotide polymorphisms)";
            dataMap["Only SNPs (single nucleotide polymorphisms)"] = "Only SNPs (single nucleotide polymorphisms)";
            delegates[SnpEffWorker::SEQ_CHANGES] = new ComboBoxDelegate(dataMap);
        }

        {
            QVariantMap dataMap;
            dataMap["Do not show DOWNSTREAM changes"] = "Do not show DOWNSTREAM changes";
            dataMap["Do not show INTERGENIC changes"] = "Do not show INTERGENIC changes";
            dataMap["Do not show INTRON changes"] = "Do not show INTRON changes";
            dataMap["Do not show UPSTREAM changes"] = "Do not show UPSTREAM changes";
            dataMap["Do not show 5_PRIME_UTR or 3_PRIME_UTR changes"] = "Do not show 5_PRIME_UTR or 3_PRIME_UTR changes";
            delegates[SnpEffWorker::FILTER_OUTPUT] = new ComboBoxWithChecksDelegate(dataMap);
        }

        {
            QVariantMap genomeMap;
            genomeMap["athaliana130"] = "athaliana130";
            genomeMap["dm5.48"] = "dm5.48";
            genomeMap["hg19"] = "hg19";
            genomeMap["hg38"] = "hg38";
            genomeMap["NC_000913"] = "NC_000913";
            genomeMap["WS241"] = "WS241";

            //TODO make editable combobox
            delegates[SnpEffWorker::GENOME] = new ComboBoxEditableDelegate(genomeMap);
        }

    }

    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new SnpEffPrompter());
    proto->addExternalTool(ET_SNPEFF);

    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_CALL_VARIATIONS(), proto);
    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new SnpEffFactory());
}

//////////////////////////////////////////////////////////////////////////
//SnpEffWorker
SnpEffWorker::SnpEffWorker(Actor *a)
:BaseWorker(a)
,inputUrlPort(NULL)
,outputUrlPort(NULL)
{

}

void SnpEffWorker::init() {
    inputUrlPort = ports.value(INPUT_PORT);
    outputUrlPort = ports.value(OUTPUT_PORT);
}

Task * SnpEffWorker::tick() {

    if (inputUrlPort->hasMessage()) {
        const QString url = takeUrl();
        CHECK(!url.isEmpty(), NULL);

        const QString outputDir = FileAndDirectoryUtils::createWorkingDir(url, getValue<int>(OUT_MODE_ID), getValue<QString>(CUSTOM_DIR_ID), context->workingDir());


        SnpEffSetting setting;
        setting.inputUrl = url;
        setting.outDir = outputDir;
        setting.genome = getValue<QString>(GENOME);
        setting.updownLength = getValue<QString>(UPDOWN_LENGTH);
        setting.homohetero = getValue<QString>(HOMOHETERO_CHANGES);
        setting.seqChange = getValue<QString>(SEQ_CHANGES);
        setting.filterOut = getValue<QString>(FILTER_OUTPUT);
        setting.chrPos = getValue<QString>(CHR_POS);

        Task *t = new SnpEffTask (setting);
        connect(new TaskSignalMapper(t), SIGNAL(si_taskFinished(Task*)), SLOT(sl_taskFinished(Task*)));
        return t;
    }

    if (inputUrlPort->isEnded()) {
        setDone();
        outputUrlPort->setEnded();
    }
    return NULL;
}

void SnpEffWorker::cleanup(){

}

namespace {
    QString getTargetTaskUrl(Task * /*task*/) {

        /*
        SnpEffTask *curtask = dynamic_cast<SnpEffTask*>(task);

        if (NULL != curtask) {
            return curtask->getResult();
        }*/
        return "";
    }
}

void SnpEffWorker::sl_taskFinished(Task *task) {
    CHECK(!task->hasError(), );
    CHECK(!task->isCanceled(), );

    QString url = getTargetTaskUrl(task);
    CHECK(!url.isEmpty(), );

    sendResult(url);
    monitor()->addOutputFile(url, getActorId());
}

QString SnpEffWorker::getTargetName (const QString & /*fileUrl*/, const QString & /*outDir*/){
    /*
    QString name = getValue<QString>(OUT_NAME_ID);

    if(name == DEFAULT_NAME || name.isEmpty()){
        name = QFileInfo(fileUrl).fileName();
        name = name + getDefaultFileName();
    }
    if(outUrls.contains(outDir + name)){
        name.append(QString("_%1").arg(outUrls.size()));
    }
    outUrls.append(outDir+name);
    return name;
    */
    return "";
}


QString SnpEffWorker::takeUrl() {
    const Message inputMessage = getMessageAndSetupScriptValues(inputUrlPort);
    if (inputMessage.isEmpty()) {
        outputUrlPort->transit();
        return "";
    }

    const QVariantMap data = inputMessage.getData().toMap();
    return data[BaseSlots::URL_SLOT().getId()].toString();
}

void SnpEffWorker::sendResult(const QString &url) {
    const Message message(BaseTypes::STRING_TYPE(), url);
    outputUrlPort->put(message);
}

} //LocalWorkflow
} //U2
