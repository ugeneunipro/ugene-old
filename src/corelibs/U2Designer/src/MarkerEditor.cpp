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

#include <QHeaderView>
#include <QSplitter>
#include <QTableView>

#include <U2Designer/MarkerEditorWidget.h>

#include <U2Lang/BaseTypes.h>
#include <U2Lang/Marker.h>
#include <U2Lang/MarkerAttribute.h>

#include "MarkerEditor.h"

namespace U2 {

namespace Workflow {

/* ***********************************************************************
 * MarkerEditor
 * ***********************************************************************/
MarkerEditor::MarkerEditor()
: ActorConfigurationEditor(), markerModel(NULL)
{
}

MarkerEditor::~MarkerEditor() {
}

QWidget *MarkerEditor::getWidget() {
    return createGUI();
}

QWidget *MarkerEditor::createGUI() {
    if (NULL == markerModel) {
        return NULL;
    }

    return new MarkerEditorWidget(markerModel);
}

void MarkerEditor::setConfiguration(Actor *actor) {
    ActorConfigurationEditor::setConfiguration(actor);
    QMap<QString, Attribute*> attrs = cfg->getParameters();

    MarkerAttribute *mAttr = NULL;
    foreach (QString key, attrs.keys()) {
        Attribute *attr = attrs.value(key);
        if (MARKER_GROUP == attr->getGroup()) {
            if (NULL != mAttr) {
                assert(0);
                mAttr = NULL;
                break;
            }
            mAttr = dynamic_cast<MarkerAttribute*>(attr);
        }
    }

    if (NULL == mAttr) {
        return;
    }

    markerModel = new MarkerGroupListCfgModel(this, mAttr->getMarkers());
    connect(markerModel, SIGNAL(si_markerEdited(const QString &, const QString &)), SLOT(sl_onMarkerEdited(const QString &, const QString &)));
    connect(markerModel, SIGNAL(si_markerAdded(const QString &)), SLOT(sl_onMarkerAdded(const QString &)));
    connect(markerModel, SIGNAL(si_markerRemoved(const QString &, const QString &)), SLOT(sl_onMarkerRemoved(const QString &, const QString &)));
}

void MarkerEditor::sl_onMarkerEdited(const QString &markerId, const QString &oldMarkerName) {
    Marker *marker = markerModel->getMarkers().value(markerId);

    { // TODO: make common way to get marked object output port
        assert(1 == cfg->getOutputPorts().size());
        Port *outPort = cfg->getOutputPorts().at(0);
        assert(outPort->getOutputType()->isMap());
        QMap<Descriptor, DataTypePtr> outTypeMap = outPort->getOutputType()->getDatatypesMap();

        Descriptor newSlot = MarkerSlots::getSlotByMarkerType(marker->getType(), marker->getName());
        outTypeMap.remove(oldMarkerName);
        outTypeMap[newSlot] = BaseTypes::STRING_TYPE();
        DataTypePtr newType(new MapDataType(dynamic_cast<Descriptor&>(*(outPort->getType())), outTypeMap));
        outPort->setNewType(newType);
    }
}

void MarkerEditor::sl_onMarkerAdded(const QString &markerId) {
    Marker *marker = markerModel->getMarkers().value(markerId);

    { // TODO: make common way to get marked object output port
        assert(1 == cfg->getOutputPorts().size());
        Port *outPort = cfg->getOutputPorts().at(0);
        assert(outPort->getOutputType()->isMap());
        QMap<Descriptor, DataTypePtr> outTypeMap = outPort->getOutputType()->getDatatypesMap();

        Descriptor newSlot = MarkerSlots::getSlotByMarkerType(marker->getType(), marker->getName());
        outTypeMap[newSlot] = BaseTypes::STRING_TYPE();
        DataTypePtr newType(new MapDataType(dynamic_cast<Descriptor&>(*(outPort->getType())), outTypeMap));
        outPort->setNewType(newType);
    }
}

void MarkerEditor::sl_onMarkerRemoved(const QString &markerId, const QString &markerName) {
    Q_UNUSED(markerId);
    { // TODO: make common way to get marked object output port
        assert(1 == cfg->getOutputPorts().size());
        Port *outPort = cfg->getOutputPorts().at(0);
        assert(outPort->getOutputType()->isMap());
        QMap<Descriptor, DataTypePtr> outTypeMap = outPort->getOutputType()->getDatatypesMap();

        outTypeMap.remove(markerName);
        DataTypePtr newType(new MapDataType(dynamic_cast<Descriptor&>(*(outPort->getType())), outTypeMap));
        outPort->setNewType(newType);
    }
}

/* ***********************************************************************
* MarkerCfgModel
* ***********************************************************************/
MarkerGroupListCfgModel::MarkerGroupListCfgModel(QObject *parent, QMap<QString, Marker*> &markers)
: QAbstractTableModel(parent), markers(markers)
{

}

QVariant MarkerGroupListCfgModel::data(const QModelIndex &index, int role) const {
    if (Qt::DisplayRole == role || Qt::ToolTipRole == role) {
        Marker *m = *(markers.begin()+index.row());
        if (NULL == m) {
            return QVariant();
        }

        if (0 == index.column()) {
            return m->getName();
        } else if (1 == index.column()) {
            return m->toString();
        }
    } else {
        return QVariant();
    }
    return QVariant();
}

int MarkerGroupListCfgModel::columnCount(const QModelIndex &) const {
    return 2;
}

int MarkerGroupListCfgModel::rowCount(const QModelIndex &) const {
    return markers.size();
}

Qt::ItemFlags MarkerGroupListCfgModel::flags( const QModelIndex & ) const {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant MarkerGroupListCfgModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
        case 0:
            return tr("Marker Group");
        case 1:
            return tr("Marker Value");
        default:
            assert(false);
        }
    }
    // unreachable code
    return QVariant();
}

bool MarkerGroupListCfgModel::setData( const QModelIndex & index, const QVariant & value, int role ) {
    Q_UNUSED(index); Q_UNUSED(value); Q_UNUSED(role);
    return true;
}

bool MarkerGroupListCfgModel::removeRows(int row, int count, const QModelIndex &) {
    if (1 != count) {
        return true;
    }
    QMap<QString, Marker*>::iterator i = markers.begin();
    i += row;
    QString markerId = markers.key(*i);
    QString markerName = (*i)->getName();
    beginRemoveRows(QModelIndex(), row, row+count-1);
    markers.remove(markerId);
    endRemoveRows();

    emit si_markerRemoved(markerId, markerName);

    return true;
}

Marker *MarkerGroupListCfgModel::getMarker(int row) {
    QMap<QString, Marker*>::iterator i = markers.begin() + row;
    
    return *i;
}

QMap<QString, Marker*> &MarkerGroupListCfgModel::getMarkers() {
    return markers;
}

void MarkerGroupListCfgModel::addMarker(Marker *newMarker) {
    // TODO: generate id not using crazy methods like this
    QString markerId;
    if (markers.keys().isEmpty()) {
        markerId = "m1";
    } else {
        markerId = markers.keys().last()+"m";
        while (markers.keys().last() >= markerId) {
            markerId = "m" + QByteArray::number(qrand());
        }
    }

    beginInsertRows(QModelIndex(), rowCount(QModelIndex()), rowCount(QModelIndex()) + 1);
    markers.insert(markerId, newMarker);
    endInsertRows();

    emit si_markerAdded(markerId);
}

void MarkerGroupListCfgModel::replaceMarker(int row, Marker *newMarker) {
    QString markerId = markers.keys().at(row);

    Marker *oldMarker = markers.value(markerId);
    QString oldName = oldMarker->getName();
    delete oldMarker;

    markers.insert(markerId, newMarker);

    emit si_markerEdited(markerId, oldName);
}

} // Workflow

} // U2
