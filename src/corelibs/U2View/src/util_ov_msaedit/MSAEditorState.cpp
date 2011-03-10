#include "MSAEditorState.h"
#include "MSAEditor.h"
#include "MSAEditorFactory.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/AnnotationTableObject.h>
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

GObjectReference MSAEditorState::getMSAObject() const {
    return stateData.contains(MSA_OBJ) ? stateData[MSA_OBJ].value<GObjectReference>() : GObjectReference();
}

void MSAEditorState::setMSAObject(const GObjectReference& ref) {
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
        ss.setMSAObject(GObjectReference(msaObj));
    }

    ss.setFont(v->getFont());
    ss.setFirstPos(v->getFirstVisibleBase());

    return ss.stateData;
}


} // namespace
