#include "ProjectViewImpl.h"
#include <U2Gui/ObjectViewModel.h>

#include <U2View/SimpleTextObjectView.h>
#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/MSAEditorFactory.h>
#include <U2View/UIndexViewerFactory.h>
#include <U2View/TreeViewerFactory.h>

namespace U2 {

void ProjectViewImpl::registerBuiltInObjectViews() {
	GObjectViewFactoryRegistry* reg = AppContext::getObjectViewFactoryRegistry();
	
	GObjectViewFactory* f = new SimpleTextObjectViewFactory();
	reg->registerGObjectViewFactory(f);

	f =  new AnnotatedDNAViewFactory();
	reg->registerGObjectViewFactory(f);

    f = new MSAEditorFactory();
    reg->registerGObjectViewFactory(f);

    f = new UIndexViewerFactory();
    reg->registerGObjectViewFactory( f );

    f = new TreeViewerFactory();
    reg->registerGObjectViewFactory(f);
}

void ProjectViewImpl::unregisterBuiltInObjectViews() {
	GObjectViewFactoryRegistry* reg = AppContext::getObjectViewFactoryRegistry();
	
	GObjectViewFactory* f = reg->getFactoryById(SimpleTextObjectViewFactory::ID);
	assert(f!=NULL);
	reg->unregisterGObjectViewFactory(f);
	delete f;

	f = reg->getFactoryById(AnnotatedDNAViewFactory::ID);
	assert(f!=NULL);
	reg->unregisterGObjectViewFactory(f);
	delete f;

    f = reg->getFactoryById(MSAEditorFactory::ID);
    assert(f!=NULL);
    reg->unregisterGObjectViewFactory(f);
    delete f;

    f = reg->getFactoryById( UIndexViewerFactory::ID );
    assert( NULL != f );
    reg->unregisterGObjectViewFactory( f );
    delete f;

    f = reg->getFactoryById(TreeViewerFactory::ID);
    assert(f != NULL);
    reg->unregisterGObjectViewFactory(f);
    delete f;
}

}//namespace

