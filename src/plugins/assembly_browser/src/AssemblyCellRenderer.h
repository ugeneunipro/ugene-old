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

#include <U2Core/U2Assembly.h>

#include <QtCore/QMap>
#include <QtCore/QHash>

#include <QtGui/QColor>
#include <QtGui/QPixmap>
#include <QtGui/QFont>

namespace U2 {

/** Abstract cell renderer interface */
class AssemblyCellRenderer {
public:
    AssemblyCellRenderer() {}
    virtual ~AssemblyCellRenderer() {}

    /** Render pixmaps into cache */
    virtual void render(const QSize &size, bool text, const QFont &font) = 0;

    /** @returns cached cell pixmap */
    virtual QPixmap cellImage(char c) = 0;
    virtual QPixmap cellImage(const U2AssemblyRead &read, char c) = 0;

protected:
    static void drawCell(QPixmap &img, const QColor &color, bool text, char c, const QFont &font, const QColor &textColor);
    static void drawBackground(QPixmap &img, const QColor & color);
    static void drawText(QPixmap &img, char c, const QFont &font, const QColor & color);
};

/** Factory: create default cell renderer */
extern AssemblyCellRenderer* createAssemblyCellRenderer();

} //ns

#endif
