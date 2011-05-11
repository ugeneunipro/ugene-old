#pragma once

#include "ExpertDiscoveryView.h"

#include <U2Gui/ObjectViewModel.h>

namespace U2{

class ExpertDiscoveryViewCtx: public GObjectViewWindowContext{
	Q_OBJECT

public:
    ExpertDiscoveryViewCtx(QObject* p);
    virtual bool canHandle(GObjectView* v, GObject* obj);

    void addView(ExpertDiscoveryView* v);

protected:
    virtual void initViewContext(GObjectView* view);

// protected slots:
//     virtual void sl_windowAdded(MWMDIWindow*);

private:

    

    QList<ExpertDiscoveryView*> viewList;
};

}//namespace