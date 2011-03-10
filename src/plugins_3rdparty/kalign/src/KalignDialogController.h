#ifndef _U2_KALIGN_ALIGN_DIALOG_CONTROLLER_H_
#define _U2_KALIGN_ALIGN_DIALOG_CONTROLLER_H_

#include <QtGui/QDialog>

#include <U2Core/GAutoDeleteList.h>
#include <U2Misc/DialogUtils.h>

#include <ui/ui_KalignDialog.h>
#include "KalignTask.h"

namespace U2 {

class KalignDialogController : public QDialog, public Ui_KalignDialog {
    Q_OBJECT

public:
    KalignDialogController(QWidget* w, const MAlignment& ma, KalignTaskSettings& settings);

public slots:
    void accept();

private:
    void setupUiExt(); 
    MAlignment                          ma;
    KalignTaskSettings&                 settings;
};

class KalignWithExtFileSpecifyDialogController : public QDialog, public Ui_KalignDialog {
        Q_OBJECT
public:
        KalignWithExtFileSpecifyDialogController(QWidget* w, KalignTaskSettings& settings);
public slots:
    void accept();
private slots:
    void sl_inputFileLineEditChanged(const QString& str);
private:
    void setupUiExt(); 
    KalignTaskSettings&     settings;
    FileLineEdit*           inputFileLineEdit;
};


}//namespace
#endif
