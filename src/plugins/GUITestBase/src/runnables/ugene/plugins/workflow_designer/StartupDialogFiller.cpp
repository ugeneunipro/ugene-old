#include "StartupDialogFiller.h"

#include <QtGui/QApplication>
#include <QtGui/QPushButton>

#include "api/GTWidget.h"

namespace U2{

#define GT_CLASS_NAME "GTUtilsDialog::StartupDialogFiller"
#define GT_METHOD_NAME "run"

void StartupDialogFiller::run(){

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QList<QPushButton*> list= dialog->findChildren<QPushButton*>();

    foreach(QPushButton* but, list){
        if (but->text()=="Don't use the directory")
            GTWidget::click(os,but);
    }
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
}
