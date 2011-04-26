#include "ExpertDiscoveryViewCtx.h"

#include <U2View/ADVUtils.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/AnnotatedDNAViewFactory.h>

#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>


namespace U2{

ExpertDiscoveryViewCtx::ExpertDiscoveryViewCtx(QObject* p):GObjectViewWindowContext(p, AnnotatedDNAViewFactory::ID){
	
}
void ExpertDiscoveryViewCtx::initViewContext(GObjectView* view){

    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(view);
    if(av){
        foreach(ExpertDiscoveryView* v, viewList){
//            v->insertSeqView(av);
        }
    }
//     ADVGlobalAction* a = new ADVGlobalAction(av,
//         QIcon(":query_designer/images/query_designer.png"),
//         tr("Analyze with query schema..."), 50, ADVGlobalActionFlag_AddToAnalyseMenu);
    //connect(a, SIGNAL(triggered()), SLOT(sl_showDialog()));

}

bool ExpertDiscoveryViewCtx::canHandle(GObjectView* v, GObject* obj){
    if(v->getFactoryId() == "ED"){
        return true;
    }
    else{
        return false;
    }
}

void ExpertDiscoveryViewCtx::addView(ExpertDiscoveryView* v){
    viewList.append(v);
}

// void ExpertDiscoveryViewCtx::sl_windowAdded(MWMDIWindow* wind){
// 
//     int a = 1;
//     if(wind->getWindowName()!="Expert Discovery"){
//         foreach(ExpertDiscoveryView* v, viewList){
//             //v->insertSeqView(wind);
//         }
//     }
// }

}//namespace
