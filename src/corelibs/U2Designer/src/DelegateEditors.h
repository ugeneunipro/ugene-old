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

#ifndef _U2_WORKFLOW_URL_H_
#define _U2_WORKFLOW_URL_H_

#include <U2Lang/ConfigurationEditor.h>

#include <QtCore/QModelIndex>
#include <QtCore/QPointer>
#include <QtGui/QLineEdit>
#include <QtGui/QToolButton>
#include <QtGui/QDialog>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>

#include <QtGui/QSpinBox>
#include <QtGui/QComboBox>
#include <QtGui/QLineEdit>
#include <QtGui/QToolButton>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QFileDialog>
#include <QtCore/QCoreApplication>
#include <QtGui/QKeyEvent>
#include <QtGui/QTextEdit>

namespace U2 {

/**
 * simple realization of configuration editor
 */
class U2DESIGNER_EXPORT DelegateEditor : public ConfigurationEditor {
public:
    DelegateEditor(const QMap<QString, PropertyDelegate*>& map) : delegates(map) {}
    DelegateEditor(const QString& s, PropertyDelegate* d) {delegates.insert(s,d);}
    virtual ~DelegateEditor() {qDeleteAll(delegates.values());}
    virtual PropertyDelegate* getDelegate(const QString& name) {return delegates.value(name);}
    virtual PropertyDelegate* removeDelegate( const QString & name ) {return delegates.take( name );}
    virtual void addDelegate( PropertyDelegate * del, const QString & name ) { delegates.insert( name, del ); }
    virtual void commit() {}
protected:
    QMap<QString, PropertyDelegate*> delegates;
    
}; // DelegateEditor

class U2DESIGNER_EXPORT URLLineEdit : public QLineEdit {
    Q_OBJECT
public:
    URLLineEdit(const QString& filter, const QString& type, bool multi, bool isPath, bool saveFile, QWidget *parent)
        : QLineEdit(parent), FileFilter(filter), type(type), multi(multi), isPath(isPath), saveFile(saveFile) {}
protected:
    void focusOutEvent ( QFocusEvent * event );
private slots:
    void sl_onBrowse();

signals:
    void si_finished();
    
private:
    QString FileFilter;
    QString type;
    bool    multi;
    bool    isPath;
    bool    saveFile;
};

class U2DESIGNER_EXPORT URLDelegate : public PropertyDelegate {
    Q_OBJECT
public:
    URLDelegate(const QString& filter, const QString& type, bool multi = false, bool isPath = false, bool saveFile = true, QObject *parent = 0)
        : PropertyDelegate(parent), FileFilter(filter), type(type), multi(multi), isPath(isPath), showButton( true ), saveFile(saveFile),
        currentEditor(NULL){
    }
    virtual ~URLDelegate() {}

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const;

    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const;

public slots:
    void sl_showEditorButton( bool show );
    void sl_commit();
    void sl_extensionChanged(const QString &newExtension);
    
protected:
    QString FileFilter;
    QString type;
    bool    multi;
    bool    isPath;
    bool    showButton;
    bool    saveFile; // sets when you need only 1 file for reading (is set with multi=false)
    mutable QWidget* currentEditor;
    QString text;
};

class U2DESIGNER_EXPORT SpinBoxDelegate : public PropertyDelegate {
    Q_OBJECT
public:
    SpinBoxDelegate(const QVariantMap& props = QVariantMap(), QObject *parent = 0) : 
      PropertyDelegate(parent), spinProperties(props), currentEditor(NULL) {}
    virtual ~SpinBoxDelegate() {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const;
    QVariant getDisplayValue(const QVariant&) const;

    void setEditorProperty(const char* name, const QVariant& val);

signals:
    void si_valueChanged(int);
private:
    QVariantMap spinProperties;
    mutable QPointer<QWidget> currentEditor;
};

class U2DESIGNER_EXPORT DoubleSpinBoxDelegate : public PropertyDelegate {
public:
    DoubleSpinBoxDelegate(const QVariantMap& props = QVariantMap(), QObject *parent = 0) : 
      PropertyDelegate(parent), spinProperties(props) {}
    virtual ~DoubleSpinBoxDelegate() {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
          const QModelIndex &index) const;
    QVariant getDisplayValue(const QVariant&) const;

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

      void setEditorData(QWidget *editor, const QModelIndex &index) const;
      void setModelData(QWidget *editor, QAbstractItemModel *model,
          const QModelIndex &index) const;
      QVariant getDisplayValue(const QVariant&) const;

signals:
    void si_valueChanged( const QString & newVal ) const;
    
protected:
    QVariantMap items;
};

class U2DESIGNER_EXPORT FileModeDelegate : public ComboBoxDelegate {
public:
    FileModeDelegate(bool appendSupported, QObject *parent = 0);
    virtual ~FileModeDelegate() {}
};

class U2DESIGNER_EXPORT SchemaRunModeDelegate : public ComboBoxDelegate {
    Q_OBJECT
public:
    static const QString THIS_COMPUTER_STR;
    static const QString REMOTE_COMPUTER_STR;
    
public:
    SchemaRunModeDelegate(QObject * parent = 0);
    virtual ~SchemaRunModeDelegate() {}

public slots:
    void sl_valueChanged( const QString & val );
    
signals:
    void si_showOpenFileButton( bool show );
    
}; // SchemaRunModeDelegate

class U2DESIGNER_EXPORT AttributeScriptDelegate : public PropertyDelegate {
    Q_OBJECT
public:
    AttributeScriptDelegate(QObject *parent = 0);
    virtual ~AttributeScriptDelegate();
    
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    QVariant getDisplayValue(const QVariant&) const;

    static QString createScriptHeader(const AttributeScript & attrScript);
private slots:
    void sl_comboActivated(int);
    
}; // AttributeScriptDelegate

//class U2DESIGNER_EXPORT InputPortDataDelegate : public ComboBoxDelegate {
//    Q_OBJECT
//public:
//    InputPortDataDelegate(const QVariantMap& items, QObject *parent = NULL );
//    virtual ~InputPortDataDelegate();
//    
//    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
//    void setEditorData(QWidget *editor, const QModelIndex &index) const;
//    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
//    QVariant getDisplayValue(const QVariant&) const;
//    
//}; // InputPortDataDelegatedes

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

class U2DESIGNER_EXPORT StringListDelegate : public PropertyDelegate {
    Q_OBJECT

public:
    StringListDelegate(QObject *parent = 0) : PropertyDelegate(parent), showButton(true) {}
    virtual ~StringListDelegate() {}

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const;


    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

public slots:
    void sl_showEditorButton(bool show);
    void sl_commit();

private:
    static const QString EDITOR;
    bool showButton;
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
    
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    
}; // CharacterDelegate

}//namespace U2
#endif
