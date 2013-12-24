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

#include <QtCore/QDebug>

#include <U2Core/Log.h>
#include <U2Lang/URLAttribute.h>
#include <U2Lang/WorkflowUtils.h>
#include <U2Lang/WorkflowSettings.h>
#include <U2Lang/IntegralBusType.h>
#include <U2Lang/Datatype.h>
#include <U2Lang/BaseTypes.h>

#include "WorkflowEditorDelegates.h"
#include "WorkflowEditor.h"
#include "ActorCfgModel.h"

namespace U2 {

/*****************************
 * ActorCfgModel
 *****************************/
static const int KEY_COLUMN = 0;
static const int VALUE_COLUMN = 1;
static const int SCRIPT_COLUMN = 2;

ActorCfgModel::ActorCfgModel(QObject *parent, SchemaConfig *_schemaConfig)
: QAbstractTableModel(parent), schemaConfig(_schemaConfig), subject(NULL), scriptMode(false) {
    scriptDelegate = new AttributeScriptDelegate();
}

ActorCfgModel::~ActorCfgModel() {
    delete scriptDelegate;
}

void ActorCfgModel::setActor(Actor* cfg) {
    listValues.clear();
    attrs.clear();
    //inputPortsData.reset();
    subject = cfg;
    if (cfg) {
        //attrs = cfg->getParameters().values();
        attrs = cfg->getAttributes();
        //inputPortsData.setData(cfg->getInputPorts());
        setupAttributesScripts();

        ConfigurationEditor *editor = subject->getEditor();
        if (NULL != editor) {
            foreach (Attribute *attr, attrs) {
                PropertyDelegate *delegate = editor->getDelegate(attr->getId());
                if (NULL != delegate) {
                    delegate->setSchemaConfig(schemaConfig);
                }
            }
        }
    }
    reset();
}

void dumpDescriptors(const QList<Descriptor> & descriptors) {
    foreach( const Descriptor & d, descriptors ) {
        qDebug() << d.getId() << d.getDisplayName();
    }
}

void ActorCfgModel::setupAttributesScripts() {
    foreach( Attribute * attribute, attrs ) {
        assert(attribute != NULL);
        attribute->getAttributeScript().clearScriptVars();
        
        DataTypePtr attributeType = attribute->getAttributeType();
        // FIXME: add support for all types in scripting
        if(attributeType != BaseTypes::STRING_TYPE() && attributeType != BaseTypes::NUM_TYPE()) {
            continue;
        }
        

        foreach(const PortDescriptor *descr, subject->getProto()->getPortDesciptors()) {
            if(descr->isInput()) {
                DataTypePtr dataTypePtr = descr->getType();
                if(dataTypePtr->isMap()) {
                    QMap<Descriptor, DataTypePtr> map = dataTypePtr->getDatatypesMap();
                    foreach(const Descriptor &desc,  map.keys()) {
                        //QString id = ptr->getId();
                        //QString displayName = ptr->getDisplayName();
                        //QString doc = ptr->getDocumentation();
                        //attribute->getAttributeScript().setScriptVar(Descriptor(id,displayName,doc), QVariant());
                        QString id = desc.getId().replace(QRegExp("[^a-zA-Z0-9_]"), "_");
                        if(id.at(0).isDigit()) {
                            id.prepend("_");
                        }
                        Descriptor d(id, desc.getDisplayName(), desc.getDocumentation());
                        attribute->getAttributeScript().setScriptVar(d, QVariant());
                    }
                }
                else  if(dataTypePtr->isList()) {
                    foreach(const Descriptor & typeDescr, dataTypePtr->getAllDescriptors()) {
                        QString id = typeDescr.getId().replace(QRegExp("[^a-zA-Z0-9_]"), "_");
                        if(id.at(0).isDigit()) {
                            id.prepend("_");
                        }                        
                        Descriptor d(id, typeDescr.getDisplayName(), typeDescr.getDocumentation());
                        attribute->getAttributeScript().setScriptVar(d, QVariant());
                    }
                }
                else {
                    QString id = dataTypePtr->getId().replace(QRegExp("[^a-zA-Z0-9_]"), "_");
                    if(id.at(0).isDigit()) {
                        id.prepend("_");
                    }
                    QString displayName = dataTypePtr->getDisplayName();
                    QString doc = dataTypePtr->getDocumentation();
                    attribute->getAttributeScript().setScriptVar(Descriptor(id,displayName,doc), QVariant());
                }
            }
        }

        QString attrVarName = attribute->getDisplayName();
        QString id = attribute->getId().replace(QRegExp("[^a-zA-Z0-9_]"), "_");
        if(id.at(0).isDigit()) {
            id.prepend("_");
        }
        attribute->getAttributeScript().
            setScriptVar(Descriptor(id, attrVarName, attribute->getDocumentation()), QVariant());
    }
}

void ActorCfgModel::update() {
    reset();
}

int ActorCfgModel::columnCount(const QModelIndex &) const { 
    if(scriptMode) {
        return 3; // key, value and script
    } else {
        return 2;
    }
}

int ActorCfgModel::rowCount( const QModelIndex & parent ) const {
    if(parent.isValid()) {
        return 0;
    }

    return attrs.isEmpty() || parent.isValid() ? 0 : attrs.size()/*rows*/;
}

bool ActorCfgModel::isVisible(Attribute *a) const {
    if (NULL != dynamic_cast<URLAttribute*>(a)) {
        return false;
    }
    const QVector<const AttributeRelation*> &relations = a->getRelations();
    bool hasVisibilityRelations = false;
    foreach(const AttributeRelation *relation, relations) {
        if (VISIBILITY == relation->getType()) {
            hasVisibilityRelations = true;
            QVariant visibilityValue = data(modelIndexById(relation->getRelatedAttrId()));
            if (relation->getAffectResult(visibilityValue, QVariant()).toBool()) {
                return true;
            }
        }
    }

    return !hasVisibilityRelations;
}

Qt::ItemFlags ActorCfgModel::flags( const QModelIndex & index ) const {
    Attribute *currentAttribute = attrs.at(index.row());
    if (!isVisible(currentAttribute)) {
        return 0;
    }

    int col = index.column();
    int row = index.row();
    switch(col) {
    case KEY_COLUMN:
        return Qt::ItemIsEnabled;
    case VALUE_COLUMN:
        return row < attrs.size() ? Qt::ItemIsEditable | Qt::ItemIsEnabled : Qt::ItemIsEnabled;
    case SCRIPT_COLUMN:
        {
            if(row < attrs.size()) {
                Attribute * currentAttribute = attrs.at(row);
                assert(currentAttribute != NULL);
                // FIXME: add support for all types in scripting
                if(currentAttribute->getAttributeType() != BaseTypes::STRING_TYPE() && currentAttribute->getAttributeType() != BaseTypes::NUM_TYPE()) {
                    return Qt::ItemIsEnabled;    
                } else {
                    return Qt::ItemIsEditable | Qt::ItemIsEnabled;
                }
            } else {
                return Qt::ItemIsEnabled;
            }
        }
    default:
        assert(false);
    }
    // unreachable code
    return Qt::NoItemFlags;
}

QVariant ActorCfgModel::headerData( int section, Qt::Orientation orientation, int role ) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
        case KEY_COLUMN: 
            return WorkflowEditor::tr("Name");
        case VALUE_COLUMN: 
            return WorkflowEditor::tr("Value");
        case SCRIPT_COLUMN:
            return WorkflowEditor::tr("Script");
        default:
            assert(false);
        }
    }
    // unreachable code
    return QVariant();
}

bool ActorCfgModel::setAttributeValue( const Attribute * attr, QVariant & attrValue ) const{
    assert(attr != NULL);
    bool isDefaultVal = attr->isDefaultValue();

    attrValue = attr->getAttributePureValue();
    return isDefaultVal;
}

Attribute* ActorCfgModel::getAttributeByRow(int row) const{
    QList<Attribute*>visibleAttrs;
    foreach(Attribute* a, attrs) {
        if (isVisible(a)) {
            visibleAttrs << a;
        }
    }
    return visibleAttrs.at(row);
}

QVariant ActorCfgModel::data(const QModelIndex & index, int role ) const {
    const Attribute *currentAttribute = attrs.at(index.row());
    if (role == DescriptorRole) { // descriptor that will be shown in under editor. 'propDoc' in WorkflowEditor
        return qVariantFromValue<Descriptor>(*currentAttribute);
    }
    
    int col = index.column();
    switch(col) {
    case KEY_COLUMN:
        {
            switch (role) {
            case Qt::DisplayRole: 
                return currentAttribute->getDisplayName();
            case Qt::ToolTipRole:
                return currentAttribute->getDocumentation();
            case Qt::FontRole:
                if (currentAttribute->isRequiredAttribute()) {
                    QFont fnt; 
                    fnt.setBold(true);
                    return QVariant(fnt);
                }
            default:
                return QVariant();
            }
        }
    case VALUE_COLUMN:
        {
            if (role == ConfigurationEditor::ItemListValueRole) {
                return listValues.value(currentAttribute->getId());
            }
            
            QVariant attributeValue;
            bool isDefaultVal = setAttributeValue(currentAttribute, attributeValue);
            ConfigurationEditor* confEditor = subject->getEditor();
            PropertyDelegate* propertyDelegate = confEditor ? confEditor->getDelegate(currentAttribute->getId()) : NULL;
            switch(role) {
            case Qt::DisplayRole: 
            case Qt::ToolTipRole:
                {
                    if(propertyDelegate) {
                        return propertyDelegate->getDisplayValue(attributeValue);
                    } else {
                        QString valueStr = WorkflowUtils::getStringForParameterDisplayRole(attributeValue);
                        return !valueStr.isEmpty() ? valueStr : attributeValue;
                    }
                }
            case Qt::ForegroundRole:
                return isDefaultVal ? QVariant(QColor(Qt::gray)) : QVariant();
            case DelegateRole:
                return qVariantFromValue<PropertyDelegate*>(propertyDelegate);
            case Qt::EditRole:
            case ConfigurationEditor::ItemValueRole:
                return attributeValue;
            default:
                return QVariant();
            }
        }
    case SCRIPT_COLUMN:
        {
            // FIXME: add support for all types in scripting
            if(currentAttribute->getAttributeType() != BaseTypes::STRING_TYPE() && currentAttribute->getAttributeType() != BaseTypes::NUM_TYPE() ) {
                if( role == Qt::DisplayRole || role == Qt::ToolTipRole ) {
                    return QVariant(tr("N/A"));
                } else {
                    return QVariant();
                }
            }
            
            // for STRING type
            switch(role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
                return scriptDelegate ? 
                    scriptDelegate->getDisplayValue(qVariantFromValue<AttributeScript>(currentAttribute->getAttributeScript())) 
                    : QVariant();
            case Qt::ForegroundRole:
                return currentAttribute->getAttributeScript().isEmpty() ? QVariant(QColor(Qt::gray)) : QVariant();
            case DelegateRole:
                assert(scriptDelegate != NULL);
                return qVariantFromValue<PropertyDelegate*>(scriptDelegate);
            case Qt::EditRole:
            case ConfigurationEditor::ItemValueRole:
                return qVariantFromValue<AttributeScript>(currentAttribute->getAttributeScript());
            default:
                return QVariant();
            }
        }
    default:
        assert(false);
    }
    // unreachable code
    return QVariant();
}

bool ActorCfgModel::canSetData(Attribute *attr, const QVariant &value) {
    bool dir = false;
    bool isOutUrlAttr = RFSUtils::isOutUrlAttribute(attr, subject, dir);
    CHECK(isOutUrlAttr, true);

    RunFileSystem *rfs = schemaConfig->getRFS();
    return rfs->canAdd(value.toString(), dir);
}

DelegateTags * getTags(Actor *subject, const QString &attrId) {
    ConfigurationEditor *editor = subject->getEditor();
    CHECK(NULL != editor, NULL);
    PropertyDelegate *delegate = editor->getDelegate(attrId);
    CHECK(NULL != delegate, NULL);
    return delegate->tags();
}

bool ActorCfgModel::setData( const QModelIndex & index, const QVariant & value, int role ) {
    int col = index.column();
    Attribute* editingAttribute = attrs[index.row()];
    //Attribute *editingAttribute = getAttributeByRow(index.row());
    assert(editingAttribute != NULL);
    
    switch(col) {
    case VALUE_COLUMN:
        {
            switch(role) {
            case ConfigurationEditor::ItemListValueRole: 
                {
                    listValues.insert(editingAttribute->getId(), value); 
                    return true;
                }
            case Qt::EditRole:
            case ConfigurationEditor::ItemValueRole:
                {
                    const QString& key = editingAttribute->getId();
                    if (editingAttribute->getAttributePureValue() != value) {
                        subject->setParameter(key, value);
                        emit dataChanged(index, index);
                        uiLog.trace("committed property change");
                    }
                    foreach (const AttributeRelation *relation, editingAttribute->getRelations()) {
                        if (relation->valueChangingRelation()) {
                            DelegateTags *inf = getTags(subject, editingAttribute->getId());
                            DelegateTags *dep = getTags(subject, relation->getRelatedAttrId());
                            Attribute *depAttr = subject->getParameter(relation->getRelatedAttrId());
                            QVariant newValue = relation->getAffectResult(value, depAttr->getAttributePureValue(), inf, dep);

                            if (canSetData(depAttr, newValue)) {
                                QModelIndex idx = modelIndexById(relation->getRelatedAttrId());
                                setData(idx, newValue);
                            }
                        }
                    }
                    return true;
                }
            default:
                return false;
            }
        }
    case SCRIPT_COLUMN:
        {
            switch(role) {
            case Qt::EditRole:
            case ConfigurationEditor::ItemValueRole:
                {
                    AttributeScript attrScript = value.value<AttributeScript>();
                    editingAttribute->getAttributeScript().setScriptText(attrScript.getScriptText());
                    emit dataChanged(index, index);
                    uiLog.trace(QString("user script for '%1' attribute updated").arg(editingAttribute->getDisplayName()));
                    return true;
                }
            default:
                return false;
            }
        }
    default:
        assert(false);
    }
    
    // unreachable code
    return false;
}

void ActorCfgModel::changeScriptMode(bool _mode) {
    scriptMode = _mode;
}

} // U2
