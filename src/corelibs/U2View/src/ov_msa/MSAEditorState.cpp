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

#include "MSAEditorState.h"
#include "MSAEditor.h"
#include "MSAEditorFactory.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/DNASequenceSelection.h>

namespace U2 {

#define VIEW_ID     QString("view_id")
#define MSA_OBJ     QString("msa_obj_ref")
#define FONT        QString("font")
#define FIRST_POS   QString("first_pos")
#define ZOOM_FACTOR QString("zoom_factor")

bool MSAEditorState::isValid() const {
    return stateData.value(VIEW_ID) == MSAEditorFactory::ID;
}

GObjectReference MSAEditorState::getMSAObjectRef() const {
    return stateData.contains(MSA_OBJ) ? stateData[MSA_OBJ].value<GObjectReference>() : GObjectReference();
}

void MSAEditorState::setMSAObjectRef(const GObjectReference& ref) {
    stateData[MSA_OBJ] = QVariant::fromValue<GObjectReference>(ref);
}

QFont MSAEditorState::getFont() const {

    QVariant v = stateData.value(FONT);
    if (v.type() == QVariant::Font) {
        return v.value<QFont>();
    }
    return QFont();
}

void MSAEditorState::setFont(const QFont &f) {

    stateData[FONT] = f;
}

int MSAEditorState::getFirstPos() const {
    QVariant v = stateData.value(FIRST_POS);
    if (v.type() == QVariant::Int) {
        return v.toInt();
    }
    return -1;
}

void MSAEditorState::setFirstPos(int y) {
    stateData[FIRST_POS] = y;
}

float MSAEditorState::getZoomFactor() const {
    QVariant v = stateData.value(ZOOM_FACTOR);
    if (v.type() == QVariant::Double) {
        return v.toDouble();
    }
    return 1.0;
}

void MSAEditorState::setZoomFactor(float zoomFactor) {
    stateData[ZOOM_FACTOR] = zoomFactor;
}

QVariantMap MSAEditorState::saveState(MSAEditor* v) {
    MSAEditorState ss;

    ss.stateData[VIEW_ID]=MSAEditorFactory::ID;

    MAlignmentObject* msaObj = v->getMSAObject();
    if (msaObj) {
        ss.setMSAObjectRef(GObjectReference(msaObj));
    }

    ss.setFont(v->getFont());
    ss.setFirstPos(v->getFirstVisibleBase());

    return ss.stateData;
}

} // namespace U2
