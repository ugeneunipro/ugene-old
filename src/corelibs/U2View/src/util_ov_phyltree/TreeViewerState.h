#ifndef _U2_TREE_VIEWER_STATE_H_
#define _U2_TREE_VIEWER_STATE_H_

#include <U2Core/U2Region.h>
#include <U2Core/GObject.h>

#include <QtCore/QVariant>

namespace U2 {

class TreeViewer;

class U2VIEW_EXPORT TreeViewerState {
public:
    TreeViewerState(){}
    TreeViewerState(const QVariantMap& _stateData) : stateData(_stateData){}

    static QVariantMap saveState(TreeViewer* v);

    bool isValid() const;

    GObjectReference getPhyObject() const;
    void setPhyObject(const GObjectReference& ref);

    qreal getZoom() const;
    void setZoom(qreal s);

    QTransform getTransform() const;
    void setTransform(const QTransform& m);

    QVariantMap stateData;
};

} // namespace

#endif
