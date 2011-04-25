#ifndef _CREATEPHYTREEDIALOGCONTROLLER_H_
#define _CREATEPHYTREEDIALOGCONTROLLER_H_

#include <U2Core/global.h>
#include <U2Algorithm/CreatePhyTreeSettings.h>

#include <QtGui/QDialog>

class Ui_CreatePhyTree;
class QWidget;

namespace U2{

class MAlignmentObject;
class MAlignment;
class CreatePhyTreeWidget;

class U2VIEW_EXPORT CreatePhyTreeDialogController : public QDialog {
    Q_OBJECT
public:
    CreatePhyTreeDialogController(QWidget* parent, const MAlignmentObject* mobj, CreatePhyTreeSettings& settings);
    ~CreatePhyTreeDialogController();

    void insertContrWidget(int pos, CreatePhyTreeWidget* widget);

private slots:
    void sl_okClicked();
    void sl_browseClicked();
    void sl_onStoreSettings();
    void sl_onRestoreDefault();
    
private:
    int rightMargin;
    const MAlignment& msa;
    CreatePhyTreeSettings& settings;
    QList<CreatePhyTreeWidget*> childWidgets;
    Ui_CreatePhyTree* ui;

    bool estimateResources(); 
    bool checkSeed(int seed);
};

}

#endif