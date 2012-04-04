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

#include "AnnotHighlightTree.h"
#include "AnnotHighlightTreeItem.h"

#include <U2Gui/GUIUtils.h>



namespace U2 {

const int AnnotHighlightTreeItem::ROW_HEIGHT = 22;

AnnotHighlightTreeItem::AnnotHighlightTreeItem(const QString& _annotName, const QColor& _annotColor)
    : annotName(_annotName), annotColor(_annotColor)
{
    setText(AnnotHighlightTree::COL_NUM_ANNOT_NAME, annotName);
    drawColorCell();
    setSizeHint(AnnotHighlightTree::COL_NUM_ANNOT_NAME, QSize(0, ROW_HEIGHT));
}


void AnnotHighlightTreeItem::drawColorCell()
{
    QIcon icon = GUIUtils::createSquareIcon(annotColor, 14);
    setIcon(AnnotHighlightTree::COL_NUM_COLOR, icon);
}


void AnnotHighlightTreeItem::setColor(const QColor& _annotColor)
{
    annotColor = _annotColor;
    drawColorCell();
}

} // namespace
