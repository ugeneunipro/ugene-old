/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include <U2Core/U2OpStatus.h>

#include <U2Lang/Attribute.h>

#include <QtGui/QItemDelegate>

class QWidget;

namespace U2 {

class ConfigurationEditor;
class DelegateTags;
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
 * Abstract class for property widgets: spin box, url line...
 */
class U2LANG_EXPORT PropertyWidget : public QWidget {
    Q_OBJECT
public:
    PropertyWidget(QWidget *parent = NULL, DelegateTags *tags = NULL);
    virtual ~PropertyWidget();

    virtual QVariant value() = 0;
    virtual void setRequired();

    /**
     * Returns the widget that can be registered as a field of wizard pages.
     * Returns NULL if there is no such widget.
     */
    virtual QWidget * getField();

    void setDelegateTags(const DelegateTags *value);
    const DelegateTags * tags() const;

public slots:
    virtual void setValue(const QVariant &value) = 0;

signals:
    void si_valueChanged(const QVariant &value);

protected:
    void addMainWidget(QWidget *w);

protected:
    const DelegateTags *_tags;
};

/**
 * provides display and editing facilities for data items
 * in our case, inheritors will provide this facilities for attributes of configuration
 */
class U2LANG_EXPORT PropertyDelegate : public QItemDelegate {
public:
    PropertyDelegate(QObject *parent = 0);
    virtual ~PropertyDelegate();
    virtual QVariant getDisplayValue(const QVariant &v) const;
    virtual PropertyDelegate * clone();
    virtual PropertyWidget * createWizardWidget(U2OpStatus &os, QWidget *parent) const;
    DelegateTags * tags() const;
protected:
    DelegateTags *_tags;
}; // PropertyDelegate

class U2LANG_EXPORT DelegateTags : public QObject {
public:
    DelegateTags(QObject *parent = NULL);
    DelegateTags(const DelegateTags &other);

    QVariant get(const QString &name) const;
    void set(const QString &name, const QVariant &value);
    void set(const DelegateTags &other);

    static QString getString(const DelegateTags *tags, const QString &name);
private:
    QVariantMap tags;
};

}//GB2 namespace

#endif
