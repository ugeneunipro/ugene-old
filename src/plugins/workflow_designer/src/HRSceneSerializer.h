/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_WORKLFOW_SUPPORT_HR_FORMAT_UTILS_H_
#define _U2_WORKLFOW_SUPPORT_HR_FORMAT_UTILS_H_

#include "WorkflowViewController.h"
#include "WorkflowViewItems.h"
#include "ItemViewStyle.h"

namespace U2 {

class HRSceneSerializer : QObject {
    Q_OBJECT
public:
    static QString scene2String(WorkflowScene * scene, const Metadata & meta);
    // returns error or empty string
    static QString string2Scene(const QString & data, WorkflowScene * scene, Metadata * meta = NULL, 
                                bool select = false, bool pasteMode = false, QList<QString> includedUrls = QList<QString>());
    
    static QString items2String(const QList<QGraphicsItem*> & items, const QList<Iteration> & iterations);

    static ActorId newActorId(const QString &id, const QList<Actor*> &procs);
    
}; // HRSceneSerializer

} // U2

#endif // _U2_WORKLFOW_SUPPORT_HR_FORMAT_UTILS_H_
