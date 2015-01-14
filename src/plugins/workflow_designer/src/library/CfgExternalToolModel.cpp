/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "../WorkflowEditorDelegates.h"
#include "CfgExternalToolModel.h"

namespace U2 {

//////////////////////////////////////////////////////////////////////////
/// CfgExternalToolModel
//////////////////////////////////////////////////////////////////////////

CfgExternalToolItem::CfgExternalToolItem()  {
    dfr = AppContext::getDocumentFormatRegistry();
    dtr = Workflow::WorkflowEnv::getDataTypeRegistry();

    delegateForTypes = NULL;
    delegateForFormats = NULL;
    itemData.type = BaseTypes::DNA_SEQUENCE_TYPE()->getId();
    itemData.format = BaseDocumentFormats::FASTA;
}

CfgExternalToolItem::~CfgExternalToolItem() {
    delete delegateForTypes;
    delete delegateForFormats;
}

QString CfgExternalToolItem::getDataType() const {
    return itemData.type;
}

void CfgExternalToolItem::setDataType(const QString& id) {
    itemData.type = id;
}

QString CfgExternalToolItem::getName() const {
    return itemData.attrName;
}

void CfgExternalToolItem::setName(const QString &_name) {
    itemData.attrName = _name;
}

QString CfgExternalToolItem::getFormat() const {
    return itemData.format;
}

void CfgExternalToolItem::setFormat(const QString & f) {
    itemData.format = f;
}

QString CfgExternalToolItem::getDescription() const {
    return itemData.description;
}

void CfgExternalToolItem::setDescription(const QString & _descr) {
    itemData.description = _descr;
}

//////////////////////////////////////////////////////////////////////////
/// CfgExternalToolModel
//////////////////////////////////////////////////////////////////////////

CfgExternalToolModel::CfgExternalToolModel(bool isInput, QObject *obj)
    : QAbstractTableModel(obj), isInput(isInput)
{
    init();
}

int CfgExternalToolModel::rowCount(const QModelIndex & /* = QModelIndex */) const{
    return items.size();
}

int CfgExternalToolModel::columnCount(const QModelIndex & /* = QModelIndex */) const {
    return 4;
}

Qt::ItemFlags CfgExternalToolModel::flags(const QModelIndex &) const{
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

CfgExternalToolItem* CfgExternalToolModel::getItem(const QModelIndex &index) const {
    return items.at(index.row());
}

QList<CfgExternalToolItem*> CfgExternalToolModel::getItems() const {
    return items;
}

QVariant CfgExternalToolModel::data(const QModelIndex &index, int role /* = Qt::DisplayRole */) const {
    CfgExternalToolItem *item = getItem(index);
    int col = index.column();

    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
        if (col == 0) return item->getName();
        else if (col == 1) return item->delegateForTypes->getDisplayValue(item->getDataType());
        else if (col == 2) return item->delegateForFormats->getDisplayValue(item->getFormat());
        else if (col == 3) return item->getDescription();
        else return QVariant();
    case DelegateRole:
        if (col == 1) return qVariantFromValue<PropertyDelegate*>(item->delegateForTypes);
        else if (col == 2) return qVariantFromValue<PropertyDelegate*>(item->delegateForFormats);
        else return QVariant();
    case Qt::EditRole:
    case ConfigurationEditor::ItemValueRole:
        if (col == 1) return item->getDataType();
        else if (col == 2) return item->getFormat();
        else return QVariant();
    default:
        return QVariant();
    }
}

void CfgExternalToolModel::createFormatDelegate(const QString &newType, CfgExternalToolItem *item) {
    PropertyDelegate *delegate;
    QString format;
    if (newType == BaseTypes::DNA_SEQUENCE_TYPE()->getId()) {
        delegate = new ComboBoxDelegate(seqFormatsW);
        format = seqFormatsW.values().first().toString();
    }
    else if (newType == BaseTypes::MULTIPLE_ALIGNMENT_TYPE()->getId()) {
        delegate = new ComboBoxDelegate(msaFormatsW);
        format = msaFormatsW.values().first().toString();
    }
    else if (newType == BaseTypes::ANNOTATION_TABLE_TYPE()->getId()) {
        delegate = new ComboBoxDelegate(annFormatsW);
        format = annFormatsW.values().first().toString();
    }
    else if (newType == SEQ_WITH_ANNS){
        delegate = new ComboBoxDelegate(annFormatsW);
        format = annFormatsW.values().first().toString();
    }
    else if (newType == BaseTypes::STRING_TYPE()->getId()) {
        delegate = new ComboBoxDelegate(textFormat);
        format = textFormat.values().first().toString();
    }
    else{
        return;
    }
    item->setFormat(format);
    item->delegateForFormats = delegate;
}

bool CfgExternalToolModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    int col = index.column();
    CfgExternalToolItem * item = getItem(index);
    switch (role) {
    case Qt::EditRole:
    case ConfigurationEditor::ItemValueRole:
        if (col == 0) {
            if (item->getName() != value.toString()) {
                item->setName(value.toString());
            }
        }
        else if (col == 1) {
            QString newType = value.toString();
            if (item->getDataType() != newType) {
                if (!newType.isEmpty()) {
                    item->setDataType(newType);
                    createFormatDelegate(newType, item);
                }
            }
        }
        else if (col == 2) {
            if (item->getFormat() != value.toString() && !value.toString().isEmpty())  {
                item->setFormat(value.toString());
            }
        }
        else if (col == 3) {
            if (item->getDescription() != value.toString()) {
                item->setDescription(value.toString());
            }
        }
        emit dataChanged(index, index);
    }
    return true;
}

QVariant CfgExternalToolModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0: return tr("Name for command line parameter");
        case 1: return tr("Type");
        case 2:
            if (isInput) {
                return tr("Read as");
            }
            else {
                return tr("Write as");
            }
        case 3: return tr("Description");
        default: return QVariant();
        }
    }
    return QVariant();
}

bool CfgExternalToolModel::insertRows(int row, int count, const QModelIndex & parent) {
    Q_UNUSED(row);
    Q_UNUSED(count);
    beginInsertRows(parent, items.size(), items.size());
    CfgExternalToolItem *newItem = new CfgExternalToolItem();
    newItem->delegateForTypes = new ComboBoxDelegate(types);
    newItem->delegateForFormats = new ComboBoxDelegate(seqFormatsW);
    items.append(newItem);
    endInsertRows();
    return true;
}

bool CfgExternalToolModel::removeRows(int row, int count, const QModelIndex & parent) {
    Q_UNUSED(count);
    if (row >= 0 && row < items.size()) {
        beginRemoveRows(parent, row, row);
        items.removeAt(row);
        endRemoveRows();
        return true;
    }
    else {
        return false;
    }
}

void CfgExternalToolModel::init() {
    initTypes();
    initFormats();
}

void CfgExternalToolModel::initFormats() {
    QList<DocumentFormatId> ids = AppContext::getDocumentFormatRegistry()->getRegisteredFormats();

    DocumentFormatConstraints commonConstraints;
    commonConstraints.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    commonConstraints.addFlagToExclude(DocumentFormatFlag_SingleObjectFormat);
    commonConstraints.addFlagToExclude(DocumentFormatFlag_CannotBeCreated);

    DocumentFormatConstraints seqWrite(commonConstraints);
    seqWrite.supportedObjectTypes += GObjectTypes::SEQUENCE;

    DocumentFormatConstraints seqRead(commonConstraints);
    seqRead.supportedObjectTypes += GObjectTypes::SEQUENCE;

    DocumentFormatConstraints msaWrite(commonConstraints);
    msaWrite.supportedObjectTypes += GObjectTypes::MULTIPLE_ALIGNMENT;

    DocumentFormatConstraints msaRead(commonConstraints);
    msaRead.supportedObjectTypes += GObjectTypes::MULTIPLE_ALIGNMENT;

    DocumentFormatConstraints annWrite(commonConstraints);
    annWrite.supportedObjectTypes += GObjectTypes::ANNOTATION_TABLE;

    DocumentFormatConstraints annRead(commonConstraints);
    annRead.supportedObjectTypes += GObjectTypes::ANNOTATION_TABLE;

    foreach(const DocumentFormatId& id, ids) {
        DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById(id);

        if (df->checkConstraints(seqWrite)) {
            seqFormatsW[df->getFormatName()] = df->getFormatId();
        }

        if (df->checkConstraints(seqRead)) {
            seqFormatsR[df->getFormatName()] = df->getFormatId();
        }

        if (df->checkConstraints(msaWrite)) {
            msaFormatsW[df->getFormatName()] = df->getFormatId();
        }

        if (df->checkConstraints(msaRead)) {
            msaFormatsR[df->getFormatName()] = df->getFormatId();
        }

        if (df->checkConstraints(annWrite)) {
            annFormatsW[df->getFormatName()] = df->getFormatId();
        }

        if (df->checkConstraints(annRead)) {
            annFormatsR[df->getFormatName()] = df->getFormatId();
        }
    }

    DocumentFormat *df = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::PLAIN_TEXT);
    if (isInput) {
        textFormat[tr("String value")] = DataConfig::StringValue;
    }
    textFormat[tr("Text file")] = df->getFormatId();
    if (!isInput) {
        textFormat[tr("Output file url")] = DataConfig::OutputFileUrl;
    }
}

void CfgExternalToolModel::initTypes() {
    DataTypePtr ptr = BaseTypes::DNA_SEQUENCE_TYPE();
    types[ptr->getDisplayName()] = ptr->getId();

    ptr = BaseTypes::ANNOTATION_TABLE_TYPE();
    types[ptr->getDisplayName()] = ptr->getId();

    ptr = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
    types[ptr->getDisplayName()] = ptr->getId();

    ptr = BaseTypes::STRING_TYPE();
    types[ptr->getDisplayName()] = ptr->getId();

    types["Sequence with annotations"] = SEQ_WITH_ANNS;
}

//////////////////////////////////////////////////////////////////////////
/// AttributeItem
//////////////////////////////////////////////////////////////////////////

QString AttributeItem::getName() const {
    return name;
}

void AttributeItem::setName(const QString& _name) {
    name = _name;
}

QString AttributeItem::getDataType() const {
    return type;
}

void AttributeItem::setDataType(const QString &_type) {
    type = _type;
}

QString AttributeItem::getDescription() const {
    return description;
}

void AttributeItem::setDescription(const QString &_description) {
    description = _description;
}

//////////////////////////////////////////////////////////////////////////
/// CfgExternalToolModelAttributes
//////////////////////////////////////////////////////////////////////////

CfgExternalToolModelAttributes::CfgExternalToolModelAttributes() {
    types["URL"] = "URL";
    types["String"] = "String";
    types["Number"] = "Number";
    types["Boolean"] = "Boolean";
    delegate = new ComboBoxDelegate(types);
}
CfgExternalToolModelAttributes::~CfgExternalToolModelAttributes() {
    foreach(AttributeItem* item, items) {
        delete item;
    }
}
int CfgExternalToolModelAttributes::rowCount(const QModelIndex & /* = QModelIndex */) const{
    return items.size();
}

int CfgExternalToolModelAttributes::columnCount(const QModelIndex & /* = QModelIndex */) const {
    return 3;
}

Qt::ItemFlags CfgExternalToolModelAttributes::flags(const QModelIndex &) const{
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

AttributeItem* CfgExternalToolModelAttributes::getItem(const QModelIndex &index) const {
    return items.at(index.row());
}

QList<AttributeItem*> CfgExternalToolModelAttributes::getItems() const {
    return items;
}

QVariant CfgExternalToolModelAttributes::data(const QModelIndex &index, int role /* = Qt::DisplayRole */) const {
    AttributeItem *item = getItem(index);
    int col = index.column();

    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
        if (col == 0) return item->getName();
        else if (col == 1) return delegate->getDisplayValue(item->getDataType());
        else if (col == 2) return item->getDescription();
        else return QVariant();
    case DelegateRole:
        if (col == 1) return qVariantFromValue<PropertyDelegate*>(delegate);
        else return QVariant();
    case Qt::EditRole:
    case ConfigurationEditor::ItemValueRole:
        if (col == 1) return item->getDataType();
        else return QVariant();
    default:
        return QVariant();
    }
}

bool CfgExternalToolModelAttributes::setData(const QModelIndex &index, const QVariant &value, int role) {
    int col = index.column();
    AttributeItem * item = getItem(index);
    switch (role) {
    case Qt::EditRole:
    case ConfigurationEditor::ItemValueRole:
        if (col == 0) {
            if (item->getName() != value.toString()) {
                item->setName(value.toString());
            }
        }
        else if (col == 1) {
            QString newType = value.toString();
            if (item->getDataType() != newType) {
                if (!newType.isEmpty()) {
                    item->setDataType(newType);
                }
            }
        }
        else if (col == 2) {
            if (item->getDescription() != value.toString()) {
                item->setDescription(value.toString());
            }
        }
        emit dataChanged(index, index);
    }
    return true;
}

QVariant CfgExternalToolModelAttributes::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0: return tr("Name");
        case 1: return tr("Type");
        case 2: return tr("Description");
        default: return QVariant();
        }
    }
    return QVariant();
}

bool CfgExternalToolModelAttributes::insertRows(int row, int count, const QModelIndex & parent)  {
    Q_UNUSED(row);
    Q_UNUSED(count);
    beginInsertRows(parent, items.size(), items.size());
    AttributeItem *newItem = new AttributeItem();
    newItem->setDataType("String");
    items.append(newItem);
    endInsertRows();
    return true;
}

bool CfgExternalToolModelAttributes::removeRows(int row, int count, const QModelIndex & parent) {
    Q_UNUSED(count);
    if (row >= 0 && row < items.size()) {
        beginRemoveRows(parent, row, row);
        items.removeAt(row);
        endRemoveRows();
        return true;
    }
    else {
        return false;
    }
}

} // U2
