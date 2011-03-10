#include "TreeViewerState.h"
#include "TreeViewer.h"
#include "TreeViewerFactory.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/PhyTreeObject.h>
#include <U2Core/DNASequenceSelection.h>

namespace U2 {

#define VIEW_ID     QString("view_id")
#define PHY_OBJ     QString("phy_obj_ref")
#define ZOOM        QString("zoom")
#define TRANSFORM   QString("transform")

bool TreeViewerState::isValid() const {
    return stateData.value(VIEW_ID) == TreeViewerFactory::ID;
}

GObjectReference TreeViewerState::getPhyObject() const {
    return stateData.contains(PHY_OBJ) ? stateData[PHY_OBJ].value<GObjectReference>() : GObjectReference();
}

void TreeViewerState::setPhyObject(const GObjectReference& ref) {
    stateData[PHY_OBJ] = QVariant::fromValue<GObjectReference>(ref);
}


qreal TreeViewerState::getZoom() const {

    QVariant v = stateData.value(ZOOM);
    if (v.isValid()) {
        return v.value<qreal>();
    }
    else {
        return 1.0f;
    }
}

void TreeViewerState::setZoom(qreal s) {

    stateData[ZOOM] = s;
}

QTransform TreeViewerState::getTransform() const {

    QVariant v = stateData.value(TRANSFORM);
    if (v.type() == QVariant::Transform) {
        return v.value<QTransform>();
    }
    QTransform t;
    return t;
}

void TreeViewerState::setTransform(const QTransform& m) {

    stateData[TRANSFORM] = m;
}


QVariantMap TreeViewerState::saveState(TreeViewer* v) {
    TreeViewerState ss;

    ss.stateData[VIEW_ID] = TreeViewerFactory::ID;

    PhyTreeObject* phyObj = v->getPhyObject();
    if (phyObj) {
        ss.setPhyObject(GObjectReference(phyObj));
    }

    ss.setZoom(v->getZoom());
    ss.setTransform(v->getTransform());

    ss.stateData.unite(v->getSettingsState());

    return ss.stateData;
}

} // namespace
