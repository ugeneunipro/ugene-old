/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include "SnpEffSupport.h"
#include "SnpEffTask.h"

#include "SnpEffWorker.h"

namespace U2 {
namespace LocalWorkflow {


const QString SnpEffWorker::BASE_SNPEFF_SUBDIR = "snpeff";
const QString SnpEffWorker::INPUT_PORT = "in-file";
const QString SnpEffWorker::OUTPUT_PORT = "out-file";
const QString SnpEffWorker::OUT_MODE_ID = "out-mode";
const QString SnpEffWorker::CUSTOM_DIR_ID = "custom-dir";

const QString SnpEffWorker::INPUT_FORMAT = "inp-format";
const QString SnpEffWorker::OUTPUT_FORMAT = "out-format";
const QString SnpEffWorker::GENOME = "genome";
const QString SnpEffWorker::UPDOWN_LENGTH = "updown-length";

const QString SnpEffWorker::CANON = "canon";
const QString SnpEffWorker::HGVS = "hgvs";
const QString SnpEffWorker::LOF = "lof";
const QString SnpEffWorker::MOTIF = "motif";

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
    Descriptor desc( ACTOR_ID, SnpEffWorker::tr("SnpEff Annotation and Filtration"),
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

        Descriptor inpFormat(SnpEffWorker::INPUT_FORMAT, SnpEffWorker::tr("Input format"),
            SnpEffWorker::tr("Select the input format of variations."));

        Descriptor outFormat(SnpEffWorker::OUTPUT_FORMAT, SnpEffWorker::tr("Output format"),
            SnpEffWorker::tr("Select the format of annotated output files."));

        Descriptor genome(SnpEffWorker::GENOME, SnpEffWorker::tr("Genome"),
            SnpEffWorker::tr("Select the target genome. Genome data will be downloaded if it is not found."));

        Descriptor updownLength(SnpEffWorker::UPDOWN_LENGTH, SnpEffWorker::tr("Upstream/downstream length"),
            SnpEffWorker::tr("Upstream and downstream interval size. Eliminate any upstream and downstream effect by using 0 length"));

        Descriptor canon(SnpEffWorker::CANON, SnpEffWorker::tr("Cannonical transcripts"),
            SnpEffWorker::tr("Use only cannonical transcripts"));

        Descriptor hgvs(SnpEffWorker::HGVS, SnpEffWorker::tr("HGVS nomenclature"),
            SnpEffWorker::tr("Annotate using HGVS nomenclature"));

        Descriptor lof(SnpEffWorker::LOF, SnpEffWorker::tr("Annotate Loss of function"),
            SnpEffWorker::tr("Annotate Loss of function (LOF) and Nonsense mediated decay (NMD)"));

        Descriptor motif(SnpEffWorker::MOTIF, SnpEffWorker::tr("Annotate TFBSs motifs"),
            SnpEffWorker::tr("Annotate transcription factor binding site motifs (only available for latest GRCh37)"));


        a << new Attribute(outDir, BaseTypes::NUM_TYPE(), false, QVariant(FileAndDirectoryUtils::WORKFLOW_INTERNAL));
        Attribute* customDirAttr = new Attribute(customDir, BaseTypes::STRING_TYPE(), false, QVariant(""));
        customDirAttr->addRelation(new VisibilityRelation(SnpEffWorker::OUT_MODE_ID, FileAndDirectoryUtils::CUSTOM));
        a << customDirAttr;

        a << new Attribute( inpFormat, BaseTypes::STRING_TYPE(), false, "vcf");
        a << new Attribute( outFormat, BaseTypes::STRING_TYPE(), false, "vcf");
        a << new Attribute( genome, BaseTypes::STRING_TYPE(), false, "hg19");
        a << new Attribute( updownLength, BaseTypes::STRING_TYPE(), false, "0");
        a << new Attribute( canon, BaseTypes::BOOL_TYPE(), false, false);
        a << new Attribute( hgvs, BaseTypes::BOOL_TYPE(), false, false);
        a << new Attribute( lof, BaseTypes::BOOL_TYPE(), false, false);
        a << new Attribute( motif, BaseTypes::BOOL_TYPE(), false, false);
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
            inFMap["VCF"] = "vcf";
            inFMap["Tabular"] = "txt";
            inFMap["Pileup"] = "pileup";
            inFMap["BED"] = "bed";
            delegates[SnpEffWorker::INPUT_FORMAT] = new ComboBoxDelegate(inFMap);
        }
        {
            QVariantMap outFMap;
            outFMap["VCF (only if VCF input)"] = "vcf";
            outFMap["Tabular"] = "txt";
            outFMap["BED"] = "bed";
            outFMap["BED Annotations"] = "bedAnn";
            delegates[SnpEffWorker::OUTPUT_FORMAT] = new ComboBoxDelegate(outFMap);
        }
        {
            QVariantMap dataMap;
            dataMap["No upstream/downstream interval (0 bases)"] = "0";
            dataMap["200 bases"] = "200";
            dataMap["500 bases"] = "500";
            dataMap["1000 bases"] = "1000";
            dataMap["5000 bases"] = "5000";
            dataMap["10000 bases"] = "10000";
            dataMap["20000 bases"] = "20000";
            delegates[SnpEffWorker::UPDOWN_LENGTH] = new ComboBoxDelegate(dataMap);
        }
        {
            QVariantMap genomeMap;
            genomeMap["Arabidopsis Thaliana (athaliana130)"] = "athaliana130";
            genomeMap["Drosophila Melanogaster (dm5.48)"] = "dm5.48";
            genomeMap["Homo sapiens (hg19)"] = "hg19";
            genomeMap["Homo sapiens (hg38)"] = "hg38";
            genomeMap["Ecoli K12 MG1655 (NC_000913)"] = "NC_000913";
            genomeMap["C. elegans (WS241)"] = "WS241";
            delegates[SnpEffWorker::GENOME] = new ComboBoxEditableDelegate(genomeMap);
        }

    }

    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new SnpEffPrompter());
    proto->addExternalTool(ET_JAVA);
    proto->addExternalTool(ET_SNPEFF);

    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_VARIATION_ANALYSIS(), proto);
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

        QString outputDir = FileAndDirectoryUtils::createWorkingDir(url, getValue<int>(OUT_MODE_ID), getValue<QString>(CUSTOM_DIR_ID), context->workingDir());
        U2OpStatusImpl os;
        outputDir = GUrlUtils::createDirectory(outputDir + SnpEffWorker::BASE_SNPEFF_SUBDIR, "_", os);

        SnpEffSetting setting;
        setting.inputUrl = url;
        setting.outDir = outputDir;
        setting.inFormat = getValue<QString>(INPUT_FORMAT);
        setting.outFormat = getValue<QString>(OUTPUT_FORMAT);
        setting.genome = getValue<QString>(GENOME);
        setting.updownLength = getValue<QString>(UPDOWN_LENGTH);
        setting.canon = getValue<bool>(CANON);
        setting.hgvs = getValue<bool>(HGVS);
        setting.lof = getValue<bool>(LOF);
        setting.motif = getValue<bool>(MOTIF);

        SnpEffTask *t = new SnpEffTask (setting);
        connect(new TaskSignalMapper(t), SIGNAL(si_taskFinished(Task*)), SLOT(sl_taskFinished(Task*)));

        QList<ExternalToolListener *> listeners = createLogListeners();
        listeners[0]->setLogProcessor(new SnpEffLogProcessor(monitor(), getActorId()));
        t->addListeners(listeners);

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
    QString getTargetTaskUrl(Task * task) {
        SnpEffTask *curtask = dynamic_cast<SnpEffTask*>(task);
        if (NULL != curtask) {
            return curtask->getResult();
        }
        return "";
    }

    QString getSummaryUrl(Task * task) {
        SnpEffTask *curtask = dynamic_cast<SnpEffTask*>(task);
        if (NULL != curtask) {
            return curtask->getSummaryUrl();
        }
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

    QString summary = getSummaryUrl(task);
    CHECK(!summary.isEmpty(), );
    monitor()->addOutputFile(summary, getActorId(), true);
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

const QStrStrMap SnpEffLogProcessor::wellKnownMessages = SnpEffLogProcessor::initWellKnownMessages();
const QMap<QString, QRegExp> SnpEffLogProcessor::messageCatchers = SnpEffLogProcessor::initWellKnownCatchers();

SnpEffLogProcessor::SnpEffLogProcessor(WorkflowMonitor *monitor, const QString &actor) :
    ExternalToolLogProcessor(),
    monitor(monitor),
    actor(actor)
{

}

void SnpEffLogProcessor::processLogMessage(const QString &message) {
    foreach (const QRegExp &catcher, messageCatchers.values()) {
        if (-1 != catcher.indexIn(message)) {
            addNotification(messageCatchers.key(catcher), catcher.cap(1).toInt());
        }
    }
}

void SnpEffLogProcessor::addNotification(const QString &key, int count) {
    SAFE_POINT(wellKnownMessages.contains(key), "An unknown snpEff internal error: " + key, );
    const QString warningMessage = key + ": " + wellKnownMessages[key] + " (count: " + QString::number(count) + ")";
    monitor->addError(warningMessage, actor, Problem::U2_WARNING);
}

QStrStrMap SnpEffLogProcessor::initWellKnownMessages() {
    QStrStrMap result;

    result["ERROR_CHROMOSOME_NOT_FOUND"] = "Chromosome does not exists in reference genome database. "
                                                      "Typically indicates a mismatch between the chromosome names "
                                                      "in the input file and the chromosome names used in the reference genome";

    result["ERROR_OUT_OF_CHROMOSOME_RANGE"] = "The variant’s genomic coordinate "
                                                         "is greater than chromosome's length";

    result["E1"] = result["ERROR_CHROMOSOME_NOT_FOUND"];
    result["E2"] = result["ERROR_OUT_OF_CHROMOSOME_RANGE"];

    result["WARNING_REF_DOES_NOT_MATCH_GENOME"] = "This means that the ‘REF’ field "
                                                               "in the input VCF file does not match the reference genome. This warning may indicate "
                                                               "a conflict between input data and data from reference genome "
                                                                "(for instance is the input VCF was aligned to a different reference genome)";

    result["WARNING_SEQUENCE_NOT_AVAILABLE"] = "Reference sequence is not available, "
                                                            "thus no inference could be performed";

    result["WARNING_TRANSCRIPT_INCOMPLETE"] = "A protein coding transcript having "
                                                           "a non-multiple of 3 length. It indicates that the reference "
                                                           "genome has missing information about this particular transcript";

    result["WARNING_TRANSCRIPT_MULTIPLE_STOP_CODONS"] = "A protein coding transcript has "
                                                                     "two or more STOP codons in the middle of the coding sequence (CDS). "
                                                                     "This should not happen and it usually means the reference genome "
                                                                     "may have an error in this transcript";

    result["WARNING_TRANSCRIPT_NO_START_CODON"] = "A protein coding transcript does not have "
                                                               "a proper START codon. It is rare that a real transcript does not have a START codon, "
                                                               "so this probably indicates an error or missing information in the reference genome";

    result["W1"] = result["WARNING_REF_DOES_NOT_MATCH_GENOME"];
    result["W2"] = result["WARNING_SEQUENCE_NOT_AVAILABLE"];
    result["W3"] = result["WARNING_TRANSCRIPT_INCOMPLETE"];
    result["W4"] = result["WARNING_TRANSCRIPT_MULTIPLE_STOP_CODONS"];
    result["W5"] = result["WARNING_TRANSCRIPT_NO_START_CODON"];

    return result;
}

QMap<QString, QRegExp> SnpEffLogProcessor::initWellKnownCatchers() {
    QMap<QString, QRegExp> result;

    foreach (const QString &message, wellKnownMessages.keys()) {
        result[message] = QRegExp(message + "\\t(\\d+)");
    }

    return result;
}

} //LocalWorkflow
} //U2
