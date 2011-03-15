/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _U2_WORKFLOW_MAP_EDITOR_H_
#define _U2_WORKFLOW_MAP_EDITOR_H_

#include <U2Lang/Configuration.h>
#include <U2Lang/ConfigurationEditor.h>

class QTableWidget;
class QTextEdit;

namespace U2 {

/**
 * base class for controllers of map datatype attribute editors of configuration
 * this is base class for port data editors
 * 
 * from - type came from integral bus
 * to type of port
 * prop - busmap property name
 */
class U2LANG_EXPORT MapDatatypeEditor : public QObject, public ConfigurationEditor {
    Q_OBJECT
public:
    MapDatatypeEditor(Configuration* cfg, const QString& prop, DataTypePtr from, DataTypePtr to);
    virtual ~MapDatatypeEditor() {}
    virtual QWidget* getWidget();
    virtual void commit();
    int getOptimalHeight();

private slots:
    void sl_showDoc();
signals:
    void si_showDoc(const QString&);
protected:
    bool isInfoMode() const {return from == to;}
    virtual QWidget* createGUI(DataTypePtr from, DataTypePtr to);
protected:
    Configuration* cfg;
    const QString propertyName;
    DataTypePtr from, to;
    QTableWidget* table;
    QTextEdit* doc;
    
}; // MapDatatypeEditor


namespace Workflow { class IntegralBusPort;}

/**
 * IntegralBusPort realization of MapDatatypeEditor
 */
class U2LANG_EXPORT BusPortEditor : public MapDatatypeEditor {
    Q_OBJECT
public:
    BusPortEditor(Workflow::IntegralBusPort* p);
    virtual ~BusPortEditor() {}
protected:
    virtual QWidget* createGUI(DataTypePtr from, DataTypePtr to);
    Workflow::IntegralBusPort* port;
private slots:
    void handleDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight);
    
}; // BusPortEditor


class U2LANG_EXPORT DescriptorListEditorDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    DescriptorListEditorDelegate(QObject *parent = 0) : QItemDelegate(parent) {}
    virtual ~DescriptorListEditorDelegate() {}

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    
    //void updateEditorGeometry(QWidget *editor,
    //  const QStyleOptionViewItem &option, const QModelIndex &index) const;
    
}; // DescriptorListEditorDelegate

}//namespace U2

#endif
