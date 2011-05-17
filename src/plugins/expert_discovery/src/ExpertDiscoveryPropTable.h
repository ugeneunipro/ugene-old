#pragma once

#include "ExpertDiscoveryTreeWidgets.h"

#include <QTableWidget>
#include <QComboBox>

namespace U2 {

class EDPropertiesTable : public QTableWidget{
	Q_OBJECT
public:
    EDPropertiesTable(QWidget* parent);

    void representPIProperties(EDProjectItem* pItem);

    void clearAll();
	
private:
    void addNewGroup(const QString& name);
    void addNewField(const QString& name);

    void cleanup();

    EDProjectItem* curPItem;
    bool isSeq;
    int seqOffset;

protected slots:
    void sl_cellChanged(QTableWidgetItem * tItem);
    void sl_cellDataChanged(int row, int column);
    void sl_comboEditTextChangerd(const QString& t);

public slots:
    void sl_treeSelChanged(QTreeWidgetItem * tItem);

signals:
    void si_propChanged(EDProjectItem* item, const EDPIProperty* prop, QString newVal);

};

class EDPropertyItem : public QTableWidgetItem{
public:
    EDPropertyItem(const QString& val, int nGroup, int nProp, bool editable);
    //EDPropertyItem(const QString& val, int nGroup, int nProp, const EDPIPropertyTypeList* pType,  bool editable);
    int getGroup(){return nGroup;}
    int getProp(){return nProp;}
    QString getData() {return data;}
    void setData(const QString& d){data = d;}
private:
    int nGroup;
    int nProp;
    QString data;
};

class EDPropertyItemList : public QComboBox{
public:
    //EDPropertyItem(const QString& val, int nGroup, int nProp, bool editable);
    EDPropertyItemList(const QString& val, int nGroup, int nProp, const EDPIPropertyTypeList* pType,  bool editable);
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
