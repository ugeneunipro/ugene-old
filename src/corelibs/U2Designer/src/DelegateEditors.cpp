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

#include <U2Core/L10n.h>
#include <U2Core/Log.h>
#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/GUrlUtils.h>

#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowUtils.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/ScriptEditorDialog.h>

#include "PropertyWidget.h"

#include "DelegateEditors.h"

namespace U2 {

DelegateEditor::DelegateEditor(const DelegateEditor &other) {
    foreach (const QString &id, other.delegates.keys()) {
        delegates[id] = other.delegates[id]->clone();
    }
}

/********************************
 * SpinBoxDelegate
 ********************************/
PropertyWidget * SpinBoxDelegate::createWizardWidget(U2OpStatus & /*os*/, QWidget *parent) const {
    return new SpinBoxWidget(spinProperties, parent);
}

QWidget * SpinBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/* index */) const
{
    SpinBoxWidget *editor = new SpinBoxWidget(spinProperties, parent);
    connect(editor, SIGNAL(valueChanged(int)), SIGNAL(si_valueChanged(int)));

    currentEditor = editor;

    return editor;
}

void SpinBoxDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    int value = index.model()->data(index, ConfigurationEditor::ItemValueRole).toInt();
    SpinBoxWidget *spinBox = static_cast<SpinBoxWidget*>(editor);
    spinBox->setValue(value);
}

void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    SpinBoxWidget *spinBox = static_cast<SpinBoxWidget*>(editor);
    int value = spinBox->value().toInt();
    model->setData(index, value, ConfigurationEditor::ItemValueRole);
}

QVariant SpinBoxDelegate::getDisplayValue( const QVariant& v) const {
    QSpinBox editor;
    WorkflowUtils::setQObjectProperties(editor, spinProperties);
    editor.setValue(v.toInt());
    return editor.text();
}

void SpinBoxDelegate::setEditorProperty(const char* name, const QVariant& val) {
    spinProperties[name] = val;
    if (!currentEditor.isNull()) {
        currentEditor->setProperty(name, val);
    }
}

/********************************
* DoubleSpinBoxDelegate
********************************/
const int DoubleSpinBoxDelegate::DEFAULT_DECIMALS_VALUE = 5;

DoubleSpinBoxDelegate::DoubleSpinBoxDelegate(const QVariantMap &props, QObject *parent)
: PropertyDelegate(parent), spinProperties(props)
{
    if (!spinProperties.contains("decimals")) {
        spinProperties["decimals"] = DEFAULT_DECIMALS_VALUE;
    }
}

PropertyWidget * DoubleSpinBoxDelegate::createWizardWidget(U2OpStatus & /*os*/, QWidget *parent) const {
    return new DoubleSpinBoxWidget(spinProperties, parent);
}

QWidget * DoubleSpinBoxDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &/* option */,
                                       const QModelIndex &/* index */) const
{
    return new DoubleSpinBoxWidget(spinProperties, parent);
}

void DoubleSpinBoxDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    QVariant value = index.model()->data(index, ConfigurationEditor::ItemValueRole);
    DoubleSpinBoxWidget *spinBox = static_cast<DoubleSpinBoxWidget*>(editor);
    spinBox->setValue(value);
}

void DoubleSpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    DoubleSpinBoxWidget *spinBox = static_cast<DoubleSpinBoxWidget*>(editor);
    double value = spinBox->value().toDouble();
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
PropertyWidget * ComboBoxDelegate::createWizardWidget(U2OpStatus & /*os*/, QWidget *parent) const {
    return new ComboBoxWidget(items, parent);
}

QWidget *ComboBoxDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &/* option */,
                                       const QModelIndex &/* index */) const
{
    ComboBoxWidget *editor = new ComboBoxWidget(items, parent);
    connect(editor, SIGNAL(valueChanged(const QString &)),
        SLOT(sl_commit()));
    connect(editor, SIGNAL(valueChanged(const QString &)),
        SIGNAL(si_valueChanged(const QString &)));

    return editor;
}

void ComboBoxDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    QVariant val = index.model()->data(index, ConfigurationEditor::ItemValueRole);
    ComboBoxWidget *box = static_cast<ComboBoxWidget*>(editor);
    box->setValue(val);
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    ComboBoxWidget *box = static_cast<ComboBoxWidget*>(editor);
    model->setData(index, box->value(), ConfigurationEditor::ItemValueRole);
}

QVariant ComboBoxDelegate::getDisplayValue(const QVariant& val) const {
    QString display = items.key(val);
    emit si_valueChanged( display );
    return QVariant( display );
}

void ComboBoxDelegate::sl_commit() {
    ComboBoxWidget *editor = static_cast<ComboBoxWidget*>(sender());

    if (editor) {
        emit commitData(editor);
    }
}

/********************************
* ComboBoxWithUrlsDelegate
********************************/

PropertyWidget * ComboBoxWithUrlsDelegate::createWizardWidget(U2OpStatus & /*os*/, QWidget *parent) const {
    return new ComboBoxWithUrlWidget(items, isPath, parent);
}

QWidget *ComboBoxWithUrlsDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &/* option */,
                                        const QModelIndex &/* index */) const
{
    ComboBoxWithUrlWidget *editor = new ComboBoxWithUrlWidget(items, isPath, parent);
    connect(editor, SIGNAL(valueChanged(const QString &)),
        this, SIGNAL(si_valueChanged(const QString &)));

    return editor;
}

void ComboBoxWithUrlsDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const
{
    QVariant val = index.model()->data(index, ConfigurationEditor::ItemValueRole);
    ComboBoxWithUrlWidget *box = static_cast<ComboBoxWithUrlWidget*>(editor);
    box->setValue(val);
}

void ComboBoxWithUrlsDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    ComboBoxWithUrlWidget *box = static_cast<ComboBoxWithUrlWidget*>(editor);
    model->setData(index, box->value(), ConfigurationEditor::ItemValueRole);
}


QVariant ComboBoxWithUrlsDelegate::getDisplayValue(const QVariant& val) const {
     QString display = items.key(val);
     emit si_valueChanged( display );
     return QVariant( display );
 }

/********************************
* ComboBoxWithChecksDelegate
********************************/

PropertyWidget * ComboBoxWithChecksDelegate::createWizardWidget(U2OpStatus & /*os*/, QWidget *parent) const {
    return new ComboBoxWithChecksWidget(items, parent);
}

QWidget *ComboBoxWithChecksDelegate::createEditor(QWidget *parent,
                                                const QStyleOptionViewItem &/* option */,
                                                const QModelIndex &/* index */) const
{
    ComboBoxWithChecksWidget* editor = new ComboBoxWithChecksWidget(items, parent);
    connect(editor, SIGNAL(valueChanged(const QString &)),
        this, SIGNAL(si_valueChanged(const QString &)));
    return editor;
}

void ComboBoxWithChecksDelegate::setEditorData(QWidget *editor,
                                             const QModelIndex &index) const
{
    QVariant val = index.model()->data(index, ConfigurationEditor::ItemValueRole);
    ComboBoxWithChecksWidget *box = static_cast<ComboBoxWithChecksWidget*>(editor);
    box->setValue(val);
}

void ComboBoxWithChecksDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                            const QModelIndex &index) const
{
    ComboBoxWithChecksWidget *box = static_cast<ComboBoxWithChecksWidget*>(editor);
    model->setData(index, box->value(), ConfigurationEditor::ItemValueRole);
}


QVariant ComboBoxWithChecksDelegate::getDisplayValue(const QVariant& val) const {
    QString display = val.toString();
    emit si_valueChanged( display );
    return QVariant( display );
}


/********************************
* URLDelegate
********************************/
URLDelegate::URLDelegate(const QString& filter, const QString& type, bool multi, bool isPath, bool saveFile, QObject *parent, const QString &format)
: PropertyDelegate(parent), lastDirType(type), multi(multi), isPath(isPath), saveFile(saveFile)
{
    tags()->set("filter", filter);
    tags()->set("format", format);
}

URLWidget * URLDelegate::createWidget(QWidget *parent) const {
    URLWidget *result = new URLWidget(lastDirType, multi, isPath, saveFile, tags(), parent);
    if (saveFile) {
        result->setSchemaConfig(schemaConfig);
    }
    return result;
}

PropertyWidget * URLDelegate::createWizardWidget(U2OpStatus & /*os*/, QWidget *parent) const {
    return createWidget(parent);
}

QWidget * URLDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &/* option */,
                                       const QModelIndex &/* index */) const
{
    URLWidget *editor = createWidget(parent);
    connect(editor, SIGNAL(finished()), SLOT(sl_commit()));
    return editor;
}

void URLDelegate::sl_commit() {
    URLWidget *editor = static_cast<URLWidget*>(sender());

    if(editor->value().toString() != text) {
        text = editor->value().toString();
        emit commitData(editor);
    }
}

void URLDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    QVariant val = index.model()->data(index, ConfigurationEditor::ItemValueRole);
    URLWidget *lineEdit = dynamic_cast<URLWidget*>(editor);
    lineEdit->setValue(val);
}

void URLDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    URLWidget *lineEdit = dynamic_cast<URLWidget*>(editor);
    QString val = lineEdit->value().toString().replace('\\', '/').trimmed();
    QStringList urls = val.split(";", QString::SkipEmptyParts);
    val = urls.join(";");
    model->setData(index, val, ConfigurationEditor::ItemValueRole);
    if (multi) {
        QVariantList vl;
        foreach(QString s, val.split(";")) {
            vl.append(s.trimmed());
        }
        model->setData(index, vl, ConfigurationEditor::ItemListValueRole);
    }
}

PropertyDelegate::Type URLDelegate::type() const {
    if (saveFile) {
        if (isPath) {
            return OUTPUT_DIR;
        }
        return OUTPUT_FILE;
    }
    return NO_TYPE;
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
            AttributeScript attrScript = combo->property(SCRIPT_PROPERTY.toLatin1().constData()).value<AttributeScript>();
            
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
    combo->setProperty(SCRIPT_PROPERTY.toLatin1().constData(), qVariantFromValue<AttributeScript>(attrScript));
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
 * StringListDelegate
 ********************************/
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

StingListWidget::StingListWidget(QWidget *parent)
: PropertyWidget(parent)
{
    edit = new StingListEdit(this);
    edit->setFrame(false);
    edit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    addMainWidget(edit);

    QToolButton *button = new QToolButton(this);
    button->setText("...");
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    connect(button, SIGNAL(clicked()), edit, SLOT(sl_onExpand()));
    layout()->addWidget(button);

    connect(edit, SIGNAL(si_finished()), SIGNAL(finished()));
}

QVariant StingListWidget::value() {
    return edit->text();
}

void StingListWidget::setValue(const QVariant &value) {
    edit->setText(value.toString());
}

void StingListWidget::setRequired() {
    edit->setPlaceholderText(L10N::required());
}

PropertyWidget * StringListDelegate::createWizardWidget(U2OpStatus & /*os*/, QWidget *parent) const {
    return new StingListWidget(parent);
}

QWidget * StringListDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const {
    StingListWidget *widget = new StingListWidget(parent);
    connect(widget, SIGNAL(finished()), SLOT(sl_commit()));

    currentEditor = widget;
    return widget;
}

void StringListDelegate::sl_commit() {
    emit commitData(currentEditor);
}

void StringListDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QVariant val = index.model()->data(index, ConfigurationEditor::ItemValueRole);
    StingListWidget *lineEdit = dynamic_cast<StingListWidget*>(editor);
    lineEdit->setValue(val);
}

void StringListDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    StingListWidget *lineEdit = dynamic_cast<StingListWidget*>(editor);
    
    QString val = lineEdit->value().toString();
    model->setData(index, val, ConfigurationEditor::ItemValueRole);

    QVariantList vl;
    foreach(const QString &s, val.split(";", QString::SkipEmptyParts)) {
        vl.append(s.trimmed());
    }

    model->setData(index, vl, ConfigurationEditor::ItemListValueRole);
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
PropertyWidget * CharacterDelegate::createWizardWidget(U2OpStatus & /*os*/, QWidget *parent) const {
    return new DefaultPropertyWidget(1, parent);
}

QWidget * CharacterDelegate::createEditor(QWidget *parent,
                                   const QStyleOptionViewItem &/* option */,
                                   const QModelIndex &/* index */) const
{
    return new DefaultPropertyWidget(1, parent);
}

void CharacterDelegate::setEditorData(QWidget *editor,
                                const QModelIndex &index) const
{
    QVariant val = index.model()->data(index, ConfigurationEditor::ItemValueRole);
    DefaultPropertyWidget *lineEdit = dynamic_cast<DefaultPropertyWidget*>(editor);
    lineEdit->setValue(val);
}

void CharacterDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                               const QModelIndex &index) const
{
    DefaultPropertyWidget *lineEdit = dynamic_cast<DefaultPropertyWidget*>(editor);
    model->setData(index, lineEdit->value().toString(), ConfigurationEditor::ItemValueRole);
}

}//namespace U2
