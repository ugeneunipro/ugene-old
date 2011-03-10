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

    GObjectReference getMSAObject() const;

    void setMSAObject(const GObjectReference& ref);

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
