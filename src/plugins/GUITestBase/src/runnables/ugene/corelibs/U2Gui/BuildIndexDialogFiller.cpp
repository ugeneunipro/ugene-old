#include "BuildIndexDialogFiller.h"

#include "api/GTFileDialog.h"
#include "api/GTComboBox.h"
#include "api/GTWidget.h"
#include "api/GTLineEdit.h"
#include <QtGui/QApplication>
#include <QtGui/QComboBox>
#include <QtGui/QLineEdit>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::BuildIndexDialogFiller"
#define GT_METHOD_NAME "run"
void BuildIndexDialogFiller::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QComboBox* methodNamesBox = dialog->findChild<QComboBox*>("methodNamesBox");
    for(int i=0; i < methodNamesBox->count();i++){
        if(methodNamesBox->itemText(i) == method){
            GTComboBox::setCurrentIndex(os, methodNamesBox, i);
        }
    }

    GTFileDialogUtils *ob = new GTFileDialogUtils(os, refPath, refFileName);
    GTUtilsDialog::waitForDialog(os, ob);
    GTWidget::click(os, GTWidget::findWidget(os, "addRefButton",dialog));

    if (!useDefaultIndexName) {
        QLineEdit* indexFileNameEdit = dialog->findChild<QLineEdit*>("indexFileNameEdit");
        indexFileNameEdit->clear();
        indexFileNameEdit->setText(indPath + indFileName);
    }

    GTWidget::click(os, GTWidget::findWidget(os, "buildButton", dialog));
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

} // namespace
