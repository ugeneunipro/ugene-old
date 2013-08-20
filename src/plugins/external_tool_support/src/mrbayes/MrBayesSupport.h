/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_MRBAYES_SUPPORT_H
#define _U2_MRBAYES_SUPPORT_H

#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/PhyTree.h>
#include <U2View/CreatePhyTreeDialogController.h>
#include <U2Algorithm/PhyTreeGenerator.h>

#define ET_MRBAYES "MrBayes"
#define MRBAYES_TMP_DIR   "mrbayes"

namespace U2 {

class MrBayesSupport : public ExternalTool {
    Q_OBJECT
public:
    MrBayesSupport(const QString& name, const QString& path = "");
};

class MrBayesAdapter : public PhyTreeGenerator{
public:
    virtual Task* createCalculatePhyTreeTask(const MAlignment& ma, const CreatePhyTreeSettings& s) ;
    virtual void setupCreatePhyTreeUI(CreatePhyTreeDialogController* c, const MAlignment& ma);
};

class MrBayesModelTypes{
public:
    static QString poisson;
    static QString jones;
    static QString dayhoff;
    static QString mtrev;
    static QString mtmam;
    static QString wag;
    static QString rtrev;
    static QString cprev;
    static QString vt;
    static QString blosum;
    static QString equalin;
    static QList<QString> getAAModelTypes();

    static QString JC69;
    static QString HKY85;
    static QString GTR;
    static QString Codon;
    static QList<QString> getSubstitutionModelTypes();

};

class MrBayesVariationTypes{
public:
    static QString equal;
    static QString gamma;
    static QString propinv;
    static QString invgamma;
    static QList<QString> getVariationTypes();
};

}//namespace
#endif // _U2_MRBAYES_SUPPORT_H
