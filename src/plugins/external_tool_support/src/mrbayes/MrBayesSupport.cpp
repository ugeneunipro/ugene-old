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

#include "MrBayesTask.h"
#include "MrBayesTests.h"
#include "MrBayesSupport.h"
#include "MrBayesDialogWidget.h"

#include <U2Core/AppContext.h>
#include <U2Algorithm/PhyTreeGeneratorRegistry.h>

#include <U2Core/GAutoDeleteList.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/DialogUtils.h>

#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTest.h>
#include <U2Test/GTestFrameworkComponents.h>

namespace U2 {

MrBayesSupport::MrBayesSupport(const QString& name, const QString& path) : ExternalTool(name, path)
{
    if (AppContext::getMainWindow()) {
        icon = QIcon(":external_tool_support/images/mrbayes.png");
        grayIcon = QIcon(":external_tool_support/images/mrbayes_gray.png");
        warnIcon = QIcon(":external_tool_support/images/mrbayes_warn.png");
    }
#ifdef Q_OS_WIN
    executableFileName="mrbayes.exe";
#else
    #ifdef Q_OS_LINUX
    executableFileName="mrbayes"; //need check on unix
    #endif
#endif
    validationArguments<<"";
    validMessage="MrBayes";
    description=tr("<i>MrBayes</i> MrBayes is a program for the Bayesian estimation of phylogeny." 
                   "Bayesian inference of phylogeny is based upon a quantity called the posterior "
                   "probability distribution of trees, which is the probability of a tree conditioned "
                   "on the observations. The conditioning is accomplished using Bayes's theorem. "
                   "The posterior probability distribution of trees is impossible to calculate analytically; "
                   "instead, MrBayes uses a simulation technique called Markov chain Monte Carlo (or MCMC) "
                   "to approximate the posterior probabilities of trees.");
    versionRegExp=QRegExp("MrBayes v(\\d+\\.\\d+\\.\\d+)");
    toolKitName="MrBayes";

    //register the method  
    PhyTreeGeneratorRegistry* registry = AppContext::getPhyTreeGeneratorRegistry();
    registry->registerPhyTreeGenerator(new MrBayesAdapter(), MRBAYES_TOOL_NAME);

//     GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
//     XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
//     assert(xmlTestFormat!=NULL);
// 
//     GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
//     l->qlist = MrBayesToolTests::createTestFactories();
// 
//     foreach(XMLTestFactory* f, l->qlist) { 
//         bool res = xmlTestFormat->registerTestFactory(f);
//         Q_UNUSED(res);
//         assert(res);
//     }
}

////////////////////////////////////////
//MrBayesAdapter

Task* MrBayesAdapter::createCalculatePhyTreeTask(const MAlignment& ma, const CreatePhyTreeSettings& s){
    return new MrBayesSupportTask(ma, s);
}
void MrBayesAdapter::setupCreatePhyTreeUI(CreatePhyTreeDialogController* c, const MAlignment& ma){
    CreatePhyTreeWidget* w = new MrBayesWidget(c, ma);
    c->insertContrWidget(1,w);
}

////////////////////////////////////////
//MrBayesModelTypes

QString MrBayesModelTypes::poisson("poisson");
QString MrBayesModelTypes::jones("jones");
QString MrBayesModelTypes::dayhoff("dayhoff");
QString MrBayesModelTypes::mtrev("mtrev");
QString MrBayesModelTypes::mtmam("mtmam");
QString MrBayesModelTypes::wag("wag");
QString MrBayesModelTypes::rtrev("rtrev");
QString MrBayesModelTypes::cprev("cprev");
QString MrBayesModelTypes::vt("vt");
QString MrBayesModelTypes::blosum("blosum");
QString MrBayesModelTypes::equalin("equalin");
QList<QString> MrBayesModelTypes::getAAModelTypes(){
    static QList<QString> list;
    if (list.isEmpty()) {
        list.append(MrBayesModelTypes::poisson);
        list.append(MrBayesModelTypes::jones);
        list.append(MrBayesModelTypes::dayhoff);
        list.append(MrBayesModelTypes::mtrev);
        list.append(MrBayesModelTypes::mtmam);
        list.append(MrBayesModelTypes::wag);
        list.append(MrBayesModelTypes::rtrev);
        list.append(MrBayesModelTypes::cprev);
        list.append(MrBayesModelTypes::vt);
        list.append(MrBayesModelTypes::blosum);
        list.append(MrBayesModelTypes::equalin);
    }

    return list;
}

QString MrBayesModelTypes::JC69("JC69 (Nst=1)");
QString MrBayesModelTypes::HKY85("HKY85 (Nst=2)");
QString MrBayesModelTypes::GTR("GTR (Nst=6)");
QString MrBayesModelTypes::Codon("Codon");
QList<QString> MrBayesModelTypes::getSubstitutionModelTypes(){
    static QList<QString> list;
    if (list.isEmpty()) {
        list.append(MrBayesModelTypes::JC69);
        list.append(MrBayesModelTypes::HKY85);
        list.append(MrBayesModelTypes::GTR);
        //list.append(MrBayesModelTypes::Codon);
    }

    return list;
}

QString MrBayesVariationTypes::equal("equal");
QString MrBayesVariationTypes::gamma("gamma");
QString MrBayesVariationTypes::propinv("propinv");
QString MrBayesVariationTypes::invgamma("invgamma");
QList<QString> MrBayesVariationTypes::getVariationTypes(){
    static QList<QString> list;
    if (list.isEmpty()) {
        list.append(MrBayesVariationTypes::equal);
        list.append(MrBayesVariationTypes::gamma);
        list.append(MrBayesVariationTypes::propinv);
        list.append(MrBayesVariationTypes::invgamma);
    }

    return list;
}
}//namespace
