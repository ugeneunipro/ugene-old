#ifndef _U2_ANNOTATION_SETTINGS_DIALOG_IMPL_H_
#define _U2_ANNOTATION_SETTINGS_DIALOG_IMPL_H_

#include <ui/ui_AnnotationSettingsDialog.h>
#include <U2Core/AnnotationSettings.h>

#include <QtGui/QDialog>
#include <QtGui/QTreeWidgetItem>

namespace U2 {

class AnnotationSettingsDialogImpl : public QDialog, public Ui_AnnotationSettingsDialog {
Q_OBJECT
public:
    AnnotationSettingsDialogImpl(QWidget* p);

private slots:
    void sl_itemClicked(QTreeWidgetItem * item, int column);
    void sl_okClicked();
    void sl_itemDoubleClicked(QTreeWidgetItem* item, int col);

private:
    void loadModel();
    void storeModel();
};


//////////////////////////////////////////////////////////////////////////
class ASTreeItem : public QTreeWidgetItem {
public:
    ASTreeItem(const AnnotationSettings* as);
    
    void fillModel();
    
    void drawColorCell();

    AnnotationSettings as;
};

}//namespace

#endif
