#ifndef _U2_FILE_DBI_PLUGIN_H_
#define _U2_FILE_DBI_PLUGIN_H_

#include <U2Core/PluginModel.h>

namespace U2 {


class FileDbiPlugin : public Plugin {
    Q_OBJECT
public:
    FileDbiPlugin();
private slots:
    void test();
};


} //namespace

#endif
