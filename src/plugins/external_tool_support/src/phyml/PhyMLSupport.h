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

#ifndef _U2_PHYML_SUPPORT_H_
#define _U2_PHYML_SUPPORT_H_

#include <U2Algorithm/PhyTreeGenerator.h>

#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/PhyTree.h>

#include <U2View/CreatePhyTreeDialogController.h>

namespace U2 {

class PhyMLSupport : public ExternalTool {
    Q_OBJECT
public:
    PhyMLSupport(const QString& name);

    static const QString PhyMlRegistryId;
    static const QString PhyMlTempDir;
};

class PhyMLAdapter : public PhyTreeGenerator{
public:
    virtual Task* createCalculatePhyTreeTask(const MAlignment& ma, const CreatePhyTreeSettings& s) ;
    virtual void setupCreatePhyTreeUI(CreatePhyTreeDialogController* c, const MAlignment& ma);
};

enum SubstModelTrRatioType {
    ANY_TT_RATIO,
    ONLY_FIXED_TT_RATIO,
    ONLY_ESTIMATED_TT_RATIO,
    WITHOUT_TT_RATIO
};

class PhyMLModelTypes{
public:
    static const QStringList& getAminoAcidModelTypes() {return aminoSubstitutionModels;}
    static const QStringList& getDnaModelTypes() {return dnaSubstitutionModels;}
    static SubstModelTrRatioType getTtRatioType(const QString& modelName); 
private:
    static const QStringList dnaSubstitutionModels;//all dna models
    static const QStringList dnaModelsWithFixedTtRatio;//only fixed Transition / transversion ratio 
    static const QStringList dnaModelsWithEstimatedTtRatio;//only estimated Transition / transversion ratio 
    static const QStringList aminoSubstitutionModels;
};

class PhyMLRatioTestsTypes{
public:
    static const QStringList& getRatioTestsTypes() {return ratioTestsTypes;}
private:
    static const QStringList ratioTestsTypes;
};

class TreeSearchingParams {
public:
    static const QStringList& getInputTreeTypes() {return inputTreeTypes;}
    static const QStringList& getTreeImprovementTypes() {return treeImprovementTypes;}
private:
    static const QStringList inputTreeTypes;
    static const QStringList treeImprovementTypes;
};

}   // namespace U2
#endif // _U2_PHYML_SUPPORT_H_
