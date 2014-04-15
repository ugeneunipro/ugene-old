#include "RemoteBLASTDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTSpinBox.h"
#include "api/GTDoubleSpinBox.h"
#include "api/GTCheckBox.h"
#include "api/GTLineEdit.h"
#include "api/GTComboBox.h"
#include "api/GTRadioButton.h"
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QGroupBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialogButtonBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#endif

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::RemoteBLASTDialogFiller"
#define GT_METHOD_NAME "run"

void RemoteBLASTDialogFiller::run() {
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QDialogButtonBox *buttonBox = dialog->findChild<QDialogButtonBox*>(QString::fromUtf8("buttonBox"));
    GT_CHECK(buttonBox != NULL, "buttonBox not found");

    QPushButton *button = buttonBox->button(QDialogButtonBox::Ok);
    GT_CHECK(button != NULL, "standart button not found");
    GTWidget::click(os, button);

}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
