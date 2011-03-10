#ifndef _U2_PHYLTREE_BRANCH_SETTINGS_DIALOG_H_
#define _U2_PHYLTREE_BRANCH_SETTINGS_DIALOG_H_

#include <ui/ui_BranchSettingsDialog.h>

#include <util_ov_phyltree/GraphicsBranchItem.h>
#include <QtGui/QDialog>

namespace U2 {

class BranchSettingsDialog : public QDialog, public Ui_BranchSettingsDialog{
    Q_OBJECT
public:
    BranchSettingsDialog(QWidget *parent, const BranchSettings& branchSettings);

    virtual void accept();
    BranchSettings getSettings() const;

protected slots:
    void sl_colorButton();

private:
    BranchSettings settings, changedSettings;

    void updateColorButton();
};

} //namespace

#endif
