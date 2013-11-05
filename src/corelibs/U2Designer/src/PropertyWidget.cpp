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

#include <QLayout>
#include <QFileDialog>

#include <U2Core/L10n.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Lang/SchemaConfig.h>
#include <U2Lang/URLContainer.h>
#include <U2Lang/WorkflowSettings.h>
#include <U2Lang/WorkflowUtils.h>
#include <U2Gui/LastUsedDirHelper.h>

#include "OutputFileDialog.h"

#include "PropertyWidget.h"

namespace U2 {

/************************************************************************/
/* DefaultPropertyWidget */
/************************************************************************/
DefaultPropertyWidget::DefaultPropertyWidget(int maxLength, QWidget *parent)
: PropertyWidget(parent)
{
    lineEdit = new QLineEdit(this);
    if (maxLength >= 0) {
        lineEdit->setMaxLength(maxLength);
    }
    addMainWidget(lineEdit);
    connect(lineEdit, SIGNAL(textChanged(const QString &)), SLOT(sl_valueChanged(const QString &)));
}

QVariant DefaultPropertyWidget::value() {
    return lineEdit->text();
}

void DefaultPropertyWidget::setValue(const QVariant &value) {
    lineEdit->setText(value.toString());
}

void DefaultPropertyWidget::setRequired() {
    lineEdit->setPlaceholderText(L10N::required());
}

void DefaultPropertyWidget::sl_valueChanged(const QString &value) {
    emit si_valueChanged(value);
}

/************************************************************************/
/* SpinBoxWidget */
/************************************************************************/
SpinBoxWidget::SpinBoxWidget(const QVariantMap &spinProperties, QWidget *parent)
: PropertyWidget(parent)
{
    spinBox = new QSpinBox(this);
    WorkflowUtils::setQObjectProperties(*spinBox, spinProperties);
    addMainWidget(spinBox);

    connect(spinBox, SIGNAL(valueChanged(int)), SLOT(sl_valueChanged(int)));
}

QVariant SpinBoxWidget::value() {
    spinBox->interpretText();
    return spinBox->value();
}

void SpinBoxWidget::setValue(const QVariant &value) {
    spinBox->setValue(value.toInt());
}

bool SpinBoxWidget::setProperty(const char *name, const QVariant &value) {
    return spinBox->setProperty(name, value);
}

void SpinBoxWidget::sl_valueChanged(int value) {
    emit valueChanged(value);
    emit si_valueChanged(value);
}

/************************************************************************/
/* DoubleSpinBoxWidget */
/************************************************************************/
DoubleSpinBoxWidget::DoubleSpinBoxWidget(const QVariantMap &spinProperties, QWidget *parent)
: PropertyWidget(parent)
{
    spinBox = new QDoubleSpinBox(this);
    WorkflowUtils::setQObjectProperties(*spinBox, spinProperties);
    addMainWidget(spinBox);

    connect(spinBox, SIGNAL(valueChanged(double)), SLOT(sl_valueChanged(double)));
}

QVariant DoubleSpinBoxWidget::value() {
    spinBox->interpretText();
    return spinBox->value();
}

void DoubleSpinBoxWidget::setValue(const QVariant &value) {
    spinBox->setValue(value.toDouble());
}

void DoubleSpinBoxWidget::sl_valueChanged(double value) {
    emit si_valueChanged(value);
}

/************************************************************************/
/* ComboBoxWidget */
/************************************************************************/
ComboBoxWidget::ComboBoxWidget(const QVariantMap &items, QWidget *parent)
: PropertyWidget(parent)
{
    comboBox = new QComboBox(this);
    addMainWidget(comboBox);

    foreach (const QString &key, items.keys()) {
        comboBox->addItem(key, items[key]);
    }
    connect(comboBox, SIGNAL(activated(const QString &)),
        this, SIGNAL(valueChanged(const QString &)));
    connect(comboBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(sl_valueChanged(int)));
}

QVariant ComboBoxWidget::value() {
    return comboBox->itemData(comboBox->currentIndex());
}

void ComboBoxWidget::setValue(const QVariant &value) {
    int idx = comboBox->findData(value);
    comboBox->setCurrentIndex(idx);
}

void ComboBoxWidget::sl_valueChanged(int) {
    emit valueChanged(value().toString());
    emit si_valueChanged(value());
}

ComboBoxWidget * ComboBoxWidget::createBooleanWidget(QWidget *parent) {
    QVariantMap values;
    values[ComboBoxWidget::tr("False")] = false;
    values[ComboBoxWidget::tr("True")] = true;
    return new ComboBoxWidget(values, parent);
}

/************************************************************************/
/* ComboBoxWithUrlWidget */
/************************************************************************/
ComboBoxWithUrlWidget::ComboBoxWithUrlWidget(const QVariantMap &items, bool _isPath, QWidget *parent)
: PropertyWidget(parent)
, customIdx(-1)
, isPath(_isPath)
{
    comboBox = new QComboBox(this);
    addMainWidget(comboBox);
    setStyleSheet("{background-color: green;}");

    foreach (const QString &key, items.keys()) {
        comboBox->addItem(key, items[key]);
    }

    QToolButton * toolButton = new QToolButton(this);
    toolButton->setText("...");
    toolButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    connect(toolButton, SIGNAL(clicked()), SLOT(sl_browse()));
    layout()->addWidget(toolButton);

#ifdef Q_OS_MAC
    toolButton->setMinimumHeight(18);
    QString style = "QComboBox {"
                    "min-height: 19px;"
                    "max-height: 24px;"
                    "}";
    comboBox->setStyleSheet(style);
#endif

    connect(comboBox, SIGNAL(activated(const QString &)),
        this, SIGNAL(valueChanged(const QString &)));
    connect(comboBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(sl_valueChanged(int)));
}

QVariant ComboBoxWithUrlWidget::value() {
    return comboBox->itemData(comboBox->currentIndex());
}

void ComboBoxWithUrlWidget::setValue(const QVariant &value) {
    int idx = comboBox->findData(value);
    if (idx == -1){
        if (customIdx == -1){
            comboBox->addItem(value.toString(), value);
            customIdx = comboBox->findData(value);
        }else{
            comboBox->setItemText(customIdx, value.toString());
            comboBox->setItemData(customIdx, value);
        }
        comboBox->setCurrentIndex(customIdx);
    }else{
        idx = comboBox->findData(value);
        comboBox->setCurrentIndex(idx);
    }
    sl_valueChanged(0);
}

void ComboBoxWithUrlWidget::sl_valueChanged(int) {
    emit si_valueChanged(value());
}

void ComboBoxWithUrlWidget::sl_browse(){
    LastUsedDirHelper lod("UrlCombo");
    QString lastDir = lod.dir;

    QString name;
    if (isPath){
        lod.dir = name = QFileDialog::getExistingDirectory(NULL, tr("Select a directory"), lastDir);
        if (!name.isEmpty()) {
            setValue(name);
        }
    }else{
        lod.url = name = QFileDialog::getOpenFileName(NULL, tr("Select a file"), lastDir);
        if (!name.isEmpty()) {
            setValue(name);
        }
    }
    
    comboBox->setFocus();
}

/************************************************************************/
/* ComboBoxWithChecksWidget */
/************************************************************************/
ComboBoxWithChecksWidget::ComboBoxWithChecksWidget(const QVariantMap& _items, QWidget *parent)
: PropertyWidget(parent)
, items(_items)
{
    comboBox = new QComboBox(this);
    cm = NULL;
    addMainWidget(comboBox);

    setValue(value());

    connect(comboBox, SIGNAL(activated(const QString &)),
        this, SIGNAL(valueChanged(const QString &)));
    connect(comboBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(sl_valueChanged(int)));
   
}

QVariant ComboBoxWithChecksWidget::value() {
    QStringList sList;
    const QList<QString>& keys = items.keys();
    foreach(const QString& key, keys){
        if(items[key].toBool()){
            sList << key;
        }
    }
    return sList.join(",");
}

void ComboBoxWithChecksWidget::setValue(const QVariant &value) {
    disconnect(cm, SIGNAL(itemChanged( const QModelIndex &)), this,
        SLOT(sl_itemChanged( const QModelIndex &)));
    QStringList curList = value.toString().split(",");
    if(cm == NULL){
        cm = new QStandardItemModel(items.size(), 1, comboBox);
    }else{
        cm->clear();
        delete cm;
        cm = new QStandardItemModel(items.size(), 1, comboBox);
    }
      
    const QList<QString>& keys = items.keys();
    int i = 0;

    QStandardItem* ghostItem = new QStandardItem();
    cm->setItem(i++, ghostItem);

    foreach(const QString& key, keys){
        bool checked = curList.contains(key, Qt::CaseInsensitive);
        items[key] = checked;
        QStandardItem* item = new QStandardItem(key);
        item->setCheckable(true);
        item->setEditable(false);
        item->setSelectable(false);
        item->setCheckState((checked)? Qt::Checked: Qt::Unchecked);
        item->setData(key);
        cm->setItem(i++, item);
    }
    comboBox->setModel(cm);

    QListView* vw = new QListView(comboBox);
    vw->setModel(cm);
    vw->setRowHidden(0, true);

    comboBox->setView(vw);
    ghostItem->setText(ComboBoxWithChecksWidget::value().toString());
    connect(cm, SIGNAL(itemChanged( QStandardItem * )), this,
        SLOT(sl_itemChanged( QStandardItem * )));
}

void ComboBoxWithChecksWidget::sl_valueChanged(int) {
    comboBox->setCurrentIndex(0);
    emit si_valueChanged(value());
}

void ComboBoxWithChecksWidget::sl_itemChanged( QStandardItem * item ){
    QStandardItem* standardItem  = item;
    QString key = standardItem->data().toString();

    if (items.contains(key)){
        Qt::CheckState checkState = standardItem->checkState();
        if(checkState == Qt::Checked){
            items[key] = true;
        } else if(checkState == Qt::Unchecked){
            items[key] = false;
        }
        sl_valueChanged(0);
    }

    comboBox->setItemText(0, value().toString());
}

/************************************************************************/
/* URLWidget */
/************************************************************************/
URLWidget::URLWidget(const QString &type, bool multi, bool isPath, bool saveFile, DelegateTags *tags, QWidget *parent)
: PropertyWidget(parent, tags)
{
    setAutoFillBackground(true);

    urlLine = new URLLineEdit(type, multi, isPath, saveFile, this);
    urlLine->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    connect(urlLine, SIGNAL(si_finished()), SLOT(sl_finished()));
    connect(urlLine, SIGNAL(textChanged(const QString &)), SLOT(sl_textChanged(const QString &)));
    addMainWidget(urlLine);

    browseButton = new QToolButton(this);
    browseButton->setText("...");
    browseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    connect(browseButton, SIGNAL(clicked()), SLOT(sl_browse()));
    layout()->addWidget(browseButton);

    if (urlLine->isMulti()) {
        addButton = new QToolButton(this);
        addButton->setVisible(!urlLine->text().isEmpty());
        addButton->setText(tr("add"));
        addButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        connect(addButton, SIGNAL(clicked()), urlLine, SLOT(sl_onBrowseWithAdding()));
        layout()->addWidget(addButton);
    }
}

void URLWidget::sl_textChanged(const QString &text) {
    if (!urlLine->isMulti()) {
        return;
    }

    addButton->setVisible(!text.isEmpty());
}

QVariant URLWidget::value() {
    return urlLine->text();
}

void URLWidget::setValue(const QVariant &value) {
    if (value.canConvert< QList<Dataset> >()) {
        QStringList urls;
        foreach (const Dataset &set, value.value< QList<Dataset> >()) {
            foreach (URLContainer *c, set.getUrls()) {
                urls << c->getUrl();
            }
        }
        initialValue = urls.join(";");
    } else {
        initialValue = value.toString();
    }
    urlLine->setText(initialValue);
}

void URLWidget::setRequired() {
    urlLine->setPlaceholderText(L10N::required());
}

void URLWidget::activate() {
    if (urlLine->text().isEmpty()) {
        sl_browse();
    }
}

void URLWidget::sl_browse() {
    bool useOutDir = WorkflowSettings::isUseWorkflowOutputDirectory();
    if (!useOutDir || !urlLine->saveFile) {
        urlLine->sl_onBrowse();
        return;
    }

    RunFileSystem *rfs = getRFS();
    if (NULL == rfs) {
        urlLine->sl_onBrowse();
    } else {
        OutputFileDialog d(rfs, urlLine->isPath, urlLine->getCompletionFillerInstance(), this);
        if (d.exec()) {
            urlLine->setText(d.getResult());
        } else if (d.isSaveToFileSystem()) {
            urlLine->sl_onBrowse();
        }
        urlLine->setFocus();
    }
}

void URLWidget::sl_finished() {
    RunFileSystem *rfs = getRFS();
    if (NULL != rfs) {
        QString result = urlLine->text();
        if ((result != initialValue) && RFSUtils::isCorrectUrl(result)) {
            if (rfs->canAdd(result, urlLine->isPath)) {
                U2OpStatusImpl os;
                rfs->addItem(result, urlLine->isPath, os);
            } else {
                urlLine->setText(initialValue);
            }
        }
    }
    urlLine->setText(finalyze(urlLine->text()));
    emit si_valueChanged(urlLine->text());
    emit finished();
}

RunFileSystem * URLWidget::getRFS() {
    CHECK(NULL != schemaConfig, NULL);
    return schemaConfig->getRFS();
}

QString URLWidget::finalyze(const QString &url) {
    return url;
}

/************************************************************************/
/* NoFileURLWidget */
/************************************************************************/
NoFileURLWidget::NoFileURLWidget(const QString &type, bool multi, bool isPath, bool saveFile, DelegateTags *tags, QWidget *parent)
: URLWidget(type, multi, isPath, saveFile, tags, parent)
{

}

static const QString FILE_TAG("file");

QString NoFileURLWidget::finalyze(const QString &url, DelegateTags *tags) {
    QFileInfo info(url);
    if (url.isEmpty() || info.isDir() || info.isRelative() || !info.exists()) {
        if (NULL != tags) {
            tags->set(FILE_TAG, "");
        }
        return url;
    }

    if (NULL != tags) {
        tags->set(FILE_TAG, info.fileName());
    }
    return info.dir().absolutePath();
}

QString NoFileURLWidget::finalyze(const QString &url) {
    return finalyze(url, const_cast<DelegateTags*>(tags()));
}

} // U2
