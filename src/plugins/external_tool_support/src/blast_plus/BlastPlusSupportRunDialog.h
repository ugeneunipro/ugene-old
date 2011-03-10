#ifndef _U2_BLAST_PLUS_SUPPORT_RUN_DIALOG_H
#define _U2_BLAST_PLUS_SUPPORT_RUN_DIALOG_H

#include <QDialog>
#include <U2Core/DNASequenceObject.h>
#include <U2Misc/DialogUtils.h>
#include <ui/ui_BlastAllSupportDialog.h>
#include <U2Gui/CreateAnnotationWidgetController.h>
#include "BlastPlusSupportCommonTask.h"
#include "utils/BlastRunCommonDialog.h"

namespace U2 {

class BlastPlusSupportRunDialog : public BlastRunCommonDialog {
    Q_OBJECT
public:
    BlastPlusSupportRunDialog(DNASequenceObject* dnaso, BlastTaskSettings& settings, QString &_lastDBPath, QString &_lastDBName, QWidget* parent);
protected slots:
    virtual void sl_runQuery();
    virtual void sl_lineEditChanged();

private:
    DNASequenceObject*  dnaso;
    bool checkToolPath();
    QString &lastDBPath;
    QString &lastDBName;
};

class BlastPlusWithExtFileSpecifySupportRunDialog : public BlastRunCommonDialog {
    Q_OBJECT
public:
    BlastPlusWithExtFileSpecifySupportRunDialog(QList<BlastTaskSettings>& settingsList, QString &_lastDBPath, QString &_lastDBName, QWidget* parent);
protected slots:
    void sl_runQuery();
    void sl_lineEditChanged();
private slots:
    void sl_cancel();

    void sl_inputFileLineEditChanged(const QString& str);
    void sl_inputFileLoadTaskStateChanged();
private:
    bool checkToolPath();

    FileLineEdit*   inputFileLineEdit;
    bool            wasNoOpenProject;

    QList<BlastTaskSettings>&   settingsList;
    QList<GObjectReference>     sequencesRefList;
    QString &lastDBPath;
    QString &lastDBName;
};
}//namespace
#endif // _U2_BLAST_PLUS_SUPPORT_RUN_DIALOG_H
