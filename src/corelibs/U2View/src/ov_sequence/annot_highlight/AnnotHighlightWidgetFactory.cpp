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

#include "AnnotHighlightWidget.h"
#include "AnnotHighlightWidgetFactory.h"

#include <U2Core/U2SafePoints.h>

#include <QObject>


namespace U2 {

const QString AnnotHighlightWidgetFactory::GROUP_ID = "OP_ANNOT_HIGHLIGHT";
const QString AnnotHighlightWidgetFactory::GROUP_ICON_STR = ":core/images/annotation_settings.png";
const QString AnnotHighlightWidgetFactory::GROUP_TITLE = QString(tr("Annotations Highlighting"));


AnnotHighlightWidgetFactory::AnnotHighlightWidgetFactory()
{
    objectViewOfWidget = ObjViewType_SequenceView;
}


QWidget* AnnotHighlightWidgetFactory::createWidget(GObjectView* objView)
{
    SAFE_POINT(NULL != objView,
        QString("Internal error: unable to create widget for group '%1', object view is NULL.").arg(GROUP_ID),
        NULL);
    
    AnnotatedDNAView* annotatedDnaView = qobject_cast<AnnotatedDNAView*>(objView);
    SAFE_POINT(NULL != annotatedDnaView,
        QString("Internal error: unable to cast object view to AnnotatedDNAView for group '%1'.").arg(GROUP_ID),
        NULL);

    AnnotHighlightWidget* widget = new AnnotHighlightWidget(annotatedDnaView);
    return widget;
}


OPGroupParameters AnnotHighlightWidgetFactory::getOPGroupParameters()
{
    return OPGroupParameters(GROUP_ID, QPixmap(GROUP_ICON_STR), GROUP_TITLE);
}


} // namespace
