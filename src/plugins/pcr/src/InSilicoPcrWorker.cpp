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
#include <U2Core/DNAAlphabet.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/MultiTask.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowMonitor.h>

#include "ExtractProductTask.h"
#include "PrimersGrouperWorker.h"

#include "InSilicoPcrWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString InSilicoPcrWorkerFactory::ACTOR_ID = "in-silico-pcr";
namespace {
    const QString OUT_PORT_ID = "out";
    const QString PRIMERS_ATTR_ID = "primers-url";
    const QString REPORT_ATTR_ID = "report-url";
    const QString MISMATCHES_ATTR_ID = "mismatches";
    const QString PERFECT_ATTR_ID = "perfect-match";
    const QString MAX_PRODUCT_ATTR_ID = "max-product";

    const char * PAIR_NUMBER_PROP_ID = "pair-number";
}

/************************************************************************/
/* InSilicoPcrWorkerFactory */
/************************************************************************/
InSilicoPcrWorkerFactory::InSilicoPcrWorkerFactory()
: DomainFactory(ACTOR_ID)
{

}

Worker * InSilicoPcrWorkerFactory::createWorker(Actor *a) {
    return new InSilicoPcrWorker(a);
}

void InSilicoPcrWorkerFactory::init() {
    QList<PortDescriptor*> ports;
    {
        Descriptor inDesc(BasePorts::IN_SEQ_PORT_ID(), InSilicoPcrWorker::tr("Input sequence"), InSilicoPcrWorker::tr("Input sequence."));
        Descriptor outDesc(OUT_PORT_ID, InSilicoPcrWorker::tr("PCR product"), InSilicoPcrWorker::tr("PCR product sequence."));

        QMap<Descriptor, DataTypePtr> inType;
        inType[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();

        QMap<Descriptor, DataTypePtr> outType;
        outType[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
        outType[BaseSlots::ANNOTATION_TABLE_SLOT()] = BaseTypes::ANNOTATION_TABLE_TYPE();

        ports << new PortDescriptor(inDesc, DataTypePtr(new MapDataType(ACTOR_ID + "-in", inType)), true /*input*/);
        ports << new PortDescriptor(outDesc, DataTypePtr(new MapDataType(ACTOR_ID + "-out", outType)), false /*input*/, true /*multi*/);
    }
    QList<Attribute*> attributes;
    {
        Descriptor primersDesc(PRIMERS_ATTR_ID, InSilicoPcrWorker::tr("Primers URL"), InSilicoPcrWorker::tr("A URL to the input file with primer pairs."));
        Descriptor reportDesc(REPORT_ATTR_ID, InSilicoPcrWorker::tr("Report URL"), InSilicoPcrWorker::tr("A URL to the output file with the PCR report."));
        Descriptor mismatchesDesc(MISMATCHES_ATTR_ID, InSilicoPcrWorker::tr("Mismatches"), InSilicoPcrWorker::tr("Number of allowed mismatches."));
        Descriptor perfectDesc(PERFECT_ATTR_ID, InSilicoPcrWorker::tr("Min perfect match"), InSilicoPcrWorker::tr("Number of bases that match exactly on 3' end of primers."));
        Descriptor maxProductDesc(MAX_PRODUCT_ATTR_ID, InSilicoPcrWorker::tr("Max product size"), InSilicoPcrWorker::tr("Maximum size of amplified region."));

        attributes << new Attribute(primersDesc, BaseTypes::STRING_TYPE(), true);
        attributes << new Attribute(reportDesc, BaseTypes::STRING_TYPE(), true, "report.html");
        attributes << new Attribute(mismatchesDesc, BaseTypes::NUM_TYPE(), false, 3);
        attributes << new Attribute(perfectDesc, BaseTypes::NUM_TYPE(), false, 15);
        attributes << new Attribute(maxProductDesc, BaseTypes::NUM_TYPE(), false, 5000);
    }
    QMap<QString, PropertyDelegate*> delegates;
    {
        delegates[PRIMERS_ATTR_ID] = new URLDelegate("", "", false, false, false);
        delegates[REPORT_ATTR_ID] = new URLDelegate("", "", false, false, true);
        { // mismatches
            QVariantMap props;
            props["minimum"] = 0;
            props["maximum"] = 99;
            delegates[MISMATCHES_ATTR_ID] = new SpinBoxDelegate(props);
        }
        { // perfect match
            QVariantMap props;
            props["minimum"] = 0;
            props["maximum"] = 99;
            delegates[PERFECT_ATTR_ID] = new SpinBoxDelegate(props);
        }
        { // max product
            QVariantMap props;
            props["minimum"] = 0;
            props["maximum"] = 999999;
            delegates[MAX_PRODUCT_ATTR_ID] = new SpinBoxDelegate(props);
        }
    }

    Descriptor desc(ACTOR_ID, InSilicoPcrWorker::tr("In Silico PCR"),
        InSilicoPcrWorker::tr("Simulates PCR for input sequences and primer pairs. Creates the table with the PCR statistics."));
    ActorPrototype *proto = new IntegralBusActorPrototype(desc, ports, attributes);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new InSilicoPcrPrompter(NULL));
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_ALIGNMENT(), proto);

    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new InSilicoPcrWorkerFactory());
}

/************************************************************************/
/* InSilicoPcrPrompter */
/************************************************************************/
InSilicoPcrPrompter::InSilicoPcrPrompter(Actor *a)
: PrompterBase<InSilicoPcrPrompter>(a)
{

}

QString InSilicoPcrPrompter::composeRichDoc() {
    IntegralBusPort *input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_SEQ_PORT_ID()));
    SAFE_POINT(NULL != input, "No input port", "");
    const Actor *producer = input->getProducer(BaseSlots::DNA_SEQUENCE_SLOT().getId());
    const QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    const QString producerName = (NULL != producer) ? producer->getLabel() : unsetStr;
    const QString primersLink = getHyperlink(PRIMERS_ATTR_ID, getURL(PRIMERS_ATTR_ID));
    return tr("Simulates PCR for the sequences from <u>%1</u> and primer pairs from <u>%2</u>.").arg(producerName).arg(primersLink);
}

/************************************************************************/
/* InSilicoPcrWorker */
/************************************************************************/
InSilicoPcrWorker::InSilicoPcrWorker(Actor *a)
: BaseThroughWorker(a, BasePorts::IN_SEQ_PORT_ID(), OUT_PORT_ID), reported(false)
{

}

Task * InSilicoPcrWorker::createPrepareTask(U2OpStatus &os) const {
    QString primersUrl = getValue<QString>(PRIMERS_ATTR_ID);
    QVariantMap hints;
    hints[DocumentFormat::DBI_REF_HINT] = qVariantFromValue(context->getDataStorage()->getDbiRef());
    LoadDocumentTask *task = LoadDocumentTask::getDefaultLoadDocTask(primersUrl, hints);
    if (NULL == task) {
        os.setError(tr("Can not read the primers file: ") + primersUrl);
    }
    return task;
}

void InSilicoPcrWorker::onPrepared(Task *task, U2OpStatus &os) {
    LoadDocumentTask *loadTask = dynamic_cast<LoadDocumentTask*>(task);
    CHECK_EXT(NULL != loadTask, os.setError(L10N::internalError("Unexpected prepare task")), );

    QScopedPointer<Document> doc(loadTask->takeDocument());
    CHECK_EXT(!doc.isNull(), os.setError(tr("Can't read the file: ") + loadTask->getURLString()), );

    QList<GObject*> objects = doc->findGObjectByType(GObjectTypes::SEQUENCE);
    CHECK_EXT(!objects.isEmpty(), os.setError(tr("No primer sequences in the file: ") + loadTask->getURLString()), );
    CHECK_EXT(0 == objects.size() % 2, os.setError(tr("There is the odd number of primers int the file: ") + loadTask->getURLString()), );

    fetchPrimers(objects, os);
}

void InSilicoPcrWorker::fetchPrimers(const QList<GObject*> &objects, U2OpStatus &os) {
    for (int i=0; i<objects.size()/2; i++) {
        bool skipped = false;

        Primer forward = createPrimer(objects[2*i], skipped, os);
        CHECK_OP(os, );

        Primer reverse = createPrimer(objects[2*i + 1], skipped, os);
        CHECK_OP(os, );

        if (skipped) {
            continue;
        }

        primers << QPair<Primer, Primer>(forward, reverse);
    }
}

Primer InSilicoPcrWorker::createPrimer(GObject *object, bool &skipped, U2OpStatus &os) {
    Primer result;
    U2SequenceObject *primerSeq = dynamic_cast<U2SequenceObject*>(object);
    CHECK_EXT(NULL != primerSeq, os.setError(L10N::nullPointerError("Primer sequence")), result);

    if (primerSeq->getSequenceLength() > Primer::MAX_LEN) {
        skipped = true;
        reportError(tr("Primer sequence is too long: %1. The pair is skipped").arg(primerSeq->getSequenceName()));
        return result;
    }

    result.name = primerSeq->getSequenceName();
    result.sequence = primerSeq->getWholeSequenceData();
    return result;
}

QList<Message> InSilicoPcrWorker::fetchResult(Task *task, U2OpStatus &os) {
    QList<Message> result;
    InSilicoPcrReportTask *reportTask = dynamic_cast<InSilicoPcrReportTask*>(task);
    if (NULL != reportTask) {
        monitor()->addOutputFile(getValue<QString>(REPORT_ATTR_ID), actor->getId(), true);
        return result;
    }

    MultiTask *multiTask = dynamic_cast<MultiTask*>(task);
    CHECK_EXT(NULL != multiTask, os.setError(L10N::nullPointerError("MultiTask")), result);

    InSilicoPcrReportTask::TableRow tableRow;
    foreach (Task *t, multiTask->getTasks()) {
        InSilicoPcrTask *pcrTask = dynamic_cast<InSilicoPcrTask*>(t);
        CHECK_EXT(NULL != multiTask, os.setError(L10N::nullPointerError("InSilicoPcrTask")), result);

        int pairNumber = pcrTask->property(PAIR_NUMBER_PROP_ID).toInt();
        SAFE_POINT_EXT(pairNumber >= 0 && pairNumber < primers.size(), os.setError(L10N::internalError("Out of range")), result);

        InSilicoPcrTaskSettings settings = pcrTask->getSettings();
        tableRow.sequenceName = settings.sequenceName;
        tableRow.productsNumber[pairNumber] = pcrTask->getResults().size();

        foreach (const InSilicoPcrProduct &product, pcrTask->getResults()) {
            QVariantMap data;
            data[BaseSlots::DNA_SEQUENCE_SLOT().getId()] = createProductSequence(settings.sequenceName, settings.sequence, product.region, os);
            data[BaseSlots::ANNOTATION_TABLE_SLOT().getId()] = createBindAnnotations(product);
            int metadataId = createMetadata(settings, product.region, pairNumber);
            result << Message(output->getBusType(), data, metadataId);
        }
    }
    table << tableRow;
    return result;
}

QVariant InSilicoPcrWorker::createProductSequence(const QString &sequenceName, const QByteArray &wholeSequence, const U2Region &productRegion, U2OpStatus &os) {
    const DNAAlphabet *alphabet = AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT());
    SAFE_POINT_EXT(NULL != alphabet, os.setError(L10N::nullPointerError("DNA Alphabet")), QVariant());
    
    QString name = ExtractProductTask::getProductName(sequenceName, wholeSequence.length(), productRegion);
    QByteArray sequence = wholeSequence.mid(productRegion.startPos, productRegion.length);
    DNASequence seq(name, sequence, alphabet);

    SharedDbiDataHandler seqId = context->getDataStorage()->putSequence(seq);
    return qVariantFromValue<SharedDbiDataHandler>(seqId);
}

QVariant InSilicoPcrWorker::createBindAnnotations(const InSilicoPcrProduct &product) {
    QList<SharedAnnotationData> anns;
    anns << ExtractProductTask::getPrimerAnnotation(product.forwardPrimer, product.forwardPrimerMatchLength, U2Strand::Direct, product.region.length);
    anns << ExtractProductTask::getPrimerAnnotation(product.reversePrimer, product.reversePrimerMatchLength, U2Strand::Complementary, product.region.length);
    SharedDbiDataHandler annsId = context->getDataStorage()->putAnnotationTable(anns);
    return qVariantFromValue<SharedDbiDataHandler>(annsId);
}

int InSilicoPcrWorker::createMetadata(const InSilicoPcrTaskSettings &settings, const U2Region &productRegion, int pairNumber) {
    MessageMetadata oldMetadata = context->getMetadataStorage().get(output->getContextMetadataId());
    QString primerName = primers[pairNumber].first.name;
    QString suffix = "_" + ExtractProductTask::getProductName(primerName, settings.sequence.length(), productRegion, true);
    QString newUrl = GUrlUtils::insertSuffix(oldMetadata.getFileUrl(), suffix);

    MessageMetadata metadata(newUrl, oldMetadata.getDatasetName());
    context->getMetadataStorage().put(metadata);
    return metadata.getId();
}

Task * InSilicoPcrWorker::onInputEnded() {
    CHECK(!reported, NULL);
    reported = true;
    return new InSilicoPcrReportTask(table, primers, getValue<QString>(REPORT_ATTR_ID));
}

Task * InSilicoPcrWorker::createTask(const Message &message, U2OpStatus &os) {
    QVariantMap data = message.getData().toMap();
    SharedDbiDataHandler seqId = data[BaseSlots::DNA_SEQUENCE_SLOT().getId()].value<SharedDbiDataHandler>();
    U2SequenceObject *seq = StorageUtils::getSequenceObject(context->getDataStorage(), seqId);
    SAFE_POINT(NULL != seq, L10N::nullPointerError("Sequence"), NULL);

    if (seq->getSequenceLength() > InSilicoPcrTaskSettings::MAX_SEQUENCE_LENGTH) {
        os.setError(tr("The sequence is too long: ") + seq->getSequenceName());
        return NULL;
    }

    InSilicoPcrTaskSettings settings;
    settings.sequence = seq->getWholeSequenceData();
    settings.isCircular = seq->isCircular();
    settings.forwardMismatches = getValue<int>(MISMATCHES_ATTR_ID);
    settings.reverseMismatches = settings.forwardMismatches;
    settings.maxProductSize = getValue<int>(MAX_PRODUCT_ATTR_ID);
    settings.perfectMatch = getValue<int>(PERFECT_ATTR_ID);
    settings.sequenceName = seq->getSequenceName();

    QList<Task*> tasks;
    for (int i=0; i<primers.size(); i++) {
        settings.forwardPrimer = primers[i].first.sequence.toLocal8Bit();
        settings.reversePrimer = primers[i].second.sequence.toLocal8Bit();
        Task *pcrTask = new InSilicoPcrTask(settings);
        pcrTask->setProperty(PAIR_NUMBER_PROP_ID, i);
        tasks << pcrTask;
    }
    sequences << seqId;
    return new MultiTask(tr("Multiple In Silico PCR"), tasks);
}

/************************************************************************/
/* InSilicoPcrReportTask */
/************************************************************************/
InSilicoPcrReportTask::InSilicoPcrReportTask(const QList<TableRow> &table, const QList< QPair<Primer, Primer> > &primers, const QString &reportUrl)
: Task(tr("Generate In Silico PCR report"), TaskFlag_None), table(table), primers(primers), reportUrl(reportUrl)
{

}

void InSilicoPcrReportTask::run() {
    QScopedPointer<IOAdapter> io(IOAdapterUtils::open(reportUrl, stateInfo, IOAdapterMode_Write));
    CHECK_OP(stateInfo, );

    io->writeBlock(createReport());
}

QByteArray InSilicoPcrReportTask::createReport() const {
    QByteArray report;
    report += "<!DOCTYPE html>\n";
    report += "<html>";
    report += "<body>";

    report += productsTable();
    for (int i=0; i<primers.size(); i++) {
        QPair<Primer, Primer> pair = primers[i];
        PrimersPairStatistics calc(pair.first.sequence.toLocal8Bit(), pair.second.sequence.toLocal8Bit());

        report += getChapter(pair.first.name + " / " + pair.second.name);
        report += calc.generateReport().toLocal8Bit();
        report += "<hr/>";
    }

    report += "</body>";
    report += "</html>";
    return report;
}

QByteArray InSilicoPcrReportTask::productsTable() const {
    QByteArray result;
    result += getChapter(tr("Products count table"));
    result += "<table bordercolor=\"gray\" border=\"1\">";
    result += "<tr>";
    result += PrimerGrouperTask::createColumn(tr("Sequence name"));
    for (int i=0; i<primers.size(); i++) {
        result += PrimerGrouperTask::createColumn(primers[i].first.name + "<br/>" + primers[i].second.name);
    }
    result += "</tr>";
    foreach (const TableRow &tableRow, table) {
        result += "<tr>";
        result += PrimerGrouperTask::createCell(tableRow.sequenceName);
        for (int i=0; i<primers.size(); i++) {
            result += PrimerGrouperTask::createCell(QString::number(tableRow.productsNumber[i]));
        }
        result += "</tr>";
    }
    result += "</table>";
    return result;
}

QByteArray InSilicoPcrReportTask::getChapter(const QString &name) const {
    return "<h3>" + name.toLocal8Bit() + "</h3>";
}

} // LocalWorkflow
} // U2
