#include "MSACollapsibleModel.h"
#include "MSAEditor.h"

#include <U2Core/Log.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2Region.h>


namespace U2 {

MSACollapsibleItemModel::MSACollapsibleItemModel(MSAEditorUI* p) : QObject(p), ui(p) {
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
    items.clear();
    positions.clear();
    emit toggled();
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

int MSACollapsibleItemModel::rowToMap(int row) const {
    int invisibleRows = 0;
    for (QVector<MSACollapsableItem>::ConstIterator it = items.constBegin();
        it < items.constEnd() && it->row < row; it++) {
            if (it->isCollapsed) {
                invisibleRows += it->numRows - 1;
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

int MSACollapsibleItemModel::getLastPos() const {
    MSAEditor* ed = ui->getEditor();
    MAlignmentObject* o = ed->getMSAObject();
    const MAlignment& ma = o->getMAlignment();

    if (items.isEmpty()) {
        return ma.getNumRows() - 1;
    }

    const MSACollapsableItem& lastItem = items.last();
    int res = lastItem.row;
    if (!lastItem.isCollapsed) {
        res += lastItem.numRows - 1;
    }
    // determine if last collapsible item is last in MSA sequence list
    int lastPos = lastItem.row + lastItem.numRows - 1;
    if (lastPos < ma.getNumRows() - 1) {
        res += ma.getNumRows() -1 - res;
    }
    return res;
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

int MSACollapsibleItemModel::displayedRowsCount(){
    MSAEditor* ed = ui->getEditor();
    MAlignmentObject* o = ed->getMSAObject();
    int size = o->getNumRows();
    if (!items.isEmpty()) {
        foreach(const MSACollapsableItem item, items){
            if(item.isCollapsed){
                size -= item.numRows - 1; 
            }
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

} //namespace
