#ifndef _U2_MOLECULAR_SURFACE_RENDERER_H_
#define _U2_MOLECULAR_SURFACE_RENDERER_H_

#include <QtCore/QString>
#include <QtCore/QMap>

namespace U2 {

class MolecularSurface;
class MolecularSurfaceRenderer;
class MolecularSurfaceRendererFactory;

//! Singleton regisrtry for molecular surface(ms) renderers fabrics.
class MolecularSurfaceRendererRegistry {
public:
    //! @return Default ms renderer factory name.
    static const QString defaultFactoryName();

    //! @return List of all factories names.
    static const QList<QString> factoriesNames();

    //! @return Concreete factory by name.
    static const MolecularSurfaceRendererFactory* getFactory(const QString &name);

    //! @return Constructed ms renderer by factory name.
    static MolecularSurfaceRenderer* createMSRenderer(const QString &name);

private:
    //! Hidden constructor. Called by getInstance()
    MolecularSurfaceRendererRegistry();

    //! Returns singleton instance of registry.
    static MolecularSurfaceRendererRegistry* getInstance();

    //! Registers all ms render factories.
    void registerFactories();

private:
    QMap<QString, MolecularSurfaceRendererFactory*> factories;
};  // class MolecularSurfaceRendererRegistry


//! MolecularSurfaceRenderer abstract factory
class MolecularSurfaceRendererFactory {
public:
    virtual MolecularSurfaceRenderer* createInstance() const = 0;
};  // class MolecularSurfaceRendererFactory

#define SURF_RENDERER_FACTORY(c) \
public: \
    static const QString ID; \
class Factory : public MolecularSurfaceRendererFactory { \
public: \
    MolecularSurfaceRenderer* createInstance() const { return new c; } \
};

class MolecularSurfaceRenderer {
protected:
    MolecularSurfaceRenderer() {};

public:
    virtual ~MolecularSurfaceRenderer() {};

    virtual void drawSurface(MolecularSurface& surface) = 0;
};

class DotsRenderer : public MolecularSurfaceRenderer {
private:
    DotsRenderer() : MolecularSurfaceRenderer() {}

public:
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

