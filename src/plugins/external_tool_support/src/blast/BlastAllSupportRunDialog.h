#ifndef _U2_BLASTALL_SUPPORT_RUN_DIALOG_H
#define _U2_BLASTALL_SUPPORT_RUN_DIALOG_H

#include <QDialog>
#include <U2Core/DNASequenceObject.h>
#include <U2Misc/DialogUtils.h>
#include <ui/ui_BlastAllSupportDialog.h>
#include <U2Gui/CreateAnnotationWidgetController.h>
#include "BlastAllSupportTask.h"
#include "utils/BlastRunCommonDialog.h"

namespace U2 {

class BlastAllSupportRunDialog : public BlastRunCommonDialog {
    Q_OBJECT
public:
    BlastAllSupportRunDialog(DNASequenceObject* dnaso, BlastTaskSettings& settings, QString &_lastDBPath, QString &_lastDBName, QWidget* parent);
protected slots:
    virtual void sl_runQuery();
    virtual void sl_lineEditChanged();

private:
    DNASequenceObject*  dnaso;
    QString &lastDBPath;
    QString &lastDBName;
};

class BlastAllWithExtFileSpecifySupportRunDialog : public BlastRunCommonDialog {
    Q_OBJECT
public:
    BlastAllWithExtFileSpecifySupportRunDialog(QList<BlastTaskSettings>& settings, QString &_lastDBPath, QString &_lastDBName, QWidget* parent);
protected slots:
    void sl_runQuery();
    void sl_lineEditChanged();
private slots:
    void sl_cancel();

    void sl_inputFileLineEditChanged(const QString& str);
    void sl_inputFileLoadTaskStateChanged();
private:
    FileLineEdit*   inputFileLineEdit;
    bool            wasNoOpenProject;

    QList<BlastTaskSettings>&   settingsList;
    QList<GObjectReference>     sequencesRefList;
    QString &lastDBPath;
    QString &lastDBName;
};
}//namespace
#endif // _U2_BLASTALL_SUPPORT_RUN_DIALOG_H
