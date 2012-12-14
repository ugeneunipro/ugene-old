/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include <U2Core/FailTask.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/QVariantUtils.h>
#include <U2Formats/GenbankLocationParser.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Gui/DialogUtils.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "MACSWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString MACSWorkerFactory::ACTOR_ID("macs-id");

static const QString TREATMENT_SLOT_ID("_treatment-ann");
static const QString CONTROL_SLOT_ID("control-ann");

static const QString PEAK_REGIONS_SLOT_ID("peak-regions");
static const QString PEAK_SUMMITS_SLOT_ID("peak-summits");
static const QString WIGGLE_TREAT_SLOT_ID("wiggle-treat");

static const QString IN_TYPE_ID("macs-data");
static const QString OUT_TYPE_ID("macs-data-out");

static const QString IN_PORT_DESCR("in-data");
static const QString OUT_PORT_DESCR("out-data");

static const QString OUTPUT_DIR("output-dir");
static const QString FILE_NAMES("file-names");
static const QString WIGGLE_OUTPUT("wiggle-output");
static const QString WIGGLE_SPACE("wiggle-space");

static const QString GENOME_SIZE_ATTR_ID("genome-size");
static const QString P_VALUE_ATTR_ID("p-value");
static const QString Q_VALUE_ATTR_ID("q-value");    //(MACS 2)
static const QString USE_MODEL_ATTR_ID("use-model");
static const QString MODEL_FOLD_ATTR_ID("model-fold");
static const QString SHIFT_SIZE_ATTR_ID("shift-size");
static const QString KEEP_DUBLICATES_ATTR_ID("keep-duplicates");
static const QString BAND_WIDTH_ATTR_ID("band-width");
static const QString EXT_FR_ATTR_ID("ext-fr");    //(???)
//optional
static const QString TAG_SIZE_ATTR_ID("tag-size");
//advanced
static const QString USE_LAMBDA_ATTR_ID("use-lambda");
static const QString SMALL_NEARBY_ATTR_ID("small-nearby");
static const QString LARGE_NEARBY_ATTR_ID("large-nearby");
static const QString AUTO_BIMODAL_ATTR_ID("auto_bimodal");
static const QString SCALE_LARGE_ATTR_ID("scale_large");
static const QString SHIFT_CONTROL_ATTR_ID("shift_control");    //(MACS 2)
static const QString HALF_EXTEND_ATTR_ID("half_extend");    //(MACS 2)
static const QString BROAD_ATTR_ID("broad");    //(MACS 2)
static const QString BROAD_CUTOFF_ATTR_ID("broad_cutoff");    //(MACS 2)


/************************************************************************/
/* Worker */
/************************************************************************/
MACSWorker::MACSWorker(Actor *p)
: BaseWorker(p)
, inChannel(NULL)
, output(NULL)
{

}

void MACSWorker::init() {
    inChannel = ports.value(IN_PORT_DESCR);
    output = ports.value(OUT_PORT_DESCR);
}

Task *MACSWorker::tick() {
    if (inChannel->hasMessage()) {
        U2OpStatus2Log os;

        Message m = getMessageAndSetupScriptValues(inChannel);
        QVariantMap data = m.getData().toMap();

        QVariant treatVar;
        QVariant conVar;
        if (!data.contains(TREATMENT_SLOT_ID)) {
            os.setError("Treatment slot is empty");
            return new FailTask(os.getError());
        }

        treatVar = data[TREATMENT_SLOT_ID];

        if (data.contains(CONTROL_SLOT_ID)) {
            conVar = data[CONTROL_SLOT_ID];
        }
        const QList<SharedAnnotationData>& treatData = QVariantUtils::var2ftl(treatVar.toList());
        const QList<SharedAnnotationData>& conData = QVariantUtils::var2ftl(conVar.toList());

        MACSSettings settings = createMACSSettings(os);
        if (os.hasError()) {
            return new FailTask(os.getError());
        }

        Task* t = new MACSTask(settings, treatData, conData);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    }else if (inChannel->isEnded()) {
        setDone();
    }
    return NULL;
}

void MACSWorker::cleanup() {

}

void MACSWorker::sl_taskFinished() {
    MACSTask *t = dynamic_cast<MACSTask*>(sender());
    if (!t->isFinished() || t->hasError()) {
        return;
    }

    QVariantMap data;
    data[PEAK_REGIONS_SLOT_ID] = qVariantFromValue<QList<SharedAnnotationData> >(t->getPeaks());
    data[PEAK_SUMMITS_SLOT_ID] = qVariantFromValue<QList<SharedAnnotationData> >(t->getPeakSummits());
    if (t->getSettings().wiggleOut){
        data[WIGGLE_TREAT_SLOT_ID] = qVariantFromValue<QString>(t->getWiggleUrl());
    }else{
        data[WIGGLE_TREAT_SLOT_ID] = qVariantFromValue<QString>(QString(""));
    }
    
    output->put(Message(output->getBusType(), data));

    if (inChannel->isEnded() && !inChannel->hasMessage()) {
        setDone();
        output->setEnded();
    }
}

U2::MACSSettings MACSWorker::createMACSSettings( U2OpStatus &os ){
    MACSSettings settings;

    settings.outDir = actor->getParameter(OUTPUT_DIR)->getAttributeValue<QString>(context);
    settings.fileNames = actor->getParameter(FILE_NAMES)->getAttributeValue<QString>(context);
    settings.wiggleOut = actor->getParameter(WIGGLE_OUTPUT)->getAttributeValue<bool>(context);
    settings.wiggleSpace = actor->getParameter(WIGGLE_SPACE)->getAttributeValue<int>(context);

    settings.genomeSizeInMbp = actor->getParameter(GENOME_SIZE_ATTR_ID)->getAttributeValue<int>(context);
    settings.pValue = actor->getParameter(P_VALUE_ATTR_ID)->getAttributeValue<double>(context);
    //settings.qValue = actor->getParameter(Q_VALUE_ATTR_ID)->getAttributeValue<double>(context);    //(MACS 2)
    settings.useModel = actor->getParameter(USE_MODEL_ATTR_ID)->getAttributeValue<bool>(context);

    QString locStr = actor->getParameter(MODEL_FOLD_ATTR_ID)->getAttributeValue<QString>(context);
    U2Location l;
    QString err = Genbank::LocationParser::parseLocation(qPrintable(locStr), locStr.size(), l);
    if (!err.isEmpty()){
        algoLog.error(tr("Bad model fold region: %1. Default region is used").arg(err));
    }else{
        if(!l->regions.isEmpty()){
            settings.modelFold = l->regions.first();
        }
    }
    
    settings.shiftSize = actor->getParameter(SHIFT_SIZE_ATTR_ID)->getAttributeValue<int>(context);

    settings.keepDublicates = actor->getParameter(KEEP_DUBLICATES_ATTR_ID)->getAttributeValue<QString>(context);

    settings.bandWidth = actor->getParameter(BAND_WIDTH_ATTR_ID)->getAttributeValue<int>(context);
    //settings.extFr = actor->getParameter(EXT_FR_ATTR_ID)->getAttributeValue<bool>(context);    //(???)
    //optional
    settings.tagSize = actor->getParameter(TAG_SIZE_ATTR_ID)->getAttributeValue<int>(context); //0 for default
    //advanced
    settings.useLambda = actor->getParameter(USE_LAMBDA_ATTR_ID)->getAttributeValue<bool>(context);
    settings.smallNearby = actor->getParameter(SMALL_NEARBY_ATTR_ID)->getAttributeValue<int>(context);
    settings.largeNearby = actor->getParameter(LARGE_NEARBY_ATTR_ID)->getAttributeValue<int>(context);
    settings.autoBimodal = actor->getParameter(AUTO_BIMODAL_ATTR_ID)->getAttributeValue<bool>(context);
    settings.scaleLarge = actor->getParameter(SCALE_LARGE_ATTR_ID)->getAttributeValue<bool>(context);
    //settings.shiftControl = actor->getParameter(SHIFT_CONTROL_ATTR_ID)->getAttributeValue<bool>(context);    //(MACS 2)
    //settings.halfExtend = actor->getParameter(HALF_EXTEND_ATTR_ID)->getAttributeValue<bool>(context);    //(MACS 2)
    //settings.broad = actor->getParameter(BROAD_ATTR_ID)->getAttributeValue<bool>(context);    //(MACS 2)
    //settings.broadCutoff = actor->getParameter(BROAD_CUTOFF_ATTR_ID)->getAttributeValue<double>(context);    //(MACS 2)

    return settings;
}


QStringList MACSWorker::getOutputFiles() {
    return QStringList();
}

/************************************************************************/
/* Factory */
/************************************************************************/

class MACSAnnotationsValidator : public ConfigurationValidator {
public:
    MACSAnnotationsValidator(const QString& _sl1, const QString& _port, const QString& _sl2):sl1(_sl1),sl2(_sl2),port(_port){
        
    }
    virtual bool validate(const Configuration* cfg, QStringList& output) const {
        const Workflow::Actor* a = dynamic_cast<const Workflow::Actor*>(cfg);
        assert(a);

        Workflow::Port* p = a->getPort(port);
        assert(p->isInput());

        QVariant busMap = p->getParameter(Workflow::IntegralBusPort::BUS_MAP_ATTR_ID)->getAttributePureValue();
        QString slot1Val = busMap.value<QStrStrMap>().value(sl1);
        QString slot2Val = busMap.value<QStrStrMap>().value(sl2);
        U2OpStatusImpl os;
        const QList<IntegralBusSlot>& slots1 = IntegralBusSlot::listFromString(slot1Val, os);
        const QList<IntegralBusSlot>& slots2 = IntegralBusSlot::listFromString(slot2Val, os);

        bool hasCommonElements = false;

        foreach(const IntegralBusSlot& ibsl1, slots1){
            if (hasCommonElements){
                break;
            }
            foreach(const IntegralBusSlot& ibsl2, slots2){
                if (ibsl1.toString() == ibsl2.toString()){//TODO: need check by vaskin
                    hasCommonElements = true;
                    break;
                }
            }
        }

        if (hasCommonElements){
            output.append(MACSWorker::tr("Warning: Input control and treatment annotations are the same"));
        }

        return true;
    }
protected:
    QString sl1;
    QString port;
    QString sl2;
};


void MACSWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    
    //in port
    QMap<Descriptor, DataTypePtr> inTypeMap;
    Descriptor treatDesc(TREATMENT_SLOT_ID,
        MACSWorker::tr("Treatment features"),
        MACSWorker::tr("ChIP-seq treatment features."));
    Descriptor conDesc(CONTROL_SLOT_ID,
        MACSWorker::tr("Control features"),
        MACSWorker::tr("Control features (Optional)."));
    inTypeMap[treatDesc] = BaseTypes::ANNOTATION_TABLE_LIST_TYPE();
    inTypeMap[conDesc] = BaseTypes::ANNOTATION_TABLE_TYPE();

    Descriptor inPortDesc(IN_PORT_DESCR,
        MACSWorker::tr("MACS data"),
        MACSWorker::tr("ChIP-seq treatment features and control features (optional) to call peaks with MACS."));

    DataTypePtr inTypeSet(new MapDataType(IN_TYPE_ID, inTypeMap));
    portDescs << new PortDescriptor(inPortDesc, inTypeSet, true);

    //out port
    QMap<Descriptor, DataTypePtr> outTypeMap;
    Descriptor peakRegDesc(PEAK_REGIONS_SLOT_ID,
        MACSWorker::tr("Peak regions"),
        MACSWorker::tr("Peak locations."));
    Descriptor peakSummitsDescr(PEAK_SUMMITS_SLOT_ID,
        MACSWorker::tr("Peak summits"),
        MACSWorker::tr("Peak summits locations for every peaks."));
    Descriptor wiggleTreatDescr(WIGGLE_TREAT_SLOT_ID,
        MACSWorker::tr("Treatment fragments pileup (wiggle)"),
        MACSWorker::tr("Wiggle format files which can be imported to UCSC genome browser/GMOD/Affy IGB."));

    Descriptor outPortDesc(OUT_PORT_DESCR,
        MACSWorker::tr("MACS output data"),
        MACSWorker::tr("ChIP-seq peaks and summits. Pileup data (optional)"));

    outTypeMap[peakRegDesc] = BaseTypes::ANNOTATION_TABLE_TYPE();
    outTypeMap[peakSummitsDescr] = BaseTypes::ANNOTATION_TABLE_TYPE();
    outTypeMap[wiggleTreatDescr] = BaseTypes::STRING_TYPE();

    DataTypePtr outTypeSet(new MapDataType(OUT_TYPE_ID, outTypeMap));
    portDescs << new PortDescriptor(outPortDesc, outTypeSet, false, true);
    
 
     QList<Attribute*> attrs;
     {
         static const QString WIGGLE_SPACE("wiggle-space");

         Descriptor outDir(OUTPUT_DIR,
             MACSWorker::tr("Output directory"),
             MACSWorker::tr("Directory to save MACS output files"));
         Descriptor fileNames(FILE_NAMES,
             MACSWorker::tr("Name"),
             MACSWorker::tr("The name string of the experiment. MACS will use this string NAME"
             "to create output files like 'NAME_peaks.xls', 'NAME_negative_peaks.xls', 'NAME_peaks.bed' ,"
             "'NAME_summits.bed', 'NAME_model.r' and so on. So please avoid any confliction between these filenames and your existing files. (--name)"));
         Descriptor wiggleOut(WIGGLE_OUTPUT,
             MACSWorker::tr("Wiggle output"),
             MACSWorker::tr("If this flag is on, MACS will store the fragment pileup in wiggle format for the whole genome instead of for every chromosomes. (--wig) (--single-profile)"));
         Descriptor wiggleSpace(WIGGLE_SPACE,
             MACSWorker::tr("Wiggle space"),
             MACSWorker::tr("By default, the resoluation for saving wiggle files is 10 bps,i.e.,"
             "MACS will save the raw tag count every 10 bps. You can change it along with '--wig' option. (--space)"));
        Descriptor gsizeDesc(GENOME_SIZE_ATTR_ID,
            MACSWorker::tr("Genome size (Mbp)"),
            MACSWorker::tr("Homo sapience - 2700 Mbp<br>"
            "Mus musculus - 1870 Mbp<br>"
            "Caenorhabditis elegans - 90 Mbp<br>"
            "Drosophila melanogaster  - 120 Mbp<br>"
            "It's the mappable genome size or effective genome size"
            "which is defined as the genome size which can be sequenced."
            "Because of the repetitive features on the chromosomes, the actual mappable"
            "genome size will be smaller than the original size, about 90% or 70% of the genome size. (--gsize)"));
        Descriptor pvalueDesc(P_VALUE_ATTR_ID,
            MACSWorker::tr("P-value"),
            MACSWorker::tr("P-value cutoff. (--pvalue)"));
        Descriptor qvalueDesc(Q_VALUE_ATTR_ID,
            MACSWorker::tr("Q-value"),
            MACSWorker::tr("Minimum FDR (q-value) cutoff for peak detection."));
        Descriptor useModelDesc(USE_MODEL_ATTR_ID,
            MACSWorker::tr("Use Model"),
            MACSWorker::tr("Whether or not to use Macs paired peaks model. (--nomodel)"));
        Descriptor modelFoldDesc(MODEL_FOLD_ATTR_ID,
            MACSWorker::tr("Model fold"),
            MACSWorker::tr("Select the regions within MFOLD range of high-confidence enrichment ratio against. (--mfold)"));
        Descriptor shiftSizeDesc(SHIFT_SIZE_ATTR_ID,
            MACSWorker::tr("Shift size"),
            MACSWorker::tr("An arbitrary shift value used as a half of the fragment size when model is not built. (--shiftsize)"));
        Descriptor keepDupDesc(KEEP_DUBLICATES_ATTR_ID,
            MACSWorker::tr("Keep duplicates"),
            MACSWorker::tr("It controls the MACS behavior towards duplicate tags at the exact same location -- the same coordination and the same strand."
            "The default <b>auto</b> option makes MACS calculate the maximum tags at the exact same location based on binomal distribution using 1e-5 as "
            "pvalue cutoff; and the <b>all</b> option keeps every tags. If an <b>integer</b> is given, at most this number of tags will be kept at the same location. (--keep-dup)"));
        Descriptor bandWDesc(BAND_WIDTH_ATTR_ID,
            MACSWorker::tr("Band width"),
            MACSWorker::tr("The band width which is used to scan the genome for model building."
            "You can set this parameter as the sonication fragment size expected from wet experiment."
            "Used only while building the shifting model. (--bw)"));
        Descriptor extFrDesc(EXT_FR_ATTR_ID,
            MACSWorker::tr("Extended fragment pileup"),
            MACSWorker::tr("Whether or not to generate extended fragment pileup, local lambda and score tracks at every bp."));

        //optional
        Descriptor tagSizeDesc(TAG_SIZE_ATTR_ID,
            MACSWorker::tr("Tag size (optional)"),
            MACSWorker::tr("Length of reads. Determined from first 10 reads if not specified (input <b>0</b>). (--tsize)"));

        //advanced
        Descriptor useLambdaDesc(USE_LAMBDA_ATTR_ID,
            MACSWorker::tr("Use lambda"),
            MACSWorker::tr("whether to use local lambda model which can use the local bias at peak regions to throw out false positives. (--nolambda)"));
        Descriptor smallNearbyDesc(SMALL_NEARBY_ATTR_ID,
            MACSWorker::tr("Small nearby region"),
            MACSWorker::tr("The small nearby region in basepairs to calculate dynamic lambda."
            "This is used to capture the bias near the peak summit region. Invalid if there is no control data. (--slocal)"));
        Descriptor LargeNearbyDesc(LARGE_NEARBY_ATTR_ID,
            MACSWorker::tr("Large nearby region"),
            MACSWorker::tr("The large nearby region in basepairs to calculate dynamic lambda. "
            "This is used to capture the surround bias. (--llocal)"));
        Descriptor autoBimodalDesc(AUTO_BIMODAL_ATTR_ID,
            MACSWorker::tr("Auto bimodal"),
            MACSWorker::tr("Whether turn on the auto pair model process."
            "If set, when MACS failed to build paired model, it will use the nomodel"
            "settings, the “Shift size” parameter to shift and extend each tags. (--on-auto)"));
        Descriptor scaleLargeDesc(SCALE_LARGE_ATTR_ID,
            MACSWorker::tr("Scale to large"),
            MACSWorker::tr(" When set, scale the small sample up to the bigger sample."
            "By default, the bigger dataset will be scaled down towards the smaller dataset,"
            "which will lead to smaller p/qvalues and more specific results."
            "Keep in mind that scaling down will bring down background noise more. (--to-large)"));
        Descriptor shiftControlDesc(SHIFT_CONTROL_ATTR_ID,
            MACSWorker::tr("Shift control"),
            MACSWorker::tr("When set, control tags will be shifted just as ChIP tags according to their strand "
            "before the extension of d, slocal and llocal. By default, control tags are extended centered"
            "at their current positions regardless of strand. You may consider to turn this option on while"
            "comparing two ChIP datasets of different condition but the same factor."));
        Descriptor halfExtendDesc(HALF_EXTEND_ATTR_ID,
            MACSWorker::tr("Half-extend"),
            MACSWorker::tr("When set, MACS extends 1/2 d size for each fragment centered at its middle point."));
        Descriptor broadDesc(BROAD_ATTR_ID,
            MACSWorker::tr("Broad"),
            MACSWorker::tr("If set, MACS will try to call broad peaks by linking nearby highly enriched regions."
            "The linking region is controlled by another cutoff through “Broad cutoff”."
            "The maximum linking region length is 4 times of d from MACS."));
        Descriptor broadCutoffDesc(BROAD_CUTOFF_ATTR_ID,
            MACSWorker::tr("Broad cutoff"),
            MACSWorker::tr("Cutoff for broad region. This option is not available unless “Broad” is set."
            "If “P-value” is set, this is a pvalue cutoff, otherwise, it's a qvalue cutoff."));


        attrs << new Attribute(outDir, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(fileNames, BaseTypes::STRING_TYPE(), true, QVariant("Default"));
        attrs << new Attribute(wiggleOut, BaseTypes::BOOL_TYPE(), true, QVariant(true));

        Attribute* wspaceAttr = new Attribute(wiggleSpace, BaseTypes::NUM_TYPE(), true, QVariant(10));
        wspaceAttr ->addRelation(new VisibilityRelation(WIGGLE_OUTPUT, "True"));
        attrs << wspaceAttr;

        attrs << new Attribute(gsizeDesc, BaseTypes::NUM_TYPE(), false, QVariant(2700));
        attrs << new Attribute(pvalueDesc, BaseTypes::NUM_TYPE(), false, QVariant(0.00001));
        //attrs << new Attribute(qvalueDesc, BaseTypes::NUM_TYPE(), false, QVariant(0.1));    //(MACS 2)
        attrs << new Attribute(useModelDesc, BaseTypes::BOOL_TYPE(), false, QVariant(true));
        
        Attribute* foldAttr = new Attribute(modelFoldDesc, BaseTypes::STRING_TYPE(), false, QVariant(Genbank::LocationParser::buildLocationString(QVector<U2Region>()<<U2Region(9, 21))));
        foldAttr ->addRelation(new VisibilityRelation(USE_MODEL_ATTR_ID, "True"));
        attrs << foldAttr;

        Attribute* shiftAttr = new Attribute(shiftSizeDesc, BaseTypes::NUM_TYPE(), false, QVariant(100));
        foldAttr ->addRelation(new VisibilityRelation(USE_MODEL_ATTR_ID, "False"));
        //TODO:Available if: “Use Model” is False OR building model by MACSis failed
        attrs << shiftAttr;

        attrs << new Attribute(keepDupDesc, BaseTypes::STRING_TYPE(), false, QVariant("1"));
        attrs << new Attribute(bandWDesc, BaseTypes::NUM_TYPE(), false, QVariant(300));
        //attrs << new Attribute(extFrDesc, BaseTypes::BOOL_TYPE(), false, QVariant(false));    //(???)
        //optional
        attrs << new Attribute(tagSizeDesc, BaseTypes::NUM_TYPE(), false, QVariant(0));
        //advanced
        attrs << new Attribute(useLambdaDesc, BaseTypes::BOOL_TYPE(), false, QVariant(true));
        attrs << new Attribute(smallNearbyDesc, BaseTypes::NUM_TYPE(), false, QVariant(1000));
        attrs << new Attribute(LargeNearbyDesc, BaseTypes::NUM_TYPE(), false, QVariant(10000));
        attrs << new Attribute(autoBimodalDesc, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attrs << new Attribute(scaleLargeDesc, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        //attrs << new Attribute(shiftControlDesc, BaseTypes::BOOL_TYPE(), false, QVariant(false));    //(MACS 2)
        //attrs << new Attribute(halfExtendDesc, BaseTypes::BOOL_TYPE(), false, QVariant(false));    //(MACS 2)
        //attrs << new Attribute(broadDesc, BaseTypes::BOOL_TYPE(), false, QVariant(false));    //(MACS 2)
        //attrs << new Attribute(broadCutoffDesc, BaseTypes::NUM_TYPE(), false, QVariant(0.1));    //(MACS 2)
     }

     QMap<QString, PropertyDelegate*> delegates;
     {
         delegates[OUTPUT_DIR] = new URLDelegate("", "", false, true);
         {
            QVariantMap vm;
            vm["minimum"] = QVariant(1);
            vm["maximum"] = INT_MAX;
            vm["singleStep"] = QVariant(10);

            delegates[WIGGLE_SPACE] = new SpinBoxDelegate(vm);
         }
         {
            QVariantMap vm;
            vm["minimum"] = QVariant(1);
            vm["maximum"] = QVariant(10000);
            vm["singleStep"] = QVariant(1);
            vm["suffix"] = QVariant("Mbp");
            delegates[GENOME_SIZE_ATTR_ID] = new SpinBoxDelegate(vm);
         }
         {
             QVariantMap vm;
             vm["minimum"] = 0;
             vm["maximum"] = 1;
             vm["singleStep"] = 0.00001;
             vm["decimals"] = 6;
             delegates[P_VALUE_ATTR_ID] = new DoubleSpinBoxDelegate(vm);
         }
         {
             QVariantMap vm;
             vm["minimum"] = 0;
             vm["maximum"] = 1;
             vm["singleStep"] = 0.1;
             delegates[Q_VALUE_ATTR_ID] = new DoubleSpinBoxDelegate(vm);
         }
         {
             QVariantMap vm;
             vm["minimum"] = QVariant(1);
             vm["maximum"] = INT_MAX;
             vm["singleStep"] = QVariant(1);
             delegates[SHIFT_SIZE_ATTR_ID] = new SpinBoxDelegate(vm);
         }
         {
             QVariantMap vm;
             vm["minimum"] = 0;
             vm["maximum"] = INT_MAX;
             vm["singleStep"] = 1;
             delegates[BAND_WIDTH_ATTR_ID] = new SpinBoxDelegate(vm);
         }
         {
             QVariantMap vm;
             vm["minimum"] = 0;
             vm["maximum"] = INT_MAX;
             vm["singleStep"] = 1;
             delegates[TAG_SIZE_ATTR_ID] = new SpinBoxDelegate(vm);
         }
         {
             QVariantMap vm;
             vm["minimum"] = 0;
             vm["maximum"] = INT_MAX;
             vm["singleStep"] = 100;
             delegates[SMALL_NEARBY_ATTR_ID] = new SpinBoxDelegate(vm);
         }
         {
             QVariantMap vm;
             vm["minimum"] = 0;
             vm["maximum"] = INT_MAX;
             vm["singleStep"] = 1000;
             delegates[LARGE_NEARBY_ATTR_ID] = new SpinBoxDelegate(vm);
         }
         {
             QVariantMap vm;
             vm["minimum"] = 0;
             vm["maximum"] = 1;
             vm["singleStep"] = 0.1;
             delegates[BROAD_CUTOFF_ATTR_ID] = new DoubleSpinBoxDelegate(vm);
         }

     }

    Descriptor protoDesc(MACSWorkerFactory::ACTOR_ID,
        MACSWorker::tr("Find Peaks with MACS"),
        MACSWorker::tr("Performs peak calling for ChIP-Seq data."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);
    proto->setPrompter(new MACSPrompter());
    proto->setEditor(new DelegateEditor(delegates));
    proto->setValidator(new MACSAnnotationsValidator(treatDesc.getId(), portDescs.first()->getId(), conDesc.getId()));
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_CHIP_SEQ(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new MACSWorkerFactory());
}

Worker *MACSWorkerFactory::createWorker(Actor *a) {
    return new MACSWorker(a);
}

QString MACSPrompter::composeRichDoc() {
    QString res = ""; 

    Actor* treatProducer = qobject_cast<IntegralBusPort*>(target->getPort(IN_PORT_DESCR))->getProducer(TREATMENT_SLOT_ID);
    Actor* controlProducer = qobject_cast<IntegralBusPort*>(target->getPort(IN_PORT_DESCR))->getProducer(CONTROL_SLOT_ID);

    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString treatUrl = treatProducer ? treatProducer->getLabel() : unsetStr;
    QString conUrl = controlProducer ? controlProducer->getLabel() : unsetStr;
    int wiggleSpan = getParameter(WIGGLE_SPACE).toInt();
    
    QString dir = getHyperlink(OUTPUT_DIR, getURL(OUTPUT_DIR));

    res.append(tr("Uses annotations from <u>%1</u> as treatment").arg(treatUrl));
    if (controlProducer){
        res.append(tr(" and annotations from <u>%1</u> as control").arg(conUrl));
    }
 
    res.append(tr(" to call peaks."));
 
    res.append(tr(" Outputs all files in <u>%1</u> directory").arg(dir));
    if (getParameter(WIGGLE_OUTPUT).toBool()){
        res.append(tr(" and pileup with <u>%1</u> span").arg(wiggleSpan));
    }
    res.append(".");
    

    return res;
}

} // LocalWorkflow
} // U2
