#ifndef  _NEIGHBORJOIN_PLUGIN_H_
#define  _NEIGHBORJOIN_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Core/AppContext.h>
#include <U2Gui/ObjectViewModel.h>

#include <QtGui/QMenu>

namespace U2 {


class PhylipPlugin : public Plugin {
	Q_OBJECT
public:
    static const QString PHYLIP_NEIGHBOUR_JOIN;
    PhylipPlugin();
	~PhylipPlugin();
};


} //namespace

#endif
