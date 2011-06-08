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

#include "CAP3Worker.h"
#include "TaskLocalStorage.h"
#include "CAP3Support.h"

#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/Log.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/FailTask.h>

namespace U2 {
namespace LocalWorkflow {


/*
/****************************
 * ClustalWWorkerFactory
 ****************************/
//const QString ClustalWWorkerFactory::ACTOR_ID("clustalw");
//const QString GAP_OPEN_PENALTY("gap-open-penalty");
//const QString GAP_EXT_PENALTY("gap-ext-penalty");
//const QString GAP_DIST("gap-distance");
//const QString END_GAPS("close-gap-penalty");
//const QString NO_PGAPS("no-residue-specific-gaps");
//const QString NO_HGAPS("no-hydrophilic-gaps");
//const QString ITERATION("iteration-type");
//const QString NUM_ITERATIONS("iterations-max-num");
//const QString MATRIX("matrix");
//const QString EXT_TOOL_PATH("path");
//const QString TMP_DIR_PATH("temp-dir");
//
//void ClustalWWorkerFactory::init() {
//    QList<PortDescriptor*> p; QList<Attribute*> a;
//    Descriptor ind(BasePorts::IN_MSA_PORT_ID(), ClustalWWorker::tr("Input MSA"), ClustalWWorker::tr("Input MSA to process."));
//    Descriptor oud(BasePorts::OUT_MSA_PORT_ID(), ClustalWWorker::tr("ClustalW result MSA"), 
//        ClustalWWorker::tr("The result of the ClustalW alignment."));
//    
//    QMap<Descriptor, DataTypePtr> inM;
//    inM[BaseSlots::MULTIPLE_ALIGNMENT_SLOT()] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
//    p << new PortDescriptor(ind, DataTypePtr(new MapDataType("clustal.in.msa", inM)), true /*input*/);
//    QMap<Descriptor, DataTypePtr> outM;
//    outM[BaseSlots::MULTIPLE_ALIGNMENT_SLOT()] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
//    p << new PortDescriptor(oud, DataTypePtr(new MapDataType("clustal.out.msa", outM)), false /*input*/, true /*multi*/);
//    
//    Descriptor gop(GAP_OPEN_PENALTY, ClustalWWorker::tr("Gap open penalty"),
//                    ClustalWWorker::tr("The penalty for opening a gap."));
//    Descriptor gep(GAP_EXT_PENALTY, ClustalWWorker::tr("Gap extension penalty"),
//                    ClustalWWorker::tr("The penalty for extending a gap."));
//    Descriptor gd(GAP_DIST, ClustalWWorker::tr("Gap distance"),
//                    ClustalWWorker::tr("The gap separation penalty. Tries to decrease the chances of gaps being too close to each other."));
//    Descriptor eg(END_GAPS, ClustalWWorker::tr("End gaps"),
//                    ClustalWWorker::tr("The penalty for closing a gap."));
//    Descriptor npg(NO_PGAPS, ClustalWWorker::tr("Residue-specific gaps off"),
//                    ClustalWWorker::tr("Residue-specific penalties are amino specific gap penalties that reduce or increase the gap opening penalties at each position in the alignment."));
//    Descriptor nhg(NO_HGAPS, ClustalWWorker::tr("Hydrophilic gaps off"),
//                    ClustalWWorker::tr("Hydrophilic gap penalties are used to increase the chances of a gap within a run (5 or more residues) of hydrophilic amino acids."));
//    Descriptor iter(ITERATION, ClustalWWorker::tr("Iteration type"),
//                    ClustalWWorker::tr("Alignment improvement iteration type. Can take values: <p> \
//                           <ul> \
//                           <li>None - No iteration;</li> \
//                           <li>Tree - Iteration at each step of alignment process;</li> \
//                           <li>Alignment - Iteation only on final alignment.</li> \
//                           </ul>"));
//    Descriptor ni(NUM_ITERATIONS, ClustalWWorker::tr("Number of iterations"),
//                    ClustalWWorker::tr("The maximum number of iterations to perform."));
//    Descriptor matrix(MATRIX, ClustalWWorker::tr("Weight matrix"),
//                    ClustalWWorker::tr("For proteins it is a scoring table which describes the similarity of each amino acid to each other and can take values: <p> \
//                           <ul> \
//                           <li>BLOSUM - Appear to be the best available for carrying out database similarity (homology searches). \
//                           <li>PAM - Have been extremely widely used since the late '70s. \
//                           <li>Gonnet - Derived using almost the same procedure as the PAM bit are much more up to date. \
//                           <li>ID - Identitical matrix gives a score of 1.0 to two identical amino acids and a score of zero otherwise. \
//                           </ul> \
//                           <p> \
//                           For DNA it is the scores assigned to matches and mismatches. The values available are: \
//                           <ul> \
//                           <li>IUB - The default scoring matrix used by BESTFIT for the comparison of nucleic acid sequences. All matches score 1.9; all mismatches for IUB symbols score 0.\
//                           <li>ClustalW - The previous system used by Clustal W, in which matches score 1.0 and mismatches score 0. All matches for IUB symbols also score 0. \
//                           </ul>"));
//    Descriptor etp(EXT_TOOL_PATH, ClustalWWorker::tr("Tool path"),
//                    ClustalWWorker::tr("Path to the ClustalW tool."
//                        "<p>The default path can be set in the UGENE application settings."));
//
//    Descriptor tdp(TMP_DIR_PATH, ClustalWWorker::tr("Temporary directory"),
//                    ClustalWWorker::tr("Directory to store temporary files."));
//
//    a << new Attribute(gop, BaseTypes::NUM_TYPE(), false, QVariant(53.90));
//    a << new Attribute(gep, BaseTypes::NUM_TYPE(), false, QVariant(8.52));
//    a << new Attribute(gd, BaseTypes::NUM_TYPE(), false, QVariant(4.42));
//    a << new Attribute(eg, BaseTypes::BOOL_TYPE(), false, QVariant(false));
//    a << new Attribute(npg, BaseTypes::BOOL_TYPE(), false, QVariant(false));
//    a << new Attribute(nhg, BaseTypes::BOOL_TYPE(), false, QVariant(false));
//    a << new Attribute(iter, BaseTypes::NUM_TYPE(), false, 0);
//    a << new Attribute(ni, BaseTypes::NUM_TYPE(), false, QVariant(3));
//    a << new Attribute(matrix, BaseTypes::NUM_TYPE(), false, -1);
//    a << new Attribute(etp, BaseTypes::STRING_TYPE(), true, QVariant("Default"));
//    a << new Attribute(tdp, BaseTypes::STRING_TYPE(), true, QVariant("Default"));
//
//    Descriptor desc(ACTOR_ID, ClustalWWorker::tr("Align with ClustalW"),
//        ClustalWWorker::tr("Aligns multiple sequence alignments (MSAs) supplied with ClustalW."
//        "<p>ClustalW is a general purpose multiple sequence alignment program for DNA or proteins."
//        "Visit <a href=\"http://www.clustal.org/\">http://www.clustal.org/</a> to learn more about it."));
//
//    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);
//
//    QMap<QString, PropertyDelegate*> delegates;
//    {
//        QVariantMap m; m["minimum"] = double(.00); m["maximum"] = double(100.00); m["decimals"] = 2;
//        delegates[GAP_OPEN_PENALTY] = new DoubleSpinBoxDelegate(m);
//    }
//    {
//        QVariantMap m; m["minimum"] = double(.00); m["maximum"] = double(10.00); m["decimals"] = 2;
//        delegates[GAP_EXT_PENALTY] = new DoubleSpinBoxDelegate(m);
//    }
//    {
//        QVariantMap m; m["minimum"] = double(.00); m["maximum"] = double(10.00); m["decimals"] = 2;
//        delegates[GAP_DIST] = new DoubleSpinBoxDelegate(m);
//    }
//    {
//        QVariantMap vm;
//        vm["None"] = 0;
//        vm["Tree"] = 1;
//        vm["Alignment"] = 2;
//        delegates[ITERATION] = new ComboBoxDelegate(vm);
//    }
//    {
//        QVariantMap vm;
//        vm["default"] = -1;
//        vm["IUB"] = 0;
//        vm["ClustalW"] = 1;
//        vm["BLOSUM"] = 2;
//        vm["PAM"] = 3;
//        vm["GONNET"] = 4;
//        vm["ID"] = 5;
//        delegates[MATRIX] = new ComboBoxDelegate(vm);
//    }
//    delegates[EXT_TOOL_PATH] = new URLDelegate("", "executable", false);
//    delegates[TMP_DIR_PATH] = new URLDelegate("", "TmpDir", false, true);
//
//    proto->setEditor(new DelegateEditor(delegates));
//    proto->setPrompter(new ClustalWPrompter());
//    proto->setIconPath(":external_tool_support/images/clustalx.png");
//    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_ALIGNMENT(), proto);
//
//    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
//    localDomain->registerEntry(new ClustalWWorkerFactory());
//}
//
///****************************
//* ClustalWPrompter
//****************************/
//ClustalWPrompter::ClustalWPrompter(Actor* p) : PrompterBase<ClustalWPrompter>(p) {
//}
//QString ClustalWPrompter::composeRichDoc() {
//    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_MSA_PORT_ID()));
//    Actor* producer = input->getProducer(BasePorts::IN_MSA_PORT_ID());
//    QString producerName = producer ? tr(" from %1").arg(producer->getLabel()) : "";    
//    QString doc = tr("Aligns each MSA supplied <u>%1</u> with \"<u>ClustalW</u>\".")
//        .arg(producerName);
//
//    return doc;
//}
///****************************
//* ClustalWWorker
//****************************/
//ClustalWWorker::ClustalWWorker(Actor* a) : BaseWorker(a), input(NULL), output(NULL) {
//}
//
//void ClustalWWorker::init() {
//    input = ports.value(BasePorts::IN_MSA_PORT_ID());
//    output = ports.value(BasePorts::OUT_MSA_PORT_ID());
//}
//
//bool ClustalWWorker::isReady() {
//    return (input && input->hasMessage());
//}
//
//Task* ClustalWWorker::tick() {
//    Message inputMessage = getMessageAndSetupScriptValues(input);
//    cfg.gapOpenPenalty=actor->getParameter(GAP_OPEN_PENALTY)->getAttributeValue<float>();
//    cfg.gapExtenstionPenalty=actor->getParameter(GAP_EXT_PENALTY)->getAttributeValue<float>();
//    cfg.gapDist=actor->getParameter(GAP_DIST)->getAttributeValue<float>();
//    cfg.endGaps=actor->getParameter(END_GAPS)->getAttributeValue<bool>();
//    cfg.noHGaps=actor->getParameter(NO_HGAPS)->getAttributeValue<bool>();
//    cfg.noPGaps=actor->getParameter(NO_PGAPS)->getAttributeValue<bool>();
//    if(actor->getParameter(ITERATION)->getAttributeValue<int>() != 0){
//        if(actor->getParameter(ITERATION)->getAttributeValue<int>() == 1){
//            cfg.iterationType="TREE";
//        }else if(actor->getParameter(ITERATION)->getAttributeValue<int>() == 2){
//            cfg.iterationType="ALIGNMENT";
//        }
//        if(actor->getParameter(NUM_ITERATIONS)->getAttributeValue<int>() != 3){
//            cfg.numIterations=actor->getParameter(NUM_ITERATIONS)->getAttributeValue<int>();
//        }
//    }
//    if(actor->getParameter(MATRIX)->getAttributeValue<int>() == -1){
//        if(actor->getParameter(MATRIX)->getAttributeValue<int>() == 0){
//            cfg.matrix="IUB";
//        }else if(actor->getParameter(MATRIX)->getAttributeValue<int>() == 1){
//            cfg.matrix="CLUSTALW";
//        }else if(actor->getParameter(MATRIX)->getAttributeValue<int>() == 2){
//            cfg.matrix="BLOSUM";
//        }else if(actor->getParameter(MATRIX)->getAttributeValue<int>() == 3){
//            cfg.matrix="PAM";
//        }else if(actor->getParameter(MATRIX)->getAttributeValue<int>() == 4){
//            cfg.matrix="GONNET";
//        }else if(actor->getParameter(MATRIX)->getAttributeValue<int>() == 5){
//            cfg.matrix="ID";
//        }
//    }
//
//    QString path=actor->getParameter(EXT_TOOL_PATH)->getAttributeValue<QString>();
//    if(QString::compare(path, "default", Qt::CaseInsensitive) != 0){
//        AppContext::getExternalToolRegistry()->getByName(CLUSTAL_TOOL_NAME)->setPath(path);
//    }
//    path=actor->getParameter(TMP_DIR_PATH)->getAttributeValue<QString>();
//    if(QString::compare(path, "default", Qt::CaseInsensitive) != 0){
//        AppContext::getAppSettings()->getUserAppsSettings()->setTemporaryDirPath(path);
//    }
//    MAlignment msa = inputMessage.getData().toMap().value(BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId()).value<MAlignment>();
//    
//    if( msa.isEmpty() ) {
//        return new FailTask(tr("An empty MSA has been supplied to ClustalW."));
//    }
//    Task* t = new ClustalWSupportTask(new MAlignmentObject(msa), cfg);
//    connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
//    return t;
//}
//
//void ClustalWWorker::sl_taskFinished() {
//    ClustalWSupportTask* t = qobject_cast<ClustalWSupportTask*>(sender());
//    if (t->getState() != Task::State_Finished) return;
//    QVariant v = qVariantFromValue<MAlignment>(t->resultMA);
//    output->put(Message(BaseTypes::MULTIPLE_ALIGNMENT_TYPE(), v));
//    if (input->isEnded()) {
//        output->setEnded();
//    }
//    algoLog.info(tr("Aligned %1 with ClustalW").arg(t->resultMA.getName()));
//}
//
//bool ClustalWWorker::isDone() {
//    return !input || input->isEnded();
//}
//
//void ClustalWWorker::cleanup() {
//}

} //namespace LocalWorkflow
} //namespace U2
