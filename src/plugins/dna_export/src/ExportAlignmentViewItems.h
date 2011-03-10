#ifndef _U2_EXPORT_ALIGNMENT_VIEW_ITEMS_H_
#define _U2_EXPORT_ALIGNMENT_VIEW_ITEMS_H_

#include <U2Core/global.h>
#include <U2Core/U2Region.h>
#include <U2Gui/ObjectViewModel.h>

namespace U2 {

class ADVSequenceObjectContext;
class AnnotatedDNAView;
class AnnotationSelection;
class Annotation;
class LRegionsSelection;
class MAlignment;
class MSAEditor;

class ExportAlignmentViewItemsController : public GObjectViewWindowContext {
    Q_OBJECT
public:
    ExportAlignmentViewItemsController(QObject* p);


protected:
    virtual void initViewContext(GObjectView* view);
    virtual void buildMenu(GObjectView* v, QMenu* m);
};

class MSAExportContext : public QObject {
    Q_OBJECT;
public:
    MSAExportContext(MSAEditor* e);
    void buildMenu(QMenu* m);

protected slots:
    void sl_exportNucleicMsaToAmino();   

    void updateActions();

private:
    MSAEditor*      editor;
    QAction*        translateMSAAction;    

};

}//namespace

#endif
