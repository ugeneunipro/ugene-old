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

#include "MSACollapsibleModel.h"
#include "MSAEditor.h"

#include <U2Core/Log.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2Region.h>


namespace U2 {

//////////////////////////////////////////////////////////////////////////
/// MSACollapsableItem
//////////////////////////////////////////////////////////////////////////

MSACollapsableItem::MSACollapsableItem()
    : row(-1), numRows(-1), isCollapsed(false)
{

}

MSACollapsableItem::MSACollapsableItem(int startPos, int length)
    : row(startPos), numRows(length), isCollapsed(false)
{

}

//////////////////////////////////////////////////////////////////////////
/// MSACollapsibleItemModel
//////////////////////////////////////////////////////////////////////////

MSACollapsibleItemModel::MSACollapsibleItemModel(MSAEditorUI *p)
    : QObject(p), ui(p)
{

}

void MSACollapsibleItemModel::reset(const QVector<U2Region>& itemRegions) {
    items.clear();
    positions.clear();
    foreach(const U2Region& r, itemRegions) {
        if(r.length <= 1) {
            continue;
        }
        items.append(MSACollapsableItem(r.startPos, r.length));
        positions.append(r.startPos);
    }
    collapseAll(true);
}

void MSACollapsibleItemModel::reset() {
    const bool modelExists = ( !items.isEmpty( ) || !positions.isEmpty( ) );
    if ( modelExists ) {
        items.clear( );
        positions.clear( );
        emit toggled( );
    }
}

void MSACollapsibleItemModel::collapseAll(bool collapse) {
    int delta = 0;
    for (int i=0; i < items.size(); i++) {
        MSACollapsableItem& item = items[i];
        positions[i] = item.row - delta;
        item.isCollapsed = collapse;
        if (collapse) {
            delta += item.numRows - 1;
        }
    }
    emit toggled();
}

void MSACollapsibleItemModel::toggle(int pos) {
    QVector<int>::ConstIterator i = qBinaryFind(positions, pos);
    assert(i != positions.constEnd());
    int index = i - positions.constBegin();
    triggerItem(index);
    emit toggled();
}

void MSACollapsibleItemModel::triggerItem(int index) {
    MSACollapsableItem& item = items[index];
    item.isCollapsed = !item.isCollapsed;
    int delta = item.numRows - 1;
    assert(delta > 0);
    if (item.isCollapsed) {
        delta *= -1;
    }
    for (int j = index + 1; j < items.size(); j++ ) {
        positions[j] += delta;
    }
}

int MSACollapsibleItemModel::mapToRow(int lastItem, int pos) const {
    const MSACollapsableItem& item = items.at(lastItem);
    int row = item.row + pos - positions.at(lastItem);
    if (item.isCollapsed) {
        row += item.numRows - 1;
    }
    return row;
}

int MSACollapsibleItemModel::mapToRow(int pos) const {
    QVector<int>::ConstIterator i = qLowerBound(positions, pos);
    int idx = i - positions.constBegin() - 1;
    if (idx < 0) {
        return pos;
    } else {
        return mapToRow(idx, pos);
    }
}

U2Region MSACollapsibleItemModel::mapToRows(int pos) const {
    QVector<int>::ConstIterator i = qLowerBound(positions, pos);

    int idx = i - positions.constBegin();

    if (i < positions.constEnd() && *i == pos) {
        // 'pos' is top row in group
        const MSACollapsableItem& item = items.at(idx);
        if (item.isCollapsed) {
            return U2Region(item.row, item.numRows);
        }
        return U2Region(item.row, 1);
    }

    --idx;
    int startPos = 0;
    if (idx < 0) {
        startPos = pos;
    } else {
        startPos = mapToRow(idx, pos);
    }
    return U2Region(startPos, 1);
}

U2Region MSACollapsibleItemModel::mapSelectionRegionToRows(const U2Region& selectionRegion) const {
    if (selectionRegion.isEmpty()) {
        return U2Region();
    }

    if (!ui->isCollapsibleMode()) {
        return selectionRegion;
    }

    int startPos = selectionRegion.startPos;
    int endPos = startPos + selectionRegion.length - 1;

    int startSeq = 0;
    int endSeq = 0;

    int startItemIdx = itemAt(startPos);

    if (startItemIdx >= 0) {
        const MSACollapsableItem& startItem = getItem(startItemIdx);
        startSeq = startItem.row;
    } else {
        startSeq = mapToRow(startPos);
    }

    int endItemIdx = itemAt(endPos);

    if (endItemIdx >= 0) {
        const MSACollapsableItem& endItem = getItem(endItemIdx);
        endSeq = endItem.row + endItem.numRows;
    } else {
        endSeq = mapToRow(endPos) + 1;
    }

    return U2Region(startSeq, endSeq - startSeq);
}

int MSACollapsibleItemModel::rowToMap(int row, bool failIfNotVisible) const {
    int invisibleRows = 0;
    for (QVector<MSACollapsableItem>::ConstIterator it = items.constBegin(); it < items.constEnd() && it->row < row; it++) {
        if (it->isCollapsed) {
            if (it->row + it->numRows > row && failIfNotVisible) {
                return -1;
            }
            invisibleRows += (it->row + it->numRows <= row) ? it->numRows - 1 : row - it->row;
        }
    }
    return row - invisibleRows;
}

void MSACollapsibleItemModel::getVisibleRows(int startPos, int endPos, QVector<U2Region>& range) const {
    if (items.isEmpty()) {
        range.append(U2Region(startPos, endPos - startPos + 1));
        return;
    }
    QVector<int>::ConstIterator i = qLowerBound(positions, startPos);
    int idx = i - positions.constBegin() - 1;
    int start = 0;
    if (idx < 0) {
        start = startPos;
    } else {
        start = mapToRow(idx, startPos);
    }

    int j = i - positions.constBegin();
    for (; j < items.size(); j++) {
        const MSACollapsableItem& item = items.at(j);
        if (positions[j] > endPos)
            break;
        if (item.isCollapsed) {
            range.append(U2Region(start, item.row - start + 1));
            start = item.row + item.numRows;
        }
    }

    int lastRow = 0;
    if (j - 1 < 0) {
        lastRow = endPos;
    } else {
        lastRow = mapToRow(j - 1, endPos);
    }

    MSAEditor* ed = ui->getEditor();
    MAlignmentObject* obj = ed->getMSAObject();
    int alnNumRows = obj->getNumRows();
    lastRow = qMin(lastRow, alnNumRows - 1);
    int len = lastRow - start + 1;
    if (len>0) {
        range.append(U2Region(start, len));
    }
}


bool MSACollapsibleItemModel::isTopLevel(int pos) const {
    QVector<int>::ConstIterator i = qBinaryFind(positions, pos);
    if (i==positions.constEnd()) {
        return false;
    }
    return true;
}

int MSACollapsibleItemModel::itemAt(int pos) const {
    QVector<int>::ConstIterator i = qLowerBound(positions, pos);

    if (i < positions.constEnd() && *i == pos) {
        return i - positions.constBegin();
    }

    int closest = i - positions.constBegin() - 1;
    if (closest < 0) {
        return -1;
    }

    const MSACollapsableItem& it = items.at(closest);
    if (it.isCollapsed) {
        return -1;
    } else {
        int itBottom = positions.at(closest) + it.numRows - 1;
        if (pos <= itBottom) {
            return closest;
        }
        return -1;
    }
}

int MSACollapsibleItemModel::getItemPos(int index) const {
    return positions.at(index);
}

MSACollapsableItem MSACollapsibleItemModel::getItem(int index) const {
    return items.at(index);
}

int MSACollapsibleItemModel::displayedRowsCount() const {
    MSAEditor *ed = ui->getEditor();
    MAlignmentObject *o = ed->getMSAObject();
    int size = o->getNumRows();
    foreach (const MSACollapsableItem &item, items) {
        if (item.isCollapsed) {
            size -= item.numRows - 1;
        }
    }
    return size;
}

void MSACollapsibleItemModel::removeCollapsedForPosition(int index) {
    for (int i = 0, n = items.size(); i < n; ++i) {
        MSACollapsableItem &collapsedItem = items[i];

        int itemStart = collapsedItem.row;
        int itemEnd = itemStart + collapsedItem.numRows;

        if ((index >= itemStart) && (index < itemEnd)) {
            items.remove(i);

            int positionIndex = positions.indexOf(itemStart);
            positions.remove(positionIndex);
        }
    }
}

bool MSACollapsibleItemModel::isEmpty() const {
    return items.isEmpty();
}

} // namespace U2
