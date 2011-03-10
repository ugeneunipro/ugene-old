#ifndef _U2_PSIPRED_PLUGIN_H_
#define _U2_PSIPRED_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Core/AnnotationData.h>


namespace U2 {

class XMLTestFactory;

class PsipredPlugin : public Plugin {
	Q_OBJECT
public:
    PsipredPlugin();
	~PsipredPlugin();

private:
    GObjectViewWindowContext* viewCtx;
};


} //namespace

#endif // _U2_PSIPRED_PLUGIN_H_


