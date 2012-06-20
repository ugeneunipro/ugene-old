/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_WORKFLOW_CFG_EDITOR_H_
#define _U2_WORKFLOW_CFG_EDITOR_H_

#include <U2Lang/Attribute.h>

#include <QtGui/QItemDelegate>

class QWidget;

namespace U2 {

class ConfigurationEditor;
class PropertyDelegate;

/**
 * base class for controller of configuration editor
 * editing comes from delegates (see PropertyDelegate)
 */
class U2LANG_EXPORT ConfigurationEditor : public QObject {
    Q_OBJECT
public:
    enum ItemValueRole {
        ItemValueRole = Qt::UserRole + 2,
        ItemListValueRole
    }; // ItemValueRole

public:    
    ConfigurationEditor() {}
    ConfigurationEditor(const ConfigurationEditor&) : QObject() {}
    virtual ~ConfigurationEditor() {}

    // editing widget
    virtual QWidget* getWidget() { return NULL; }
    
    // this controller is a container of delegates
    virtual PropertyDelegate* getDelegate(const QString&) { return NULL; }
    virtual PropertyDelegate* removeDelegate( const QString&) { return NULL; }
    virtual void addDelegate( PropertyDelegate * , const QString & ) {}
    
    // commit data to model
    virtual void commit() {}

    // make another editor
    virtual ConfigurationEditor *clone() {return new ConfigurationEditor(*this);}

    virtual bool isEmpty() const {return false;}

signals:
    void si_configurationChanged();

}; // ConfigurationEditor

/**
 * provides display and editing facilities for data items
 * in our case, inheritors will provide this facilities for attributes of configuration
 */
class U2LANG_EXPORT PropertyDelegate : public QItemDelegate {
public:
    PropertyDelegate(QObject * parent = 0) : QItemDelegate(parent) {}
    virtual ~PropertyDelegate() {}
    virtual QVariant getDisplayValue(const QVariant& v) const {return v;}
    virtual PropertyDelegate *clone() {return new PropertyDelegate(parent());}
    
}; // PropertyDelegate

}//GB2 namespace

#endif
