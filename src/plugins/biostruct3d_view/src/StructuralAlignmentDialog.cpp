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

#include "StructuralAlignmentDialog.h"

#include <U2Core/GObject.h>
#include <U2Core/BioStruct3DObject.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/AppContext.h>
#include <U2Algorithm/StructuralAlignmentAlgorithm.h>
#include <U2Algorithm/StructuralAlignmentAlgorithmRegistry.h>

#include <QMessageBox>

namespace U2 {

/* class StructuralAlignmentDialog : public QDialog, public Ui::StructuralAlignmentDialog */

static QList<BioStruct3DObject*> findAvailableBioStructs() {
    QList<GObject*> objs = GObjectUtils::findAllObjects(UOF_LoadedOnly, GObjectTypes::BIOSTRUCTURE_3D);
    QList<BioStruct3DObject*> biostructs;
    foreach (GObject *obj, objs) {
        BioStruct3DObject *bso = qobject_cast<BioStruct3DObject*> (obj);
        assert(bso);
        biostructs << bso;
    }

    return biostructs;
}

StructuralAlignmentDialog::StructuralAlignmentDialog(const BioStruct3DObject *fixedRef/* = 0*/, int fixedRefModel/* = 0*/, QWidget *parent/* = 0*/)
        : QDialog(parent), task(0)
{
    setupUi(this);

    QList<BioStruct3DObject*> biostructs = findAvailableBioStructs();

    foreach (BioStruct3DObject *bs, biostructs) {
        reference->addItem(bs->getGObjectName(), qVariantFromValue((void*)bs));
        alter->addItem(bs->getGObjectName(), qVariantFromValue((void*)bs));
    }

    createModelLists();

    if (fixedRef) {
        int idx = reference->findData(qVariantFromValue((void*)fixedRef));
        assert(idx != -1 && "Fixed ref must be in biostructs");
        reference->setCurrentIndex(idx);
        reference->setDisabled(true);
        refModel->setDisabled(true);
    }

    if (fixedRefModel) {
        int idx = refModel->findData(qVariantFromValue(fixedRefModel));
        assert(idx != -1 && "Fixed ref model must be in ref biostruct");
        refModel->setCurrentIndex(idx);
    }

    connect(reference, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_biostructChanged(int)));
    connect(alter, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_biostructChanged(int)));
}

void StructuralAlignmentDialog::createModelLists() {
    createModelList(reference, reference->currentIndex(), refModel);
    createModelList(alter, alter->currentIndex(), altModel);
}

void StructuralAlignmentDialog::createModelList(QComboBox *biostruct, int idx, QComboBox *model) {
    BioStruct3DObject *bso = static_cast<BioStruct3DObject*>( biostruct->itemData(idx).value<void*>() );
    model->clear();
    foreach (const int modelId, bso->getBioStruct3D().modelMap.keys()) {
        model->addItem(QString::number(modelId), qVariantFromValue(modelId));
    }
}

void StructuralAlignmentDialog::sl_biostructChanged(int idx) {
    QObject *combo = sender();

    if (combo == reference) {
        createModelList(reference, idx, refModel);
    }
    else if (combo == alter) {
        createModelList(alter, idx, altModel);
    }
    else {
        assert(!"this handler only for reference and alter combos");
    }
}

void StructuralAlignmentDialog::accept() {
    StructuralAlignmentAlgorithmRegistry *reg = AppContext::getStructuralAlignmentAlgorithmRegistry();
    if (reg->getFactoriesIds().isEmpty()) {
        QMessageBox::warning(0, "Error", "No available algorithms, make sure that ptools plugin loaded");
        return;
    }

    BioStruct3DObject *refo = static_cast<BioStruct3DObject*>( reference->itemData(reference->currentIndex()).value<void*>() );
    BioStruct3DObject *alto = static_cast<BioStruct3DObject*>( alter->itemData(alter->currentIndex()).value<void*>() );

    alto = qobject_cast<BioStruct3DObject*>(alto->clone());

    const BioStruct3D &ref = refo->getBioStruct3D();
    const BioStruct3D &alt = alto->getBioStruct3D();

    if (ref.getNumberOfAtoms() != alt.getNumberOfAtoms()) {
        QMessageBox::warning(0, "Error", "Structures should have the same length");
        return;
    }

    int altModelName = altModel->itemData(altModel->currentIndex()).value<int>();
    int refModelName = refModel->itemData(refModel->currentIndex()).value<int>();

    // TODO: option for selecting chains
    StructuralAlignmentTaskSettings settings = { BioStruct3DReference(refo, ref.moleculeMap.keys(), refModelName),
                                                 BioStruct3DReference(alto, alt.moleculeMap.keys(), altModelName) };


    // TODO: option for selecting algorithm
    task = reg->createStructuralAlignmentTask(reg->getFactoriesIds().first(), settings);

    QDialog::accept();
}

}   // namespace U2
