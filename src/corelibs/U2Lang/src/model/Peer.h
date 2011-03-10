#ifndef _U2_WORKFLOW_PEER_H_
#define _U2_WORKFLOW_PEER_H_

#include <U2Lang/Descriptor.h>

namespace U2 {

namespace Workflow {

/**
* wrapper throw 'void*'
* use as a base class to store any pointer in your class
*/
class U2LANG_EXPORT Peer {
public:
    Peer() : peer(NULL) {}
    virtual ~Peer() {}

    // casting to whatever you want
    template<typename T> inline T* castPeer() const {
        return (T*)(peer);
    }

    void* getPeer() const {return peer;}
    void setPeer(void* p) { peer = p; }

protected:
    // any pointer
    void* peer;

}; // Peer

} // Workflow

} // U2

#endif // _U2_WORKFLOW_PEER_H_
