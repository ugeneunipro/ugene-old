
#ifndef _U2_SERIALIZABLE_H_
#define _U2_SERIALIZABLE_H_

#include <U2Core/global.h>

namespace U2 {

class U2REMOTE_EXPORT Serializable {
public:
    virtual ~Serializable();
    virtual QVariant serialize()const = 0;
    virtual bool deserialize( const QVariant & data ) = 0;
    
}; // Serializable

/*
 * this interface is same as Serializable, but it must provide human-readable serialization
 */
class U2REMOTE_EXPORT HumanSerializable {
public:
    virtual ~HumanSerializable();
    virtual QString serialize() const = 0;
    virtual bool deserialize( const QString & data ) = 0;
    
}; // HumanSerializable

} // U2

#endif // _U2_SERIALIZABLE_H_
