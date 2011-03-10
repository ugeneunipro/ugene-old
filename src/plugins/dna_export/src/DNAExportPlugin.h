#ifndef _U2_DNA_EXPORT_PLUGIN_H_
#define _U2_DNA_EXPORT_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Core/ServiceModel.h>

namespace U2 {

class ExportProjectViewItemsContoller;
class ExportSequenceViewItemsController;
class ExportAlignmentViewItemsController;

class DNAExportPlugin : public Plugin {
    Q_OBJECT
public:
    DNAExportPlugin();

};

class DNAExportService: public Service {
    Q_OBJECT
public:
    DNAExportService();

protected:
    virtual void serviceStateChangedCallback(ServiceState oldState, bool enabledStateChanged);

    ExportProjectViewItemsContoller*    projectViewController;
    ExportSequenceViewItemsController*  sequenceViewController;
    ExportAlignmentViewItemsController* alignmentViewController;

};


} //namespace

#endif
