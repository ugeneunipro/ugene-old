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

#include "MSAGeneralTab.h"
#include "MSAGeneralTabFactory.h"

#include <U2Core/U2SafePoints.h>

#include <QObject>

namespace U2 {

const QString MSAGeneralTabFactory::GROUP_ID = "OP_MSA_GENERAL";
const QString MSAGeneralTabFactory::GROUP_ICON_STR = ":core/images/settings2.png";
const QString MSAGeneralTabFactory::GROUP_TITLE = QString(tr("General"));


MSAGeneralTabFactory::MSAGeneralTabFactory(){
    objectViewOfWidget = ObjViewType_AlignmentEditor;
}


QWidget* MSAGeneralTabFactory::createWidget(GObjectView* objView){
    SAFE_POINT(NULL != objView,
        QString("Internal error: unable to create widget for group '%1', object view is NULL.").arg(GROUP_ID),
        NULL);

    MSAEditor* msa = qobject_cast<MSAEditor*>(objView);
    SAFE_POINT(NULL != msa,
        QString("Internal error: unable to cast object view to MSAEditor for group '%1'.").arg(GROUP_ID),
        NULL);

    MSAGeneralTab *widget = new MSAGeneralTab();
    return widget;
}


OPGroupParameters MSAGeneralTabFactory::getOPGroupParameters(){
    return OPGroupParameters(GROUP_ID, QPixmap(GROUP_ICON_STR), GROUP_TITLE);
}


} // namespace
