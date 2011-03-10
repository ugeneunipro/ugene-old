#ifndef _U2_MUSCLE_ALIGN_DIALOG_CONTROLLER_H_
#define _U2_MUSCLE_ALIGN_DIALOG_CONTROLLER_H_

#include "MuscleTask.h"

#include <U2Core/GAutoDeleteList.h>
#include <U2Misc/DialogUtils.h>

#include <ui/ui_MuscleAlignDialog.h>

#include <QtGui/QDialog>

namespace U2 {

class MuscleAlignPreset {
public:
    virtual ~MuscleAlignPreset(){}
    virtual void apply(MuscleTaskSettings& ts) const = 0;

    QString name;
    QString desc;
};

class DefaultModePreset : public MuscleAlignPreset {
public:
    DefaultModePreset();
    virtual void apply(MuscleTaskSettings& ts) const {
        //no options
        ts.reset();
    }
};

class LargeModePreset : public MuscleAlignPreset {
public:
    LargeModePreset();

    virtual void apply(MuscleTaskSettings& ts) const {
        ts.reset();
        ts.maxIterations = 2;
    }
};

class RefineModePreset : public MuscleAlignPreset {
public:
    RefineModePreset();
    virtual void apply(MuscleTaskSettings& ts) const {
        ts.reset();
        ts.op = MuscleTaskOp_Refine;
    }
};


class MuscleAlignDialogController : public QDialog, public Ui_MuscleAlignmentDialog {
    Q_OBJECT

public:
    MuscleAlignDialogController(QWidget* w, const MAlignment& ma, MuscleTaskSettings& settings);
    
public slots:
    void accept();

private slots:
    void sl_onPresetChanged(int current);
    
private:
    void initPresets();

    MAlignment                          ma;
    MuscleTaskSettings&                 settings;
    GAutoDeleteList<MuscleAlignPreset>  presets;
};

class MuscleAlignWithExtFileSpecifyDialogController : public QDialog, public Ui_MuscleAlignmentDialog {
    Q_OBJECT

public:
    MuscleAlignWithExtFileSpecifyDialogController(QWidget* w, MuscleTaskSettings& settings);
    
public slots:
    void accept();

private slots:
    void sl_onPresetChanged(int current);
    void sl_inputFileLineEditChanged(const QString& str);
private:
    void initPresets();
    MuscleTaskSettings&                 settings;
    GAutoDeleteList<MuscleAlignPreset>  presets;
    FileLineEdit*                       inputFileLineEdit;
};


}//namespace
#endif
