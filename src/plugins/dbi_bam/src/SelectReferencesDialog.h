#ifndef _U2_BAM_SELECT_REFERENCES_DIALOG_H_
#define _U2_BAM_SELECT_REFERENCES_DIALOG_H_

#include <QDialog>
#include "ui_SelectReferencesDialog.h"
#include "Header.h"

namespace U2 {
namespace BAM {

class SelectReferencesDialog : public QDialog
{
    Q_OBJECT

public:
    SelectReferencesDialog(const QList<Header::Reference> &references);

    const QList<GUrl> &getReferenceUrls()const;

private:
    Ui::SelectReferencesDialog ui;

    QList<Header::Reference> references;
    QList<GUrl> referenceUrls;
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_SELECT_REFERENCES_DIALOG_H_
