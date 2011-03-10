#include "BioStruct3DColorScheme.h"
#include "BioStruct3DGLRender.h"
#include "BallAndStickGLRenderer.h"
#include "TubeGLRenderer.h"
#include "VanDerWaalsGLRenderer.h"
#include "WormsGLRenderer.h"

namespace U2 { 

#define REGISTER_FACTORY(c) \
    map.insert(c::ID, new c::Factory)

QMap<QString,BioStruct3DGLRendererFactory*> BioStruct3DGLRendererFactory::createFactories()
{
    QMap<QString,BioStruct3DGLRendererFactory*> map;
    REGISTER_FACTORY(BallAndStickGLRenderer);
    REGISTER_FACTORY(TubeGLRenderer);
    REGISTER_FACTORY(VanDerWaalsGLRenderer);
    REGISTER_FACTORY(WormsGLRenderer);
    return map;
}

const QString BioStruct3DGLRendererFactory::defaultFactoryName()
{
    return WormsGLRenderer::ID;
}



void BioStruct3DGLRenderer::updateColorScheme()
{
    
}

BioStruct3DGLRenderer::BioStruct3DGLRenderer( const BioStruct3D& _bioStruct, const BioStruct3DColorScheme* _s )  :  
    bioStruct(_bioStruct), colorScheme(_s), glWidget(_s->getBioStruct3DGlWidget())
{
    bigDL = -1;
}

void BioStruct3DGLRenderer::setColorScheme( const BioStruct3DColorScheme* s )
{
     colorScheme = s;
     updateColorScheme();
}

} //namespace
