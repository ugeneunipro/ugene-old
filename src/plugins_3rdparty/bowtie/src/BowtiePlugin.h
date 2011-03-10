#ifndef _U2_BOWTIE_PLUGIN_H_
#define _U2_BOWTIE_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Core/AppContext.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Lang/Datatype.h>

#include <QtGui/QMenu>

namespace U2 {

class MSAEditor;
class XMLTestFactory;
class BowtieMSAEditorContext;

class BowtiePlugin : public Plugin {
    Q_OBJECT
public:
	static const QString EBWT_INDEX_TYPE_ID;

	static DataTypePtr EBWT_INDEX_TYPE();

    BowtiePlugin();
    ~BowtiePlugin();

private:
    BowtieMSAEditorContext* ctx;
};
} //namespace

#endif
