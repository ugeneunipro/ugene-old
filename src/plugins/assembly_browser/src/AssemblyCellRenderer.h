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

#ifndef __ASSEMBLY_CELL_RENDERER__
#define __ASSEMBLY_CELL_RENDERER__

#include <QtCore/QMap>
#include <QtCore/QVector>

#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QFont>

namespace U2 {

class AssemblyCellRenderer {
public:
    AssemblyCellRenderer();

    void render(const QSize & size, bool text = false, const QFont & font = QFont());

    QImage cellImage(char c);

private:
    void drawCells(const QSize & size, const QFont & font, bool text);

    void drawText(char c, const QFont & f);

    void drawCell(char c, const QColor & color);

private:
    QMap<char, QColor> colorScheme;
    QVector<QImage> images;
    QSize cachedSize;
    bool cachedTextFlag;
};

} //ns

#endif
