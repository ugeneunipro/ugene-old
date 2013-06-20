#ifndef _CREATEPHYTREEDIALOGCONTROLLER_H_
#define _CREATEPHYTREEDIALOGCONTROLLER_H_

#include <U2Algorithm/CreatePhyTreeSettings.h>

#include <U2Core/global.h>
#include <U2Core/MAlignment.h>

#include <QtGui/QDialog>


class Ui_CreatePhyTree;
class QWidget;

namespace U2{

class MAlignmentObject;
class CreatePhyTreeWidget;

class U2VIEW_EXPORT CreatePhyTreeDialogController : public QDialog {
    Q_OBJECT
public:
    CreatePhyTreeDialogController(QWidget* parent, const MAlignmentObject* mobj, CreatePhyTreeSettings& settings);
    ~CreatePhyTreeDialogController();

    void insertContrWidget(int pos, CreatePhyTreeWidget* widget);
    void clearContrWidgets();

private slots:
    void sl_okClicked();
    void sl_browseClicked();
    void sl_comboIndexChaged(int index);
    void sl_onStoreSettings();
    void sl_onRestoreDefault();
    void sl_onDispayWithMSAClicked(bool checked);

private:
    int rightMargin;
    MAlignment msa;
    CreatePhyTreeSettings& settings;
    QList<CreatePhyTreeWidget*> childWidgets;
    Ui_CreatePhyTree* ui;
};

}

#endif