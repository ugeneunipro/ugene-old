#ifndef _U2_PROJECT_TREE_ITEM_SELECTOR_DIALOG_IMPL_H_
#define _U2_PROJECT_TREE_ITEM_SELECTOR_DIALOG_IMPL_H_

#include <ui/ui_ProjectTreeItemSelectorDialogBase.h>
#include <QtGui/QDialog>

namespace U2 {

class ProjectTreeController;
class ProjectTreeControllerModeSettings;
class Document;
class GObject;

class ProjectTreeItemSelectorDialogImpl: public QDialog, public Ui::ProjectTreeItemSelectorDialogBase {
    Q_OBJECT
public:

    bool acceptByDoubleClick;

private slots:
    void sl_objectClicked(GObject* obj);

private:
    friend class ProjectTreeItemSelectorDialog;
    ProjectTreeItemSelectorDialogImpl(QWidget* p, const ProjectTreeControllerModeSettings& s);
    ~ProjectTreeItemSelectorDialogImpl();
    ProjectTreeController* controller;
};

}//namespace
#endif
