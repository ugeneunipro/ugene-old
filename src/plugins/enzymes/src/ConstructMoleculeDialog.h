#ifndef _U2_CONSTRUCT_MOLECULE_DIALOG_H_
#define _U2_CONSTRUCT_MOLECULE_DIALOG_H_

#include <QtCore/QList>
#include <ui/ui_ConstructMoleculeDialog.h>

#include "CloningUtilTasks.h"

namespace U2 {

class ConstructMoleculeDialog : public QDialog, public Ui_ConstructMoleculeDialog {
    Q_OBJECT
public:
    ConstructMoleculeDialog(const QList<DNAFragment>& fragments, QWidget* parent);
    virtual void accept();
private slots:
    void sl_onBrowseButtonClicked();
    void sl_onTakeButtonClicked();
    void sl_onTakeAllButtonClicked();
    void sl_onAddFromProjectButtonClicked();
    void sl_onClearButtonClicked();
    void sl_onUpButtonClicked();
    void sl_onDownButtonClicked();
    void sl_onRemoveButtonClicked();
    void sl_makeCircularBoxClicked();
    void sl_forceBluntBoxClicked();
    void sl_onEditFragmentButtonClicked();
    void sl_onItemClicked( QTreeWidgetItem * item, int column );

protected:
    bool eventFilter(QObject* obj , QEvent* event);

private:
    void update();
    QList<DNAFragment> fragments;
    QList<int> selected;
};


} //namespace

#endif // _U2_CONSTRUCT_MOLECULE_DIALOG_H_
