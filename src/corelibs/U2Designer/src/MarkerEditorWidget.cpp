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

#include <U2Designer/EditMarkerGroupDialog.h>
#include <U2Designer/MarkerEditor.h>

#include <U2Lang/Marker.h>
#include <U2Lang/MarkerUtils.h>

#include "MarkerEditorWidget.h"

namespace U2 {

MarkerEditorWidget::MarkerEditorWidget(QAbstractTableModel *markerModel, QWidget *parent)
: QWidget(parent), markerModel(markerModel)
{
    setupUi(this);
    {
        table->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
        table->horizontalHeader()->setStretchLastSection(true);
        table->horizontalHeader()->setClickable(false);
        table->verticalHeader()->hide();
        table->verticalHeader()->setDefaultSectionSize(QFontMetrics(QFont()).height() + 6);
    }

    table->setModel(markerModel);

    connect(addButton, SIGNAL(clicked()), SLOT(sl_onAddButtonClicked()));
    connect(editButton, SIGNAL(clicked()), SLOT(sl_onEditButtonClicked()));
    connect(removeButton, SIGNAL(clicked()), SLOT(sl_onRemoveButtonClicked()));
}

void MarkerEditorWidget::sl_onAddButtonClicked() {
    EditMarkerGroupDialog dlg(true, NULL, this);

    if (dlg.exec()) {
        Marker *newMarker = dlg.getMarker();
        Workflow::MarkerGroupListCfgModel *model = dynamic_cast<Workflow::MarkerGroupListCfgModel*>(table->model());
        model->addMarker(newMarker);
    }
}

void MarkerEditorWidget::sl_onEditButtonClicked() {
    QItemSelectionModel *m = table->selectionModel();
    QModelIndexList selected = m->selectedRows();
    if (1 != selected.size()) {
        return;
    }

    Workflow::MarkerGroupListCfgModel *model = dynamic_cast<Workflow::MarkerGroupListCfgModel*>(table->model());
    EditMarkerGroupDialog dlg(false, model->getMarker(selected.first().row()), this);

    if (dlg.exec()) {
        Marker *newMarker = dlg.getMarker();
        model->replaceMarker(selected.first().row(), newMarker);
    }
}

void MarkerEditorWidget::sl_onRemoveButtonClicked() {
    QItemSelectionModel *m = table->selectionModel();
    QModelIndexList selected = m->selectedRows();
    if (1 != selected.size()) {
        return;
    }

    markerModel->removeRows(selected.first().row(), 1, selected.first());
}

bool MarkerEditorWidget::checkEditMarkerGroupResult(const QString &oldName, Marker *newMarker, QString &message) {
    Workflow::MarkerGroupListCfgModel *model = dynamic_cast<Workflow::MarkerGroupListCfgModel*>(table->model());
    QMap<QString, Marker*> &markers = model->getMarkers();


    if (oldName != newMarker->getName()) {
        foreach (Marker *m, markers.values()) {
            if (m->getName() == newMarker->getName()) {
                message.append(tr("Duplicate marker's name: %1").arg(newMarker->getName()));
                return false;
            }
        }
    }

    return true;
}

bool MarkerEditorWidget::checkAddMarkerGroupResult(Marker *newMarker, QString &message) {
    Workflow::MarkerGroupListCfgModel *model = dynamic_cast<Workflow::MarkerGroupListCfgModel*>(table->model());
    QMap<QString, Marker*> &markers = model->getMarkers();

    foreach (Marker *m, markers.values()) {
        if (m->getName() == newMarker->getName()) {
            message.append(tr("Duplicate marker's name: %1").arg(newMarker->getName()));
            return false;
        }
    }

    QRegExp rx("\\s");
    if (rx.indexIn(newMarker->getName()) >= 0) {
        message.append(tr("Marker's name contains spaces: %1").arg(newMarker->getName()));
        return false;
    }

    return true;
}

} // U2
