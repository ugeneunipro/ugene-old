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

#include <U2Core/L10n.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2SafePoints.h>

#include "ProjectUtils.h"
#include "ProjectViewModel.h"

#include "ProjectFilterProxyModel.h"

namespace U2 {

ProjectFilterProxyModel::ProjectFilterProxyModel(const ProjectTreeControllerModeSettings &settings, QObject *p)
    : QSortFilterProxyModel(p), settings(settings)
{
    setDynamicSortFilter(true);
    setFilterKeyColumn(0);
}

void ProjectFilterProxyModel::updateSettings(const ProjectTreeControllerModeSettings &newSettings) {
    settings = newSettings;
    invalidateFilter();
}

ProjectViewModel * ProjectFilterProxyModel::sourceModel() const {
    ProjectViewModel *srcModel = qobject_cast<ProjectViewModel *>(QSortFilterProxyModel::sourceModel());
    SAFE_POINT(NULL != srcModel, L10N::nullPointerError("project view model"), NULL);
    return srcModel;
}

QModelIndex ProjectFilterProxyModel::getIndexForDoc(Document *doc) const {
    ProjectViewModel *srcModel = sourceModel();
    CHECK(NULL != srcModel, QModelIndex());
    return mapFromSource(srcModel->getIndexForDoc(doc));
}

bool ProjectFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
    ProjectViewModel *srcModel = sourceModel();
    CHECK(NULL != srcModel, false);

    const QModelIndex index = srcModel->index(sourceRow, 0, sourceParent);
    ProjectViewModel::Type itemType = srcModel->itemType(index);
    switch (itemType) {
    case ProjectViewModel::DOCUMENT :
        return settings.isDocumentShown(srcModel->toDocument(index));
    case ProjectViewModel::FOLDER :
        return filterAcceptsFolder(srcModel->toFolder(index));
    case ProjectViewModel::OBJECT :
        return settings.isObjectShown(srcModel->toObject(index));
    default :
        FAIL("Unexpected project item type", false);
    }
}

bool ProjectFilterProxyModel::filterAcceptsFolder(const Folder *folder) const {
    SAFE_POINT(NULL != folder, "Invalid folder detected", false);

    ProjectViewModel *srcModel = sourceModel();
    CHECK(NULL != srcModel, true);

    const QString path = folder->getFolderPath();

    if (!settings.isObjectFilterActive()) {
        return true;
    } else if (ProjectUtils::isFolderInRecycleBinSubtree(path)) {
        return false;
    }

    Document *doc = folder->getDocument();
    QList<GObject *> objs = srcModel->getFolderObjects(doc, path);
    foreach(GObject *obj, objs) {
        if (settings.isObjectShown(obj)) {
            return true;
        }
    }

    foreach (Folder *folder, srcModel->getSubfolders(doc, path)) {
        if (filterAcceptsFolder(folder)) {
            return true;
        }
    }

    return false;
}

bool ProjectFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
    if (!left.isValid() && right.isValid()) {
        return false;
    } else if (left.isValid() && !right.isValid()) {
        return true;
    }

    ProjectViewModel *srcModel = sourceModel();
    CHECK(NULL != srcModel, true);

    ProjectViewModel::Type leftType = srcModel->itemType(left);
    ProjectViewModel::Type rightType = srcModel->itemType(right);

    const QString leftName = left.data(Qt::DisplayRole).toString();
    const QString rightName = right.data(Qt::DisplayRole).toString();

    if (leftType == rightType && leftType == ProjectViewModel::FOLDER) { // keep the Recycle Bin on top
        if (U2ObjectDbi::RECYCLE_BIN_FOLDER == leftName) {
            return true;
        } else if (U2ObjectDbi::RECYCLE_BIN_FOLDER == rightName) {
            return false;
        }
    }

    if (leftType == rightType) {
        return leftName < rightName;
    } else if (ProjectViewModel::FOLDER == leftType && ProjectViewModel::OBJECT == rightType) {
        return true;
    } else if (ProjectViewModel::OBJECT == leftType && ProjectViewModel::FOLDER == rightType) {
        return false;
    } else {
        FAIL("Unexpected project item types", false);
    }
}

} // namespace U2
