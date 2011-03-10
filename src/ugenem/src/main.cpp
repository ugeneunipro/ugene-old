#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

#include "SendReportDialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString message;
    if(argc > 1) {
        message = QString::fromUtf8(QByteArray::fromBase64(argv[1]));
    } else {
        message = "";
    }
    SendReportDialog dlg(message);
    dlg.setWindowIcon(QIcon(":ugenem/images/crash_icon.png"));
    dlg.exec();
    return 0;
}