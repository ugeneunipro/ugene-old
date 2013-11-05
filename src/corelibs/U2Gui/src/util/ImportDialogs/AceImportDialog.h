#ifndef _U2_ACE_IMPORT_DIALOG_H_
#define _U2_ACE_IMPORT_DIALOG_H_

#include "ui_AceImportDialog.h"

#include <U2Formats/AceImporter.h>

namespace U2 {

class AceImportDialog : public ImportDialog, public Ui_AceImportDialog {
    Q_OBJECT
public:
    AceImportDialog(const QVariantMap& settings);

private slots:
    void sl_selectFileClicked();

protected:
    bool isValid();
    void applySettings();

    static const QString EXTENSION;
};

}   // namespace U2

#endif // _U2_ACE_IMPORT_DIALOG_H_
