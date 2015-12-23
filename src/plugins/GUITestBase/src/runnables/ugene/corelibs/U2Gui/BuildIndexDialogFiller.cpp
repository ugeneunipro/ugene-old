#include "BuildIndexDialogFiller.h"

#include <base_dialogs/GTFileDialog.h>
#include <primitives/GTComboBox.h>
#include <primitives/GTWidget.h>
#include <primitives/GTLineEdit.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <QtGui/QDialogButtonBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#endif

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::BuildIndexDialogFiller"
#define GT_METHOD_NAME "commonScenario"
void BuildIndexDialogFiller::commonScenario() {

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

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

} // namespace
