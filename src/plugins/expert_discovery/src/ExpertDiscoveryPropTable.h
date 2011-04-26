#pragma once

#include "ExpertDiscoveryTreeWidgets.h"

#include <QTableWidget>

namespace U2 {

class EDPropertiesTable : public QTableWidget{
	Q_OBJECT
public:
    EDPropertiesTable(QWidget* parent);

    void representPIProperties(EDProjectItem* pItem);
	
private:
    void addNewGroup(const QString& name);
    void addNewField(const QString& name);

    EDProjectItem* curPItem;

protected slots:
    void sl_cellChanged(QTableWidgetItem * tItem);

public slots:
    void sl_treeSelChanged(QTreeWidgetItem * tItem);

signals:
    void si_propChanged(EDProjectItem* item, const EDPIProperty* prop, QString newVal);

};

class EDPropertyItem : public QTableWidgetItem{
public:
    EDPropertyItem(const QString& val, int nGroup, int nProp, bool editable);
    EDPropertyItem(const QString& val, int nGroup, int nProp, const EDPIPropertyTypeList* pType,  bool editable);
    int getGroup(){return nGroup;}
    int getProp(){return nProp;}
    QString getData() {return data;}
    void setData(const QString& d){data = d;}
private:
    int nGroup;
    int nProp;
    QString data;
};

}//namespace
