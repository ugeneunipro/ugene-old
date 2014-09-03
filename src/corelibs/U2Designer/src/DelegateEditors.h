/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_WORKFLOW_URL_H_
#define _U2_WORKFLOW_URL_H_

#include <U2Designer/URLLineEdit.h>

#include <U2Lang/ConfigurationEditor.h>

#include <QtCore/QModelIndex>
#include <QtCore/QPointer>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>
#include <QtGui/QDialog>
#include <QtGui/QListWidget>
#include <QtGui/QSpinBox>
#include <QtGui/QComboBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QTextEdit>
#else
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QDialog>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTextEdit>
#endif

#include <QtCore/QCoreApplication>
#include <QtGui/QKeyEvent>

#include "PropertyWidget.h"

namespace U2 {

/**
 * simple realization of configuration editor
 */
class U2DESIGNER_EXPORT DelegateEditor : public ConfigurationEditor {
    Q_OBJECT
public:
    DelegateEditor(const QMap<QString, PropertyDelegate*>& map) : delegates(map) {}
    DelegateEditor(const QString& s, PropertyDelegate* d) {delegates.insert(s,d);}
    DelegateEditor(const DelegateEditor &other);
    virtual ~DelegateEditor() {qDeleteAll(delegates.values());}
    virtual PropertyDelegate* getDelegate(const QString& name) {return delegates.value(name);}
    virtual PropertyDelegate* removeDelegate( const QString & name ) {return delegates.take( name );}
    virtual void updateDelegates();
    virtual void updateDelegate( const QString & name );
    virtual void addDelegate( PropertyDelegate * del, const QString & name ) { delegates.insert( name, del ); }
    virtual void commit() {}
    virtual ConfigurationEditor *clone() {return new DelegateEditor(*this);}

protected:
    QMap<QString, PropertyDelegate*> delegates;

private:
    DelegateEditor& operator = (const DelegateEditor&);
}; // DelegateEditor

class U2DESIGNER_EXPORT URLDelegate : public PropertyDelegate {
    Q_OBJECT
public:
    URLDelegate(const QString& filter, const QString& type, bool multi = false, bool isPath = false, bool saveFile = true, QObject *parent = 0, const QString &format = "", bool noFilesMode = false);

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const;
    virtual PropertyWidget * createWizardWidget(U2OpStatus &os, QWidget *parent) const;

    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const;

    virtual PropertyDelegate *clone() {
        return new URLDelegate(tags()->get("filter").toString(), lastDirType, multi, isPath, saveFile, parent(), tags()->get("format").toString(), noFilesMode);
    }
    virtual Type type() const;

private slots:
    void sl_commit();

public:
    QString lastDirType;
    bool    multi;
    bool    isPath;
    bool    saveFile; // sets when you need only 1 file for reading (is set with multi=false)
    QString text;
    bool noFilesMode;

private:
    URLWidget * createWidget(QWidget *parent) const;
};

class U2DESIGNER_EXPORT SpinBoxDelegate : public PropertyDelegate {
    Q_OBJECT
public:
    SpinBoxDelegate(const QVariantMap& props = QVariantMap(), QObject *parent = 0) : 
      PropertyDelegate(parent), spinProperties(props), currentEditor(NULL) {}
    virtual ~SpinBoxDelegate() {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const;
    virtual PropertyWidget * createWizardWidget(U2OpStatus &os, QWidget *parent) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const;
    QVariant getDisplayValue(const QVariant&) const;

    void setEditorProperty(const char* name, const QVariant& val);

    virtual PropertyDelegate *clone() {
        return new SpinBoxDelegate(spinProperties, parent());
    }

    void getItems( QVariantMap &items ) const;

signals:
    void si_valueChanged(int);
private slots:
    void sl_commit();
private:
    QVariantMap spinProperties;
    mutable QPointer<SpinBoxWidget> currentEditor;
};

class U2DESIGNER_EXPORT DoubleSpinBoxDelegate : public PropertyDelegate {
    Q_OBJECT
public:
    DoubleSpinBoxDelegate(const QVariantMap& props = QVariantMap(), QObject *parent = 0);
    virtual ~DoubleSpinBoxDelegate() {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
          const QModelIndex &index) const;
    virtual PropertyWidget * createWizardWidget(U2OpStatus &os, QWidget *parent) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
          const QModelIndex &index) const;
    QVariant getDisplayValue(const QVariant&) const;

    virtual PropertyDelegate *clone() {
        return new DoubleSpinBoxDelegate(spinProperties, parent());
    }

    void getItems(QVariantMap &items ) const;

    static const int DEFAULT_DECIMALS_VALUE;

private slots:
    void sl_commit();
private:
    QVariantMap spinProperties;
};

class U2DESIGNER_EXPORT ComboBoxDelegate : public PropertyDelegate {
    Q_OBJECT
public:
    ComboBoxDelegate(const QVariantMap& items, QObject *parent = 0) : PropertyDelegate(parent), items(items) {}
    virtual ~ComboBoxDelegate() {}

      QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
          const QModelIndex &index) const;
      virtual PropertyWidget * createWizardWidget(U2OpStatus &os, QWidget *parent) const;

      void setEditorData(QWidget *editor, const QModelIndex &index) const;
      void setModelData(QWidget *editor, QAbstractItemModel *model,
          const QModelIndex &index) const;
      QVariant getDisplayValue(const QVariant&) const;

      virtual PropertyDelegate *clone() {
          return new ComboBoxDelegate(items, parent());
      }

      void getItems( QVariantMap &items ) const;
      QVariantMap getItems() const;

signals:
    void si_valueChanged( const QString & newVal ) const;

private slots:
    void sl_commit();

protected:
    QVariantMap items;
};

class U2DESIGNER_EXPORT ComboBoxWithUrlsDelegate : public PropertyDelegate {
    Q_OBJECT
public:
    ComboBoxWithUrlsDelegate(const QVariantMap& items, bool _isPath = false, QObject *parent = 0) : PropertyDelegate(parent), items(items), isPath(_isPath) {}
    virtual ~ComboBoxWithUrlsDelegate() {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const;
    virtual PropertyWidget * createWizardWidget(U2OpStatus &os, QWidget *parent) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const;
    QVariant getDisplayValue(const QVariant&) const;

    virtual PropertyDelegate *clone() {
        return new ComboBoxWithUrlsDelegate(items, isPath, parent());
    }

signals:
    void si_valueChanged( const QString & newVal ) const;

private slots:
    void sl_valueChanged(const QString &newVal);

protected:
    QVariantMap items;
    bool isPath;
};

class U2DESIGNER_EXPORT ComboBoxWithDbUrlsDelegate : public PropertyDelegate {
    Q_OBJECT
public:
    ComboBoxWithDbUrlsDelegate(QObject *parent = NULL);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual PropertyWidget * createWizardWidget(U2OpStatus &os, QWidget *parent) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    QVariant getDisplayValue(const QVariant&) const;

    virtual PropertyDelegate *clone();

signals:
    void si_valueChanged(const QString &newVal) const;

private slots:
    void sl_valueChanged(const QString &newVal);

private:
    QVariantMap items;
};

class U2DESIGNER_EXPORT ComboBoxWithChecksDelegate: public PropertyDelegate{
    Q_OBJECT
public:
    ComboBoxWithChecksDelegate(const QVariantMap& items, QObject *parent = 0) : PropertyDelegate(parent), items(items){}
    virtual ~ComboBoxWithChecksDelegate() {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const;
    virtual PropertyWidget * createWizardWidget(U2OpStatus &os, QWidget *parent) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const;
    QVariant getDisplayValue(const QVariant&) const;

    virtual PropertyDelegate *clone() {
        return new ComboBoxWithChecksDelegate(items, parent());
    }

    void getItems( QVariantMap &items ) const;

signals:
    void si_valueChanged( const QString & newVal ) const;

protected:
    QVariantMap items;
};

class U2DESIGNER_EXPORT FileModeDelegate : public ComboBoxDelegate {
public:
    FileModeDelegate(bool appendSupported, QObject *parent = 0);
    virtual ~FileModeDelegate() {}

    virtual PropertyDelegate *clone() {
        return new FileModeDelegate(3 == items.size(), parent());
    }
};

class U2DESIGNER_EXPORT SchemaRunModeDelegate : public ComboBoxDelegate {
    Q_OBJECT
public:
    static const QString THIS_COMPUTER_STR;
    static const QString REMOTE_COMPUTER_STR;
    
public:
    SchemaRunModeDelegate(QObject * parent = 0);
    virtual ~SchemaRunModeDelegate() {}

    virtual PropertyDelegate *clone() {
        return new SchemaRunModeDelegate(parent());
    }

public slots:
    void sl_valueChanged( const QString & val );
    
signals:
    void si_showOpenFileButton( bool show );
    
}; // SchemaRunModeDelegate

class ScriptSelectionWidget : public PropertyWidget {
    Q_OBJECT
public:
    ScriptSelectionWidget(QWidget *parent = NULL);
    QVariant value();

public slots:
    void setValue(const QVariant &value);

private slots:
    void sl_comboActivated(int itemId);

signals:
    void si_finished();

private:
    QComboBox *combobox;
};

class U2DESIGNER_EXPORT AttributeScriptDelegate : public PropertyDelegate {
    Q_OBJECT
public:
    AttributeScriptDelegate(QObject *parent = 0);
    virtual ~AttributeScriptDelegate();
    
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    QVariant getDisplayValue(const QVariant&) const;

    static QString createScriptHeader(const AttributeScript &attrScript);

    virtual PropertyDelegate *clone() {
        return new AttributeScriptDelegate(parent());
    }

private slots:
    void sl_commit();
}; // AttributeScriptDelegate

class U2DESIGNER_EXPORT StingListEdit : public QLineEdit {
    Q_OBJECT

public:
    StingListEdit(QWidget *parent) : QLineEdit(parent) {}

protected:
    void focusOutEvent(QFocusEvent *event);

signals:
    void si_finished();

private slots:
    void sl_onExpand();
};

class StingListWidget : public PropertyWidget {
    Q_OBJECT
public:
    StingListWidget(QWidget *parent = NULL);
    virtual QVariant value();
    virtual void setValue(const QVariant &value);
    virtual void setRequired();

signals:
    void finished();

private:
    StingListEdit *edit;
};

class U2DESIGNER_EXPORT StringListDelegate : public PropertyDelegate {
    Q_OBJECT

public:
    StringListDelegate(QObject *parent = 0) : PropertyDelegate(parent) {}
    virtual ~StringListDelegate() {}

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const;
    virtual PropertyWidget * createWizardWidget(U2OpStatus &os, QWidget *parent) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    virtual PropertyDelegate *clone() {
        return new StringListDelegate(parent());
    }

public slots:
    void sl_commit();

private:
    mutable QWidget *currentEditor;
};

class SelectorDialogHandler {
public:
    virtual QDialog *createSelectorDialog(const QString& init) = 0;
    virtual QString getSelectedString(QDialog *dlg) = 0;
};


class U2DESIGNER_EXPORT StringSelectorDelegate: public PropertyDelegate {
    Q_OBJECT
public:
    StringSelectorDelegate(const QString& _initValue, SelectorDialogHandler *_f, QObject *o = NULL): 
        PropertyDelegate(o), initValue(_initValue), f(_f) {}
    virtual ~StringSelectorDelegate() {}

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const ;

    virtual PropertyDelegate *clone() {
        return new StringSelectorDelegate(initValue, f, parent());
    }

private slots:
    void sl_onClick();
    void sl_commit();

private:
    mutable QLineEdit *valueEdit;
    mutable QWidget *currentEditor;
    QString initValue;
    bool multipleSelection;
    SelectorDialogHandler *f;
};

class U2DESIGNER_EXPORT CharacterDelegate : public PropertyDelegate {
    Q_OBJECT
public:
    CharacterDelegate(QObject *parent = 0) : PropertyDelegate(parent) {}
    virtual ~CharacterDelegate() {}
    
    virtual QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual PropertyWidget * createWizardWidget(U2OpStatus &os, QWidget *parent) const;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    virtual PropertyDelegate *clone() {
        return new CharacterDelegate(parent());
    }
    
}; // CharacterDelegate

}//namespace U2
#endif
