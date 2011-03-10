#ifndef _U2_MAFFT_SUPPORT_RUN_DIALOG_H
#define _U2_MAFFT_SUPPORT_RUN_DIALOG_H

#include <QtGui/QDialog>
#include <U2Misc/DialogUtils.h>
#include <ui/ui_MAFFTSupportRunDialog.h>
#include "MAFFTSupportTask.h"

namespace U2 {

class MAFFTSupportRunDialog : public QDialog, public Ui_MAFFTSupportRunDialog {
    Q_OBJECT
public:
    MAFFTSupportRunDialog(MAFFTSupportTaskSettings& settings, QWidget* parent);
private slots:
    void sl_align();
private:
    MAFFTSupportTaskSettings&        settings;
};

class MAFFTWithExtFileSpecifySupportRunDialog : public QDialog, public Ui_MAFFTSupportRunDialog {
    Q_OBJECT
public:
    MAFFTWithExtFileSpecifySupportRunDialog(MAFFTSupportTaskSettings& settings, QWidget* parent);
private slots:
    void sl_align();
    void sl_inputFileLineEditChanged(const QString& str);
private:
    MAFFTSupportTaskSettings&     settings;
    FileLineEdit*                   inputFileLineEdit;
};

}//namespace
#endif // _U2_MAFFT_SUPPORT_RUN_DIALOG_H
