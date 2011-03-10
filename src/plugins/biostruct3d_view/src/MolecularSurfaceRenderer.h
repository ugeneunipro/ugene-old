#ifndef _U2_MOLECULAR_SURFACE_RENDERER_H_
#define _U2_MOLECULAR_SURFACE_RENDERER_H_

#include <QtCore/QString>
#include <QtCore/QMap>

namespace U2 {

class MolecularSurface;
class MolecularSurfaceRenderer;

class MolecularSurfaceRendererFactory {
public:
    MolecularSurfaceRendererFactory() { }
    virtual ~MolecularSurfaceRendererFactory() { }
    virtual MolecularSurfaceRenderer* createInstance() = 0;
    static QMap<QString,MolecularSurfaceRendererFactory*> createFactories();
};

#define SURF_RENDERER_FACTORY(c) \
public: \
    static const QString ID; \
class Factory : public MolecularSurfaceRendererFactory { \
public: \
    Factory() { } \
    MolecularSurfaceRenderer* createInstance() { return new c; } \
};

class MolecularSurfaceRenderer {
public:
    MolecularSurfaceRenderer();
    virtual void drawSurface(MolecularSurface& surface) = 0;
};

class DotsRenderer : public MolecularSurfaceRenderer{
public:
    DotsRenderer() {}
    virtual void drawSurface(MolecularSurface& surface);
    SURF_RENDERER_FACTORY(DotsRenderer)
};

class ConvexMapRenderer : public MolecularSurfaceRenderer {
public:
    ConvexMapRenderer() {}
    virtual void drawSurface(MolecularSurface& surface);
    SURF_RENDERER_FACTORY(ConvexMapRenderer)
};



} //namespace

#endif //_U2_MOLECULAR_SURFACE_RENDERER_H_

