#ifndef _U2_OPSTATUS_H_
#define _U2_OPSTATUS_H_

#include <U2Core/global.h>

namespace U2 {

/**
    Operation status.
    If operation failed, the reason is written in 'error' field
    If operation hangs, it can be canceled with 'cancelFlag'
*/
class U2CORE_EXPORT U2OpStatus {
public:
    virtual ~U2OpStatus(){}
    
    virtual void setError(const QString & err) = 0;
    virtual QString getError() const  = 0;
    virtual bool hasError() const = 0;

    virtual bool isCanceled() const = 0;
    virtual void setCanceled(bool v)  = 0;

    virtual bool isCoR() const  {return isCanceled() || hasError();}

    /** Returns progress value in percents. Special value '-1' is used to indicate 'No info' progress state */
    virtual int getProgress() const = 0;
    virtual void setProgress(int v)  = 0;

    virtual QString getStatusDesc() const = 0;
    virtual void setStatusDesc(const QString& desc)  = 0;
};

} //namespace

#endif
