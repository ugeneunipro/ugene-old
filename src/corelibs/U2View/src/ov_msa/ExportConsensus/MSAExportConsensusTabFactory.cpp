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

#include "MSAExportConsensusTabFactory.h"
#include "MSAExportConsensusTab.h"

#include <U2Core/U2SafePoints.h>

#include <QtCore/QObject>

namespace U2 {

const QString MSAExportConsensusFactoryTab::GROUP_ID = "OP_EXPORT_CONSENSUS";
const QString MSAExportConsensusFactoryTab::GROUP_ICON_STR = ":core/images/todo.png";
const QString MSAExportConsensusFactoryTab::GROUP_TITLE = QString(MSAExportConsensusFactoryTab::tr("Export consensus"));
const QString MSAExportConsensusFactoryTab::GROUP_DOC_PAGE = "4227131";


MSAExportConsensusFactoryTab::MSAExportConsensusFactoryTab(){
    objectViewOfWidget = ObjViewType_AlignmentEditor;
}


QWidget* MSAExportConsensusFactoryTab::createWidget(GObjectView* objView){
    SAFE_POINT(NULL != objView,
        QString("Internal error: unable to create widget for group '%1', object view is NULL.").arg(GROUP_ID),
        NULL);

    MSAEditor* msa = qobject_cast<MSAEditor*>(objView);
    SAFE_POINT(NULL != msa,
        QString("Internal error: unable to cast object view to MSAEditor for group '%1'.").arg(GROUP_ID),
        NULL);

    MSAExportConsensusTab *widget = new MSAExportConsensusTab(msa);
    return widget;
}


OPGroupParameters MSAExportConsensusFactoryTab::getOPGroupParameters(){
    return OPGroupParameters(GROUP_ID, QPixmap(GROUP_ICON_STR), GROUP_TITLE, GROUP_DOC_PAGE);
}

} // namespace
