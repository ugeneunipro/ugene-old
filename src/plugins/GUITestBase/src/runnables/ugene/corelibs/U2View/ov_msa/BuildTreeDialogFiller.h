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

#ifndef _U2_GT_RUNNABLES_BUILD_TREE_DIALOG_FILLER_H_
#define _U2_GT_RUNNABLES_BUILD_TREE_DIALOG_FILLER_H_

#include "GTUtilsDialog.h"

namespace U2 {

class BuildTreeDialogFiller : public Filler {
public:
    enum ConsensusType {
        MAJORITYEXT,
        STRICTCONSENSUS,
        MAJORITY,
        M1
    };

    BuildTreeDialogFiller(U2OpStatus &os, QString _saveTree = "default", int _model = 0, double _alpha = 0, bool _displayWithMsa = false);
    BuildTreeDialogFiller(U2OpStatus &os, int _replicates=0, QString _saveTree = "default", int _seed = 5, ConsensusType _type = MAJORITYEXT, double _fraction = 0.5);
    BuildTreeDialogFiller(U2OpStatus &os, CustomScenario *cs);

    void commonScenario();

private:
    QString saveTree;
    int model,replicates,seed;
    double alpha,fraction;
    ConsensusType type;
    bool displayWithMsa;
};

class BuildTreeDialogFillerPhyML : public Filler {
 public:
    BuildTreeDialogFillerPhyML(U2OpStatus &os, bool _freqOptimRadioPressed);
    void run();

private:
    bool freqOptimRadioPressed;
};

}

#endif //_U2_GT_RUNNABLES_BUILD_TREE_DIALOG_FILLER_H_
