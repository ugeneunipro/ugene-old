#ifndef _U2_AUTODELETE_LIST_H_
#define _U2_AUTODELETE_LIST_H_

#include <QtCore/QObject>

namespace U2 {

template <class T>
class GAutoDeleteList : public QObject {
public:
    GAutoDeleteList(QObject* p = NULL) : QObject(p){}
    virtual ~GAutoDeleteList() {qDeleteAll(qlist);}
    QList<T*> qlist;
};

//todo: move to separate header
template <class T>
class gauto_array {
public:
    gauto_array(T* p = NULL) : data(p){}
    ~gauto_array() {
        delete[] data;
    }
    T* get() const {return data;}
    T* data;
};

}//namespace

#endif
