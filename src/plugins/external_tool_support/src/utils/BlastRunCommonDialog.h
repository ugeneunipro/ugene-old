#ifndef _U2_BLAST_RUN_COMMON_DIALOG_H
#define _U2_BLAST_RUN_COMMON_DIALOG_H

#include <QDialog>
#include <U2Core/DNASequenceObject.h>
#include <U2Misc/DialogUtils.h>
#include <ui/ui_BlastAllSupportDialog.h>
#include <U2Gui/CreateAnnotationWidgetController.h>
#include "BlastTaskSettings.h"

namespace U2 {

class BlastRunCommonDialog : public QDialog, public Ui_BlastAllSupportDialog {
    Q_OBJECT
public:
    BlastRunCommonDialog(BlastTaskSettings& settings, QWidget* parent);
protected slots:
    virtual void sl_runQuery() = 0;
    virtual void sl_lineEditChanged() = 0;

    void sl_onBrowseDatabasePath();//=

    void sl_onProgNameChange(int i);//=
    void sl_onMatchScoresChanged(int i);//=
    void sl_onMatrixChanged(int i);//=
    void sl_megablastChecked();//=
    void sl_restoreDefault();//=
protected:
    BlastTaskSettings&                  settings;
    bool                                needRestoreDefault;
    CreateAnnotationWidgetController*   ca_c;
};
}//namespace
#endif // _U2_BLAST_RUN_COMMON_DIALOG_H
