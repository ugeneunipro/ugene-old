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

#ifndef _U2_MSA_EDITOR_STATE_H_
#define _U2_MSA_EDITOR_STATE_H_

#include <U2Core/U2Region.h>
#include <U2Core/GObject.h>

#include <QtCore/QVariant>

namespace U2 {

class MSAEditor;


class U2VIEW_EXPORT MSAEditorState {
public:
    MSAEditorState(){}

    MSAEditorState(const QVariantMap& _stateData) : stateData(_stateData){}

    static QVariantMap saveState(MSAEditor* v);

    bool isValid() const;

    GObjectReference getMSAObjectRef() const;

    void setMSAObjectRef(const GObjectReference& ref);

    QFont getFont() const;
    void setFont(const QFont &f);

    int getFirstPos() const;
    void setFirstPos(int y);

    float getZoomFactor() const;
    void setZoomFactor(float zoomFactor);

    QVariantMap stateData;
};




} // namespace

#endif
