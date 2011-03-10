#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QMessageBox>
#include <QtCore/QMap>
#include <QtCore/QFile>

#include <U2Core/GAutoDeleteList.h>
#include <U2Gui/GUIUtils.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVConstants.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/ADVUtils.h>
#include <U2Core/AppContext.h>
#include <U2Algorithm/SecStructPredictAlgRegistry.h>
#include <U2Core/BioStruct3D.h>
#include <U2Core/AnnotationSettings.h>
#include <U2Algorithm/SecStructPredictTask.h>

#include "GorIVPlugin.h"
#include "GorIVAlgTask.h"
#include "gor.h"

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    SecStructPredictPlugin* plug = new SecStructPredictPlugin();
    return plug;
}

SecStructPredictPlugin::SecStructPredictPlugin() : Plugin(tr("GORIV"), tr("GORIV protein secondary structure prediction")) {
    // Register GORIV algorithm
    SecStructPredictAlgRegistry* registry = AppContext::getSecStructPredictAlgRegistry();
    SecStructPredictTaskFactory* taskFactory = new GorIVAlgTask::Factory;
    registry->registerAlgorithm(taskFactory, GorIVAlgTask::taskName);

    //Register GORIV annotation settings
    AnnotationSettingsRegistry* asr =AppContext::getAnnotationsSettingsRegistry();
    AnnotationSettings* as = new AnnotationSettings(GORIV_ANNOTATION_NAME, true, QColor(102,255, 0), true);
    as->nameQuals.append(BioStruct3D::SecStructTypeQualifierName);
    asr->changeSettings(QList<AnnotationSettings*>() << as, false);
}

SecStructPredictPlugin::~SecStructPredictPlugin() {
}


}//namespace

