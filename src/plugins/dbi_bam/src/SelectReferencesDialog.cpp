#include "SelectReferencesDialog.h"

namespace U2 {
namespace BAM {

SelectReferencesDialog::SelectReferencesDialog(const QList<Header::Reference> &references) :
    QDialog(NULL),
    references(references)
{
    ui.setupUi(this);

    for(int index = 0;index < references.size();index++) {
        referenceUrls.append(GUrl());

        ui.referencesTable->insertRow(index);
        ui.referencesTable->setItem(index, 0, new QTableWidgetItem(QString(references[index].getName())));
        ui.referencesTable->setItem(index, 1, new QTableWidgetItem(QString::number(references[index].getLength())));
    }
}

const QList<GUrl> &SelectReferencesDialog::getReferenceUrls()const {
    return referenceUrls;
}

} // namespace BAM
} // namespace U2
