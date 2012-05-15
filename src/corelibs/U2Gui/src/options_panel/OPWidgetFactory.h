/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_OP_WIDGET_FACTORY_H_
#define _U2_OP_WIDGET_FACTORY_H_

#include <U2Core/global.h>

#include <U2Gui/ObjectViewModel.h>

#include <QtGui/QPixmap>


namespace U2 {


enum ObjectViewType {ObjViewType_SequenceView, ObjViewType_AlignmentEditor, ObjViewType_AssemblyBrowser};

struct U2GUI_EXPORT OPGroupParameters
{
public:
    OPGroupParameters(QString groupId, QPixmap headerImage, QString title);

    inline QString getGroupId() { return groupId; }
    inline QPixmap getIcon() { return groupIcon; }
    inline QString getTitle() { return groupTitle; }

private:
    QString groupId;
    QPixmap groupIcon;
    QString groupTitle;
};


enum OPGroupState {OPGroupState_Opened, OPGroupState_Closed};


class U2GUI_EXPORT OPWidgetFactory : public QObject
{
    Q_OBJECT

public:
    OPWidgetFactory();

    virtual QWidget* createWidget(GObjectView* objView) = 0;

    virtual OPGroupParameters getOPGroupParameters() = 0;

    virtual ObjectViewType getObjectViewType() { return objectViewOfWidget; }

    virtual OPGroupState getGroupState() { return groupState; }
    virtual void setGroupState(OPGroupState newState) { groupState = newState; }

protected:
    GObjectView*    objView;
    ObjectViewType  objectViewOfWidget;
    OPGroupState    groupState;
};

} // namespace

#endif

