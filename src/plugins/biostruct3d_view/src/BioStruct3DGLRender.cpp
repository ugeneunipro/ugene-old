#include "BioStruct3DColorScheme.h"
#include "BioStruct3DGLRender.h"
#include "BallAndStickGLRenderer.h"
#include "TubeGLRenderer.h"
#include "VanDerWaalsGLRenderer.h"
#include "WormsGLRenderer.h"

namespace U2 {

/* class BioStruct3DGLRendererRegistry */
const QString BioStruct3DGLRendererRegistry::defaultFactoryName() {
    return WormsGLRenderer::ID;
}

const QList<QString> BioStruct3DGLRendererRegistry::factoriesNames() {
    return getInstance()->factories.keys();
}

const BioStruct3DGLRendererFactory* BioStruct3DGLRendererRegistry::getFactory(const QString &name) {
    return getInstance()->factories.value(name, 0);
}

BioStruct3DGLRenderer* BioStruct3DGLRendererRegistry::createRenderer(const QString &name, const BioStruct3D &bs, const BioStruct3DColorScheme* s, const QList<int> &sm, const BioStruct3DGLWidget *w) {
    const BioStruct3DGLRendererFactory *fact = getFactory(name);

    if (fact) {
        return fact->createInstance(bs,s,sm,w);
    }

    return 0;
}

BioStruct3DGLRendererRegistry::BioStruct3DGLRendererRegistry() {
    registerFactories();
}

BioStruct3DGLRendererRegistry* BioStruct3DGLRendererRegistry::getInstance() {
    static BioStruct3DGLRendererRegistry *reg = new BioStruct3DGLRendererRegistry();
    return reg;
}

#define REGISTER_FACTORY(c) factories.insert(c::ID, new c::Factory)
void BioStruct3DGLRendererRegistry::registerFactories() {
    REGISTER_FACTORY(BallAndStickGLRenderer);
    REGISTER_FACTORY(TubeGLRenderer);
    REGISTER_FACTORY(VanDerWaalsGLRenderer);
    REGISTER_FACTORY(WormsGLRenderer);
}


/* class BioStruct3DGLRenderer */
BioStruct3DGLRenderer::BioStruct3DGLRenderer( const BioStruct3D& _bioStruct, const BioStruct3DColorScheme* _s, const QList<int> &_shownModels, const BioStruct3DGLWidget *widget)
        : bioStruct(_bioStruct), colorScheme(_s), shownModels(_shownModels), glWidget(widget)
{
    bigDL = -1;
}

void BioStruct3DGLRenderer::setColorScheme( const BioStruct3DColorScheme* s )
{
     colorScheme = s;
     updateColorScheme();
}

} //namespace
