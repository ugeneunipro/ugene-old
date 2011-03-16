#ifndef _U2_BIOSTRUCT3D_RENDERER_H_
#define _U2_BIOSTRUCT3D_RENDERER_H_

#include <QtCore/QString>
#include <QtCore/QMap>

namespace U2 { 

class BioStruct3D;
class BioStruct3DGLRenderer;
class BioStruct3DGLRendererFactory;
class BioStruct3DGLWidget;
class BioStruct3DColorScheme;

//! Singleton regisrtry for renderers fabrics.
class BioStruct3DGLRendererRegistry {
public:
    //! @return Default renderer factory name.
    static const QString defaultFactoryName();

    //! @return List of all factories names.
    static const QList<QString> factoriesNames();

    //! @return Concreete factory by name.
    static const BioStruct3DGLRendererFactory* getFactory(const QString &name);

    //! @return Constructed renderer by factory name.
    static BioStruct3DGLRenderer* createRenderer(const QString &name, const BioStruct3D &bs, const BioStruct3DColorScheme* s, const QList<int> &shownModels, const BioStruct3DGLWidget *w);

private:
    //! Hidden constructor. Called by getInstance()
    BioStruct3DGLRendererRegistry();

    //! Returns singleton instance of registry.
    static BioStruct3DGLRendererRegistry* getInstance();

    //! Registers all render factories.
    void registerFactories();

private:
    QMap<QString, BioStruct3DGLRendererFactory*> factories;
};  // class BioStruct3DGLRendererRegistry


//! BioStruct3DGLRenderer abstract factory
class BioStruct3DGLRendererFactory {
public:
    //! @return Concreete renderer
    virtual BioStruct3DGLRenderer* createInstance(const BioStruct3D& bs, const BioStruct3DColorScheme* s, const QList<int> &shownModels, const BioStruct3DGLWidget *w) const = 0;
};  // class BioStruct3DGLRendererFactory


#define RENDERER_FACTORY(c) \
public: \
    static const QString ID; \
class Factory : public BioStruct3DGLRendererFactory { \
public: \
    Factory() { } \
    BioStruct3DGLRenderer* createInstance(const BioStruct3D& bs, const BioStruct3DColorScheme* s, const QList<int> &sm, const BioStruct3DGLWidget *w) const { return new c(bs,s,sm,w); } \
};


//! Abstract biological 3D structure OpenGL renderer
class BioStruct3DGLRenderer {
protected:
    BioStruct3DGLRenderer(const BioStruct3D &biostruct, const BioStruct3DColorScheme *scheme, const QList<int> &shownModels, const BioStruct3DGLWidget *widget);

public:
    virtual ~BioStruct3DGLRenderer() {};

    //! Visualizes macromolecule.
    virtual void drawBioStruct3D() = 0;

    //! Used to update current color scheme, whenever it is changed.
    virtual void updateColorScheme() = 0;

    //! Used to update shown models list, whenever it is changed.
    virtual void updateShownModels() = 0;

    //! Sets new color scheme.
    void setColorScheme(const BioStruct3DColorScheme* s);

    //! @returns current color scheme.
    const BioStruct3DColorScheme *getColorScheme() const { return colorScheme; }

    //! @returns shown models indexes list reference.
    //! indexes are just index numbers of models, NOT modelIds
    QList<int>& getShownModelsIndexes() { return shownModels; }


protected:
    const BioStruct3D& bioStruct;
    const BioStruct3DColorScheme* colorScheme;

protected:
    QList<int> shownModels;

protected:
    const BioStruct3DGLWidget* glWidget;

    unsigned int bigDL;
};


} //namespace

#endif // _U2_BIOSTRUCT3D_RENDERER_H_
