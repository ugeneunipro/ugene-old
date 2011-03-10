#ifndef _U2_ANNOTATION_COLLOCATOR_PLUGIN_H_
#define _U2_ANNOTATION_COLLOCATOR_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Gui/ObjectViewModel.h>

#include <QtCore/QMap>
#include <QtGui/QAction>


namespace U2 {

class XMLTestFactory;

class AnnotatorPlugin : public Plugin {
    Q_OBJECT
public:
    AnnotatorPlugin();

private:
    GObjectViewWindowContext* viewCtx;
};

class AnnotatorViewContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    AnnotatorViewContext(QObject* p);

protected slots:
    void sl_showCollocationDialog();

protected:
    virtual void initViewContext(GObjectView* view);
};

class AnnotatorTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};

} //namespace
#endif
