/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#ifndef _U2_MSA_COLLAPSIBLE_MODEL_H_
#define _U2_MSA_COLLAPSIBLE_MODEL_H_

#include <QtCore/QObject>
#include <QtCore/QVector>

#include <U2Core/global.h>

namespace U2 {

#define MODIFIER "modifier"
#define MAROW_SIMILARITY_SORT "marow_similarity_sort"

class MSACollapsableItem {
public:
    MSACollapsableItem();
    MSACollapsableItem(int startPos, int length);

    int row;
    int numRows;
    bool isCollapsed;
};

class U2Region;
class MSAEditorUI;
class MAlignment;
class MAlignmentModInfo;

class U2VIEW_EXPORT MSACollapsibleItemModel : public QObject {
    Q_OBJECT
public:
    MSACollapsibleItemModel(MSAEditorUI *p);

    // creates model with every item collapsed
    // 'itemRegions' has to be sorted list of non-intersecting regions
    void reset(const QVector<U2Region> &itemRegions);

    void reset();

    void toggle(int pos);

    void collapseAll(bool collapse);

    int mapToRow(int pos) const;

    U2Region mapToRows(int pos) const;

    U2Region mapSelectionRegionToRows(const U2Region &selectionRegion) const;

    /**
    * The method converts the row position in the whole msa into its "visible" position (i.e.
    * the row position that takes into account collapsed items).
    * Returns -1 if the row is inside a collapsed item and @failIfNotVisible is true.
    */
    int rowToMap(int row, bool failIfNotVisible = false) const;

    void getVisibleRows(int startPos, int endPos, QVector<U2Region> &rows) const;

    bool isTopLevel(int pos) const;

    int itemAt(int pos) const;

    int getItemPos(int index) const;

    MSACollapsableItem getItem(int index) const;

    int displayedRowsCount() const;

    /** If there is a collapsible item at 'pos' position, it is removed. */
    void removeCollapsedForPosition(int pos);

    bool isEmpty() const;

signals:
    void toggled();

private:
    void triggerItem(int index);
    int mapToRow(int lastItem, int pos) const;

private:
    MSAEditorUI* ui;
    QVector<MSACollapsableItem> items;
    QVector<int> positions;
};

} //namespace

#endif
