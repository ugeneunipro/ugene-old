#ifndef _U2_OPSTATUS_UTILS_H_
#define _U2_OPSTATUS_UTILS_H_

#include <U2Core/U2OpStatus.h>
#include <U2Core/Task.h>

namespace U2 {

/** 
    Default helper stub for U2OpStatus 
    Note: implementation is not thread safe!
*/
class U2CORE_EXPORT U2OpStatusImpl : public U2OpStatus {
public:
    U2OpStatusImpl() : cancelFlag(false), progress(-1){}

    virtual void setError(const QString & err) {error = err;}
    virtual QString getError() const  {return error;}

    virtual bool hasError() const {return !error.isEmpty();}

    virtual bool isCanceled() const {return cancelFlag != 0;}
    virtual void setCanceled(bool v)  {cancelFlag = v;}

    virtual int getProgress() const {return progress;}
    virtual void setProgress(int v)  {progress = v;}

    virtual QString getStatusDesc() const {return statusDesc;}
    virtual void setStatusDesc(const QString& desc)  {statusDesc = desc;}

private:
    QString error;
    int     cancelFlag;
    int     progress;
    QString statusDesc;
};

/** Default helper stub for U2OpStatus */
class U2CORE_EXPORT U2OpTaskStatus: public U2OpStatus {
public:
    U2OpTaskStatus(TaskStateInfo* i) : ti(i)  {}
    virtual void setError(const QString & err)  {ti->setError(err);}
    virtual QString getError() const  {return ti->getError();}
    virtual bool hasError() const {return ti->hasErrors();}

    virtual bool isCanceled() const {return ti->cancelFlag != 0;}
    virtual void setCanceled(bool v)  {ti->cancelFlag = v;};

    virtual int getProgress() const  {return ti->progress;}
    virtual void setProgress(int v)  {ti->progress = v;}

    virtual QString getStatusDesc() const {return ti->getStateDesc();}
    virtual void setStatusDesc(const QString& desc)  {ti->setStateDesc(desc);}

private:
    TaskStateInfo* ti;
};


}// namespace

#endif
