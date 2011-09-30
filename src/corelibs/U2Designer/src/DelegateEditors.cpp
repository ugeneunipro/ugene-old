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

#include <U2Core/Log.h>
#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/GUrlUtils.h>

#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowUtils.h>

#include <U2Gui/ScriptEditorDialog.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/DialogUtils.h>

#include "DelegateEditors.h"

namespace U2 {

/********************************
 * SpinBoxDelegate
 ********************************/
QWidget *SpinBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/* index */) const
{
    currentEditor = new QSpinBox(parent);
    connect(currentEditor, SIGNAL(valueChanged(int)), SIGNAL(si_valueChanged(int)));
    WorkflowUtils::setQObjectProperties(*currentEditor, spinProperties);

    return currentEditor;
}

void SpinBoxDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    int value = index.model()->data(index, ConfigurationEditor::ItemValueRole).toInt();
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(value);
}

void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    int value = spinBox->value();
    model->setData(index, value, ConfigurationEditor::ItemValueRole);
}

QVariant SpinBoxDelegate::getDisplayValue( const QVariant& v) const {
    QSpinBox editor;
    WorkflowUtils::setQObjectProperties(editor, spinProperties);
    editor.setValue(v.toInt());
    return editor.text();
}

void SpinBoxDelegate::setEditorProperty(const char* name, const QVariant& val ) {
    spinProperties[name] = val;
    if (!currentEditor.isNull()) {
        currentEditor->setProperty(name, val);
    }
}

/********************************
* DoubleSpinBoxDelegate
********************************/
QWidget *DoubleSpinBoxDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &/* option */,
                                       const QModelIndex &/* index */) const
{
    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    WorkflowUtils::setQObjectProperties(*editor, spinProperties);

    return editor;
}

void DoubleSpinBoxDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    double value = index.model()->data(index, ConfigurationEditor::ItemValueRole).toDouble();
    QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
    spinBox->setValue(value);
}

void DoubleSpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
    spinBox->interpretText();
    double value = spinBox->value();
    model->setData(index, value, ConfigurationEditor::ItemValueRole);
}

QVariant DoubleSpinBoxDelegate::getDisplayValue( const QVariant& v) const {
    QDoubleSpinBox editor;
    WorkflowUtils::setQObjectProperties(editor, spinProperties);
    editor.setValue(v.toDouble());
    return editor.text();
}

/********************************
* ComboBoxDelegate
********************************/
QWidget *ComboBoxDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &/* option */,
                                       const QModelIndex &/* index */) const
{
    QComboBox *editor = new QComboBox(parent);
    //editor->setFrame(false);
    //editor->setSizePolicy(QSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred));

    QMapIterator<QString, QVariant> it(items);
    while (it.hasNext())
    {
        it.next();
        editor->addItem(it.key(), it.value());
    }
    
    connect( editor, SIGNAL( activated( const QString & ) ), this, SIGNAL( si_valueChanged( const QString & ) ) );
    return editor;
}

void ComboBoxDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    QVariant val = index.model()->data(index, ConfigurationEditor::ItemValueRole);
    QComboBox *box = static_cast<QComboBox*>(editor);
    int idx = box->findData(val);
    box->setCurrentIndex(idx);
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QComboBox *box = static_cast<QComboBox*>(editor);
    QVariant val = box->itemData(box->currentIndex());
    model->setData(index, val, ConfigurationEditor::ItemValueRole);
}

QVariant ComboBoxDelegate::getDisplayValue(const QVariant& val) const {
    QString display = items.key(val);
    emit si_valueChanged( display );
    return QVariant( display );
}

/********************************
* URLLineEdit
********************************/
void URLLineEdit::sl_onBrowse() {
    this->browse(false);
}

void URLLineEdit::sl_onBrowseWithAdding() {
    this->browse(true);
}

void URLLineEdit::browse(bool addFiles) {
    LastUsedDirHelper lod(type);
    QString lastDir = lod.dir;
    GUrl currentUrl(text());
    QDir dir(currentUrl.dirPath());
    if(!text().isEmpty() && dir.exists()) {
        lastDir = currentUrl.dirPath();
    }

    QString name;
    if(isPath){
        lod.url = name = QFileDialog::getExistingDirectory(NULL, tr("Select a directory"), lastDir);
    }else if (multi) {
        QStringList lst = QFileDialog::getOpenFileNames(NULL, tr("Select file(s)"), lastDir, FileFilter);
        if (addFiles) {
            name = this->text();
            if (!lst.isEmpty()) {
                name += ";";
            }
        }
        name += lst.join(";");
        if (!lst.isEmpty()) {
            lod.url = lst.first();
        }
    } else {
        if(saveFile) {
            lod.url = name = QFileDialog::getSaveFileName(NULL, tr("Select a file"), lastDir, FileFilter, 0, QFileDialog::DontConfirmOverwrite);
            this->checkExtension(name);
        } else {
            lod.url = name = QFileDialog::getOpenFileName(NULL, tr("Select a file"), lastDir, FileFilter );
        }
    }
    if (!name.isEmpty()) {
        if (name.length() > this->maxLength()) {
            this->setMaxLength(name.length() + this->maxLength());
        }
        setText(name);
    }
    setFocus();
}

void URLLineEdit::focusOutEvent ( QFocusEvent * event) {
    // TODO: fix this low level code. It is made for fixing UGENE-577
    if (Qt::MouseFocusReason == event->reason()) {
        QLayout *layout = this->parentWidget()->layout();
        for (int i=1; i<layout->count(); i++) { //for each QToolButton in the layout
            QWidget *w = layout->itemAt(i)->widget();
            if (w->underMouse()) {
                return;
            }
        }
    }
    emit si_finished();
}

void URLLineEdit::checkExtension(QString &name) {
    DocumentFormat *format = AppContext::getDocumentFormatRegistry()->getFormatById(fileFormat);
    if (NULL != format && !name.isEmpty()) {
        QString newName(name);
        GUrl url(newName);
        QString lastSuffix = url.lastFileSuffix();
        if ("gz" == lastSuffix) {
            int dotPos = newName.length() - lastSuffix.length() - 1;
            if ((dotPos >= 0) && (QChar('.') == newName[dotPos])) {
                newName = url.getURLString().left(dotPos);
                GUrl tmp(newName);
                lastSuffix = tmp.lastFileSuffix(); 
            }
        }
        bool foundExt = false;
        foreach (QString supExt, format->getSupportedDocumentFileExtensions()) {
            if (lastSuffix == supExt) {
                foundExt = true;
                break;
            }
        }
        if (!foundExt) {
            name = name + "." + format->getSupportedDocumentFileExtensions().first();
        } else {
            int dotPos = newName.length() - lastSuffix.length() - 1;
            if ((dotPos < 0) || (QChar('.') != newName[dotPos])) {
                name = name + "." + format->getSupportedDocumentFileExtensions().first();
            }
        }
    }
}

/********************************
* URLDelegate
********************************/
QWidget *URLDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &/* option */,
                                       const QModelIndex &/* index */) const
{
    QWidget * widget = new QWidget(parent);
    URLLineEdit* documentURLEdit = new URLLineEdit(FileFilter, type, multi, isPath, saveFile, widget, fileFormat);
    documentURLEdit->setObjectName("URLLineEdit");
    documentURLEdit->setFrame(false);
    documentURLEdit->setSizePolicy(QSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred));
    widget->setFocusProxy(documentURLEdit);
    QToolButton * toolButton = new QToolButton(widget);
    toolButton->setVisible( showButton );
    toolButton->setText("...");
    toolButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred));
    connect(toolButton, SIGNAL(clicked()), documentURLEdit, SLOT(sl_onBrowse()));
    
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(documentURLEdit);
    layout->addWidget(toolButton);

    if (multi) {
        QToolButton *toolButton = new QToolButton(widget);
        toolButton->setVisible( showButton && !text.isEmpty() );
        toolButton->setText(tr("add"));
        toolButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred));
        connect(toolButton, SIGNAL(clicked()), documentURLEdit, SLOT(sl_onBrowseWithAdding()));
        layout->addWidget(toolButton);
        connect(documentURLEdit, SIGNAL(textChanged(const QString &)), SLOT(sl_textChanged(const QString &)));
    }

    currentEditor = widget;
    connect(documentURLEdit, SIGNAL(si_finished()), SLOT(sl_commit()));
    return widget;
}

void URLDelegate::sl_commit() {
    URLLineEdit *edit = static_cast<URLLineEdit*>(sender());

    if(edit->text() != text) {
        text = edit->text();
        if (currentEditor) {
            emit commitData(currentEditor);
        }
    }
}

void URLDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    QString val = index.model()->data(index, ConfigurationEditor::ItemValueRole).toString();
    QLineEdit* ed = editor->findChild<QLineEdit*>("URLLineEdit");
    assert(ed);
    ed->setText(val);
}

void URLDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QLineEdit* ed = editor->findChild<QLineEdit*>("URLLineEdit");
    assert(ed);
    QString val = ed->text().replace('\\', '/').trimmed();
    model->setData(index, val, ConfigurationEditor::ItemValueRole);
    if (multi) {
        QVariantList vl;
        foreach(QString s, val.split(";")) {
            vl.append(s.trimmed());
        }
        model->setData(index, vl, ConfigurationEditor::ItemListValueRole);
    }
}

void URLDelegate::sl_showEditorButton( bool show ) {
    showButton = show;
}

void URLDelegate::sl_formatChanged(const QString &newFormat) {
    if (newFormat.isEmpty()) {
        return;
    }

    DocumentFormat *format = AppContext::getDocumentFormatRegistry()->getFormatById(newFormat);
    QString fileFilter;
    if (NULL != format) {
        FileFilter = DialogUtils::prepareDocumentsFileFilter(newFormat, true);
    } else {
        FileFilter = newFormat + " files (*." + newFormat + ")";
    }
    fileFormat = newFormat;
}

void URLDelegate::sl_textChanged(const QString &text) {
    if (!multi) {
        return;
    }
    URLLineEdit *documentURLEdit = static_cast<URLLineEdit*>(sender());
    QLayout *layout = documentURLEdit->parentWidget()->layout();

    if (3 == layout->count()) { // QLineEdit and 2xQToolButton
        layout->itemAt(2)->widget()->setVisible(showButton && !text.isEmpty());
    }
}

/********************************
* FileModeDelegate
********************************/
FileModeDelegate::FileModeDelegate(bool appendSupported, QObject *parent) 
: ComboBoxDelegate(QVariantMap(), parent) {
    items.insert(U2::WorkflowUtils::tr("Overwrite"), SaveDoc_Overwrite);
    items.insert(U2::WorkflowUtils::tr("Rename"), SaveDoc_Roll);
    if (appendSupported) {
        items.insert(U2::WorkflowUtils::tr("Append"), SaveDoc_Append);
    }
}

/********************************
 * SchemaRunModeDelegate
 ********************************/
const QString SchemaRunModeDelegate::THIS_COMPUTER_STR      = SchemaRunModeDelegate::tr( "This computer" );
const QString SchemaRunModeDelegate::REMOTE_COMPUTER_STR    = SchemaRunModeDelegate::tr( "Remote computer" );

SchemaRunModeDelegate::SchemaRunModeDelegate( QObject * parent )
: ComboBoxDelegate( QVariantMap(), parent ) {
    items.insert( THIS_COMPUTER_STR, true );
    items.insert( REMOTE_COMPUTER_STR, false );
    
    connect( this, SIGNAL( si_valueChanged( const QString & ) ), this, 
        SLOT( sl_valueChanged( const QString & ) ) );
}

void SchemaRunModeDelegate::sl_valueChanged( const QString & val ) {
    emit si_showOpenFileButton( THIS_COMPUTER_STR == val );
}

/********************************
* AttributeScriptDelegate
********************************/

QString AttributeScriptDelegate::createScriptHeader(const AttributeScript & attrScript) {
    QString header;
    foreach( const Descriptor & desc, attrScript.getScriptVars().keys() ) {
        header += QString("var %1; // %2\n").arg(desc.getId()).arg(desc.getDisplayName());
    }
    return header;
}

const int NO_SCRIPT_ITEM_ID = 0;
const int USER_SCRIPT_ITEM_ID = 1;
const QPair<QString, int> NO_SCRIPT_ITEM_STR("no script", NO_SCRIPT_ITEM_ID);
const QPair<QString, int> USER_SCRIPT_ITEM_STR("user script", USER_SCRIPT_ITEM_ID);

const QString SCRIPT_PROPERTY = "combo_script_property";

AttributeScriptDelegate::AttributeScriptDelegate(QObject *parent) : PropertyDelegate(parent) {
}

AttributeScriptDelegate::~AttributeScriptDelegate() {
}

void AttributeScriptDelegate::sl_comboActivated(int itemId ) {
    QComboBox * editor = qobject_cast<QComboBox*>(sender());
    assert(editor != NULL);
    
    switch(itemId) {
    case NO_SCRIPT_ITEM_ID:
        {
            editor->setItemData( USER_SCRIPT_ITEM_ID, "", ConfigurationEditor::ItemValueRole );
            return;
        }
    case USER_SCRIPT_ITEM_ID:
        {
            QComboBox * combo = qobject_cast<QComboBox*>(sender());
            assert(combo != NULL);
            AttributeScript attrScript = combo->property(SCRIPT_PROPERTY.toAscii().constData()).value<AttributeScript>();
            
            ScriptEditorDialog dlg(editor, createScriptHeader(attrScript));
            dlg.setScriptText(attrScript.getScriptText());
            
            int rc = dlg.exec();
            if(rc != QDialog::Accepted) {
                editor->setItemData( USER_SCRIPT_ITEM_ID, 
                    qVariantFromValue<AttributeScript>(attrScript), ConfigurationEditor::ItemValueRole );
                return;
            }
            attrScript.setScriptText(dlg.getScriptText());
            editor->setItemData( USER_SCRIPT_ITEM_ID, 
                qVariantFromValue<AttributeScript>(attrScript), ConfigurationEditor::ItemValueRole );
            return;
        }
    default:
        assert(false);
    }
}

QWidget * AttributeScriptDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const {
    QComboBox *editor = new QComboBox(parent);
    editor->addItem(NO_SCRIPT_ITEM_STR.first);
    editor->addItem(USER_SCRIPT_ITEM_STR.first);
    connect(editor, SIGNAL(activated(int)), SLOT(sl_comboActivated(int)));
    return editor;
}

void AttributeScriptDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    QComboBox * combo = qobject_cast<QComboBox*>(editor);
    assert(combo != NULL);
    AttributeScript attrScript = index.model()->data(index, ConfigurationEditor::ItemValueRole).value<AttributeScript>();
    if( attrScript.isEmpty() ) {
        combo->setCurrentIndex(NO_SCRIPT_ITEM_STR.second);
    } else {
        combo->setCurrentIndex(USER_SCRIPT_ITEM_STR.second);
    }
    combo->setProperty(SCRIPT_PROPERTY.toAscii().constData(), qVariantFromValue<AttributeScript>(attrScript));
}

void AttributeScriptDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    QComboBox * combo = qobject_cast<QComboBox*>(editor);
    assert(combo != NULL);
    model->setData(index, combo->itemData(USER_SCRIPT_ITEM_ID, ConfigurationEditor::ItemValueRole), ConfigurationEditor::ItemValueRole);
}

QVariant AttributeScriptDelegate::getDisplayValue(const QVariant& val) const{
    AttributeScript attrScript = val.value<AttributeScript>();
    QString ret = attrScript.isEmpty() ? NO_SCRIPT_ITEM_STR.first : USER_SCRIPT_ITEM_STR.first;
    return QVariant(ret);
}

/********************************
 * AttributeScriptDelegate
 ********************************/
//InputPortDataDelegate::InputPortDataDelegate(const QVariantMap& items, QObject *parent ) : ComboBoxDelegate(items, parent) {
//}
//
//InputPortDataDelegate::~InputPortDataDelegate() {
//}

/* class WCOREAPI_EXPORT ListLineEdit : public QLineEdit */

void StingListEdit::sl_onExpand()
{
    QDialog editor(0);
    editor.setWindowTitle(StringListDelegate::tr("Enter items"));

    QPushButton *accept = new QPushButton(StringListDelegate::tr("Ok"), &editor);
    connect(accept, SIGNAL(clicked()), &editor, SLOT(accept()));
    QPushButton *reject = new QPushButton(StringListDelegate::tr("Cancel"), &editor);
    connect(reject, SIGNAL(clicked()), &editor, SLOT(reject()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout(0);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(accept);
    buttonsLayout->addWidget(reject);

    QTextEdit *edit = new QTextEdit("", &editor);

    foreach (const QString &item, text().split(";", QString::SkipEmptyParts)) {
        edit->append(item.trimmed());
    }

    QVBoxLayout *layout = new QVBoxLayout(&editor);
    layout->addWidget(edit);
    layout->addLayout(buttonsLayout);

    editor.setLayout(layout);

    editor.exec();

    if (editor.result() == QDialog::Accepted) {
        QString s = edit->toPlainText();
        s.replace("\n", "; ");
        setText(s);
        emit editingFinished();
    }
}

void StingListEdit::focusOutEvent ( QFocusEvent * ) {
    emit si_finished();
}

/* class ListEditDelegate : public PropertyDelegate */

const QString StringListDelegate::EDITOR("ListLineEdit");

QWidget* StringListDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const {
    QWidget *widget = new QWidget(parent);

    StingListEdit *edit = new StingListEdit(widget);
    edit->setObjectName(EDITOR);
    edit->setFrame(false);
    edit->setSizePolicy(QSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred));
    widget->setFocusProxy(edit);

    QToolButton *button = new QToolButton(widget);
    button->setVisible(showButton);
    button->setText("...");
    button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred));
    connect(button, SIGNAL(clicked()), edit, SLOT(sl_onExpand()));

    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(edit);
    layout->addWidget(button);

    currentEditor = widget;
    connect(edit, SIGNAL(si_finished()), SLOT(sl_commit()));

    return widget;
}

void StringListDelegate::sl_commit() {
    //sender()->disconnect(this);
    emit commitData(currentEditor);
}

void StringListDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString val = index.model()->data(index, ConfigurationEditor::ItemValueRole).toString();

    QLineEdit *ed = editor->findChild<QLineEdit*>(EDITOR);
    assert(ed);

    ed->setText(val);
}

void StringListDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *ed = editor->findChild<QLineEdit*>(EDITOR);
    assert(ed);
    
    QString val = ed->text();
    model->setData(index, val, ConfigurationEditor::ItemValueRole);

    QVariantList vl;
    foreach(const QString &s, val.split(";", QString::SkipEmptyParts)) {
        vl.append(s.trimmed());
    }

    model->setData(index, vl, ConfigurationEditor::ItemListValueRole);
}

void StringListDelegate::sl_showEditorButton(bool show) {
    showButton = show;
}

/********************************
 * StringSelectorDelegate
********************************/

QWidget *StringSelectorDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const {
    QWidget * editor = new QWidget(parent);
    valueEdit = new QLineEdit(editor);
    valueEdit->setObjectName("valueEdit");
    //valueEdit->setReadOnly(true);
    valueEdit->setFrame(false);
    valueEdit->setSizePolicy(QSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred));
    editor->setFocusProxy(valueEdit);
    QToolButton * toolButton = new QToolButton(editor);
    toolButton->setVisible( true );
    toolButton->setText("...");
    toolButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred));
    connect(toolButton, SIGNAL(clicked()), SLOT(sl_onClick()));

    QHBoxLayout* layout = new QHBoxLayout(editor);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(valueEdit);
    layout->addWidget(toolButton);

    currentEditor = editor;
    connect(valueEdit, SIGNAL(editingFinished()), SLOT(sl_commit()));
    
    return editor;
}

void StringSelectorDelegate::sl_commit() {
    //sender()->disconnect(this);
    emit commitData(currentEditor);
}

void StringSelectorDelegate::sl_onClick() {
    QDialog *dlg = f->createSelectorDialog(initValue);
    if(dlg->exec() == QDialog::Accepted) {
        valueEdit->setText(f->getSelectedString(dlg));
        sl_commit();
    }
    delete dlg;
}

void StringSelectorDelegate::setEditorData(QWidget *, const QModelIndex &index) const {
    QString val = index.model()->data(index, ConfigurationEditor::ItemValueRole).toString();
    valueEdit->setText(val);
}

void StringSelectorDelegate::setModelData(QWidget *, QAbstractItemModel *model, const QModelIndex &index) const {
    QString val = valueEdit->text().trimmed();
    model->setData(index, val, ConfigurationEditor::ItemValueRole);
    if (multipleSelection) {
        QVariantList vl;
        foreach(QString s, val.split(",")) {
            vl.append(s.trimmed());
        }
        model->setData(index, vl, ConfigurationEditor::ItemListValueRole);
    }
}

/********************************
 * CharacterDelegate
 ********************************/
QWidget *CharacterDelegate::createEditor(QWidget *parent,
                                   const QStyleOptionViewItem &/* option */,
                                   const QModelIndex &/* index */) const
{
    QLineEdit * lineEdit = new QLineEdit(parent);
    lineEdit->setMaxLength(1);
    return lineEdit;
}

void CharacterDelegate::setEditorData(QWidget *editor,
                                const QModelIndex &index) const
{
    QString val = index.model()->data(index, ConfigurationEditor::ItemValueRole).toString();
    QLineEdit * lineEdit = qobject_cast<QLineEdit*>(editor);
    assert(lineEdit);
    lineEdit->setText(val);
}

void CharacterDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                               const QModelIndex &index) const
{
    QLineEdit * lineEdit = qobject_cast<QLineEdit*>(editor);
    assert(lineEdit);
    QString val = lineEdit->text();
    model->setData(index, val, ConfigurationEditor::ItemValueRole);
}

}//namespace U2
