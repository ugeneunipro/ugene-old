#ifndef _U2_WEIGHT_MATRIX_JASPAR_DIALOG_CONTROLLER_H_
#define _U2_WEIGHT_MATRIX_JASPAR_DIALOG_CONTROLLER_H_

#include <ui/ui_SearchJASPARDatabase.h>

#include <QtGui/QDialog>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>

#include <U2Core/PFMatrix.h>

#include "WeightMatrixPlugin.h"

namespace U2 {

class PWMJASPARDialogController : public QDialog, public Ui_SearchJASPARDatabase {
    Q_OBJECT

public:
    PWMJASPARDialogController(QWidget* w = NULL);
    QString fileName;

private slots:

    void sl_onOK();
    void sl_onCancel();
    void sl_onSelectionChanged();
    void sl_onDoubleClicked(QTreeWidgetItem* item, int col);
    void sl_onTableItemClicked(QTableWidgetItem* item);

};

class JasparTreeItem;
class JasparGroupTreeItem : public QTreeWidgetItem {
public:
    JasparGroupTreeItem(const QString& s);
    QString s;
    bool operator<(const QTreeWidgetItem & other) const;
};

class JasparTreeItem : public QTreeWidgetItem {
public:
    JasparTreeItem(const JasparInfo& ed);
    JasparInfo matrix;
    bool operator<(const QTreeWidgetItem & other) const;
};

} //namespace

#endif