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

#ifndef _U2_MSA_EDITOR_TASKS_H_
#define _U2_MSA_EDITOR_TASKS_H_

#include <U2Core/GObjectReference.h>
#include <U2Gui/ObjectViewTasks.h>

namespace U2 {

class MAlignmentObject;
class UnloadedObject;
class MSAEditor;

class OpenMSAEditorTask : public ObjectViewTask {
    Q_OBJECT
public:
    OpenMSAEditorTask(MAlignmentObject* obj);
    OpenMSAEditorTask(UnloadedObject* obj);
    OpenMSAEditorTask(Document* doc);

    virtual void open();

    static void updateTitle(MSAEditor* msaEd);

private:
    QPointer<MAlignmentObject>  msaObject;
    GObjectReference            unloadedReference;
};

class OpenSavedMSAEditorTask : public ObjectViewTask {
    Q_OBJECT
public:
    OpenSavedMSAEditorTask(const QString& viewName, const QVariantMap& stateData);
    virtual void open();

    static void updateRanges(const QVariantMap& stateData, MSAEditor* ctx);
    static void addAnnotations(const QVariantMap& stateData, MSAEditor* ctx);
};


class UpdateMSAEditorTask : public ObjectViewTask {
public:
    UpdateMSAEditorTask(GObjectView* v, const QString& stateName, const QVariantMap& stateData);

    virtual void update();
};

} // namespace

#endif
