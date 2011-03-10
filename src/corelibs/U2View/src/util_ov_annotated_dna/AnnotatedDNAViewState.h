#ifndef _U2_ANNOTATED_DNA_VIEW_STATE_H_
#define _U2_ANNOTATED_DNA_VIEW_STATE_H_

#include <U2Core/U2Region.h>
#include <U2Core/GObject.h>
#include <QtCore/QVariant>

namespace U2 {

class AnnotatedDNAView;

class U2VIEW_EXPORT AnnotatedDNAViewState {
public:
    AnnotatedDNAViewState();
    AnnotatedDNAViewState(const QVariantMap& _stateData) : stateData(_stateData){}

    static QVariantMap saveState(AnnotatedDNAView* v);

    bool isValid() const;

    void setSequenceObjects(const QList<GObjectReference>& objs, const QVector<U2Region>& selections);

    QList<GObjectReference> getSequenceObjects() const;

    QVector<U2Region> getSequenceSelections() const;

    QList<GObjectReference> getAnnotationObjects() const;

    void setAnnotationObjects(const QList<GObjectReference>& objs);

    QVariantMap stateData;
};




} // namespace

#endif
