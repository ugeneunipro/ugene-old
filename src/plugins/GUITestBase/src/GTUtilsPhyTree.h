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

#ifndef GTUTILSPHYTREE_H
#define GTUTILSPHYTREE_H

#include "api/GTGlobals.h"
#include "U2View/TreeViewer.h"

namespace U2{

class GTUtilsPhyTree
{
public:
    static QList<QGraphicsItem *> getSelectedNodes(U2OpStatus& os);
    static QList<QGraphicsItem *> getUnselectedNodes(U2OpStatus& os);
    static QList<QGraphicsItem *> getNodes(U2OpStatus& os);

    static QList<QGraphicsSimpleTextItem*> getLabels(U2OpStatus& os);
    static QList<QGraphicsSimpleTextItem*> getDistances(U2OpStatus& os);
    static QStringList getLabelsText(U2OpStatus& os);
    static QList<double> getDistancesValues(U2OpStatus& os);
    static QPoint getGlobalCoord(U2OpStatus& os, QGraphicsItem* item);
private:
    static QList<QGraphicsItem *> getNodes(U2OpStatus& os, int width);
};
}
#endif // GTUTILSPHYTREE_H
