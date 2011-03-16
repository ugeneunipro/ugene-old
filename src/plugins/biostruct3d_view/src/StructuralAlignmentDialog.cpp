#include "StructuralAlignmentDialog.h"

#include <U2Core/GObject.h>
#include <U2Core/BioStruct3DObject.h>

namespace U2 {

/* class StructuralAlignmentDialog : public QDialog, public Ui::StructuralAlignmentDialog */
StructuralAlignmentDialog::StructuralAlignmentDialog(const QList<BioStruct3DObject*> biostructs, const BioStruct3DObject *fixedRef/* = 0*/, int fixedRefModel/* = 0*/, QWidget *parent/* = 0*/)
    : QDialog(parent)
{
    setupUi(this);

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
    QDialog::accept();
}

}   // namespace U2
