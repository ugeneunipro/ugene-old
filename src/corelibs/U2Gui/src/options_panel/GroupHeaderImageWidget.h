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

#ifndef _U2_GROUP_HEADER_IMAGE_WIDGET_H_
#define _U2_GROUP_HEADER_IMAGE_WIDGET_H_

#include <QtGui/QtGui>


namespace U2 {

/**
 * Widget with image that represents a group header.
 */
class GroupHeaderImageWidget : public QLabel
{
    Q_OBJECT
public:
    GroupHeaderImageWidget(const QPixmap& image);

    void setHeaderSelected();
    void setHeaderDeselected();

signals:
    /** Emitted when the widget has been pressed */
    void si_groupHeaderPressed(GroupHeaderImageWidget*, bool ctrlHold);

protected:
    virtual void mousePressEvent(QMouseEvent*);

private:
    static const QString HEADER_COMMON_STYLE;
};


} // namespace

#endif

