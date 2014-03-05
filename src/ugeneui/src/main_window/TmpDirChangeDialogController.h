#ifndef _U2_TMP_DIR_CHANGE_DIALOG_CONTROLLER_
#define _U2_TMP_DIR_CHANGE_DIALOG_CONTROLLER_

#include <ui/ui_TmpDirChangeDialog.h>


#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QLabel>
#include <QtGui/QDialog>
#include <QtGui/QFileDialog>
#else
#include <QtWidgets/QLabel>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFileDialog>
#endif

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/TmpDirChecker.h>

namespace U2 {

class TmpDirChangeDialogController : public QDialog, public Ui_TmpDirChangeDialog {
    Q_OBJECT
public:
    TmpDirChangeDialogController(QString path, QWidget* p);
    QString getTmpDirPath();
public slots:
    void sl_changeDirButtonClicked();
    void sl_exitAppButtonClicked();
    void sl_okButtonClicked();
private:
    QString tmpDirPath;
    TmpDirChecker* tmpDirChecker;
};

} //namespace

#endif // _U2_TMP_DIR_CHANGE_DIALOG_CONTROLLER_
