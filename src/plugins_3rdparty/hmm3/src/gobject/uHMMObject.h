#ifndef _GB2_HMMER3_UHMM_OBJECT_H_
#define _GB2_HMMER3_UHMM_OBJECT_H_

#include <QtCore/QString>

#include <U2Core/global.h>
#include <U2Core/GObject.h>

#include <hmmer3/hmmer.h>

namespace U2 {

class UHMMObject : public GObject {
    Q_OBJECT
public:
    static const QString        OT_ID;
    static const QString        OT_NAME;
    static const QString        OT_PNAME;
    static const QString        OT_SIGN;
    static const QString        OT_ICON;
    static const GObjectType    UHMM_OT;
    
    UHMMObject( P7_HMM* hmm, const QString& name );
    ~UHMMObject();
    
    const P7_HMM* getHMM() const;
    P7_HMM* takeHMM();
    
    virtual GObject* clone() const;
    
private:
    P7_HMM* hmm;
    
}; // UHMMObject

} // U2

#endif // _GB2_HMMER3_UHMM_OBJECT_H_
