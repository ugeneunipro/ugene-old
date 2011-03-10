#ifndef _U2_CORETESTS_H_
#define _U2_CORETESTS_H_

#include <U2Core/PluginModel.h>

namespace U2 {

class CoreTests : public Plugin {
    Q_OBJECT
public:
    CoreTests();
    virtual ~CoreTests();
private:
    void registerFactories();
};

}//namespace

#endif
