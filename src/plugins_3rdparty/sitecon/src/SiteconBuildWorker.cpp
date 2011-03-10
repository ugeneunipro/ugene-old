#include "SiteconWorkers.h"
#include "SiteconIOWorkers.h"
#include "SiteconPlugin.h"
#include "SiteconBuildDialogController.h"

#include <U2Lang/Datatype.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Core/FailTask.h>

#include <U2Core/Log.h>
#include <U2Core/MAlignment.h>

/* TRANSLATOR U2::SiteconIO */
/* TRANSLATOR U2::LocalWorkflow::SiteconBuildWorker */

namespace U2 {
namespace LocalWorkflow {

static const QString WINDOW_ATTR("window-size");
static const QString LEN_ATTR("calibrate-length");
static const QString SEED_ATTR("seed");
static const QString ALG_ATTR("weight-algorithm");

static const QString OUT_SITECON_PORT_ID("out-sitecon");

const QString SiteconBuildWorker::ACTOR_ID("sitecon-build");

void SiteconBuildWorker::registerProto() {
    QList<PortDescriptor*> p; QList<Attribute*> a;
    QMap<Descriptor, DataTypePtr> m;
    Descriptor id(BasePorts::IN_MSA_PORT_ID(), SiteconBuildWorker::tr("Input alignment"), SiteconBuildWorker::tr("Input multiple sequence alignment for building statistical model."));
    Descriptor ud(BaseSlots::URL_SLOT().getId(), SiteconBuildWorker::tr("Origin"), SiteconBuildWorker::tr("Location of input alignment, used as optional hint for model description."));
    m[ud] = BaseTypes::STRING_TYPE();
    m[BaseSlots::MULTIPLE_ALIGNMENT_SLOT()] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
    DataTypePtr t(new MapDataType(Descriptor("build.sitecon.content"), m));

    Descriptor od(OUT_SITECON_PORT_ID, SiteconBuildWorker::tr("Sitecon model"), SiteconBuildWorker::tr("Produced statistical model of specified TFBS data."));
    p << new PortDescriptor(id, t, true /*input*/);
    QMap<Descriptor, DataTypePtr> outM;
    outM[SiteconWorkerFactory::SITECON_SLOT] = SiteconWorkerFactory::SITECON_MODEL_TYPE();
    p << new PortDescriptor(od, DataTypePtr(new MapDataType("sitecon.build.out", outM)), false /*input*/, true /*multi*/);
    
    {
        Descriptor wd(WINDOW_ATTR, SiteconBuildWorker::tr("Window size, bp"), QApplication::translate("SiteconBuildDialog", "win_tip", 0, QApplication::UnicodeUTF8));
        Descriptor ld(LEN_ATTR, SiteconBuildWorker::tr("Calibration length"), QApplication::translate("SiteconBuildDialog", "seq_len_tip", 0, QApplication::UnicodeUTF8));
        Descriptor sd(SEED_ATTR, SiteconBuildWorker::tr("Random seed"), QApplication::translate("SiteconBuildDialog", "seed_tip", 0, QApplication::UnicodeUTF8));
        Descriptor ad(ALG_ATTR, SiteconBuildWorker::tr("Weight algorithm"), QApplication::translate("SiteconBuildDialog", "algo_tip", 0, QApplication::UnicodeUTF8));

        a << new Attribute(wd, BaseTypes::NUM_TYPE(), false, 40);
        a << new Attribute(ld, BaseTypes::NUM_TYPE(), false, 1000*1000);
        a << new Attribute(sd, BaseTypes::NUM_TYPE(), false, 0);
        a << new Attribute(ad, BaseTypes::BOOL_TYPE(), false, int(SiteconWeightAlg_None));
    }

    Descriptor desc(ACTOR_ID, tr("Build SITECON model"),
        tr("Builds statistical profile for SITECON. The SITECON is a program for probabilistic recognition of transcription factor binding sites."));
    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);
    QMap<QString, PropertyDelegate*> delegates;    
    
    {
        QVariantMap m2; m2["minimum"] = 0; m2["maximum"] = INT_MAX;
        delegates[SEED_ATTR] = new SpinBoxDelegate(m2);
    }
    {
        QVariantMap m2; m2["minimum"] = 1; m2["maximum"] = 1000;
        delegates[WINDOW_ATTR] = new SpinBoxDelegate(m2);
    }
    {
        QVariantMap modeMap; 
        modeMap["100K"] = 100*1000;
        modeMap["500K"] = 500*1000;
        modeMap["1M"] = 1000*1000;
        modeMap["5M"] = 5*1000*1000;
        delegates[LEN_ATTR] = new ComboBoxDelegate(modeMap);
    }
    {
        QVariantMap modeMap; 
        modeMap[tr("None")] = QVariant(SiteconWeightAlg_None);
        modeMap[tr("Algorithm2")] = QVariant(SiteconWeightAlg_Alg2);
        delegates[ALG_ATTR] = new ComboBoxDelegate(modeMap);
    }

    proto->setPrompter(new SiteconBuildPrompter());
    proto->setEditor(new DelegateEditor(delegates));
    proto->setIconPath(":sitecon/images/sitecon.png");
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_TRANSCRIPTION(), proto);
}

QString SiteconBuildPrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_MSA_PORT_ID()));
    Actor* msaProducer = input->getProducer(BasePorts::IN_MSA_PORT_ID());

    QString msaName = msaProducer ? tr("For each MSA from <u>%1</u>,").arg(msaProducer->getLabel()) : "";
    QString doc = tr("%1 build SITECON model.")
        .arg(msaName);

    return doc;
}

void SiteconBuildWorker::init() {
    input = ports.value(BasePorts::IN_MSA_PORT_ID());
    output = ports.value(OUT_SITECON_PORT_ID);
}

bool SiteconBuildWorker::isReady() {
    return (input && input->hasMessage());
}

Task* SiteconBuildWorker::tick() {
    Message inputMessage = getMessageAndSetupScriptValues(input);
    cfg.props = SiteconPlugin::getDinucleotiteProperties();
    cfg.randomSeed = actor->getParameter(SEED_ATTR)->getAttributeValue<int>();
    if(cfg.randomSeed<0){
        return new FailTask(tr("Random seed can not be less zero"));
    }
    cfg.secondTypeErrorCalibrationLen = actor->getParameter(LEN_ATTR)->getAttributeValue<int>();
    if(cfg.secondTypeErrorCalibrationLen<0){
        return new FailTask(tr("Calibration length can not be less zero"));
    }
    cfg.weightAlg = SiteconWeightAlg(actor->getParameter(ALG_ATTR)->getAttributeValue<int>());
    cfg.windowSize = actor->getParameter(WINDOW_ATTR)->getAttributeValue<int>();
    if(cfg.windowSize<0){
        return new FailTask(tr("Window size can not be less zero"));
    }
    mtype = SiteconWorkerFactory::SITECON_MODEL_TYPE();
    QVariantMap data = inputMessage.getData().toMap();
    SiteconModel model = data.value(SiteconWorkerFactory::SITECON_MODEL_TYPE_ID).value<SiteconModel>();
    QString url = data.value(BaseSlots::URL_SLOT().getId()).toString();
    
    const MAlignment& ma = data.value(BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId()).value<MAlignment>();
    Task* t = new SiteconBuildTask(cfg, ma, url);
    connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
    return t;
}

void SiteconBuildWorker::sl_taskFinished() {
    SiteconBuildTask* t = qobject_cast<SiteconBuildTask*>(sender());
    if (t->getState() != Task::State_Finished) return;
    SiteconModel model = t->getResult();
    QVariant v = qVariantFromValue<SiteconModel>(model);
    output->put(Message(mtype, v));
    if (input->isEnded()) {
        output->setEnded();
    }
    algoLog.info(tr("Built SITECON model from: %1").arg(model.aliURL));
}

bool SiteconBuildWorker::isDone() {
    return !input || input->isEnded();
}

} //namespace LocalWorkflow
} //namespace U2
