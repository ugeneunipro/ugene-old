#ifndef _U2_BIOSTRUCT3D_RENDERER_H_
#define _U2_BIOSTRUCT3D_RENDERER_H_

#include <QtCore/QString>
#include <QtCore/QMap>

namespace U2 { 

class BioStruct3D;
class BioStruct3DGLRenderer;
class BioStruct3DGlWidget;

//! BioStruct3DGLRenderer abstract factory
class BioStruct3DGLRendererFactory {
public:
    /*!
    * Constructor.
    */
    BioStruct3DGLRendererFactory() { }
    /*!
    * Destructor.
    */
    virtual ~BioStruct3DGLRendererFactory() { }
    /*!
    * @return Constructed glRenderer
    * @param bs Corresponding BioStruct3D.
    */
    virtual BioStruct3DGLRenderer* createInstance(const BioStruct3D& bs, const BioStruct3DColorScheme* s) = 0;
    /*!
    * @return Existing renderer factories associated with their names
    */
    static QMap<QString,BioStruct3DGLRendererFactory*> createFactories();
    /*!
    * @return Default renderer factory name
    */
    static const QString defaultFactoryName();
};

#define RENDERER_FACTORY(c) \
public: \
    static const QString ID; \
class Factory : public BioStruct3DGLRendererFactory { \
public: \
    Factory() { } \
    BioStruct3DGLRenderer* createInstance(const BioStruct3D& bs, const BioStruct3DColorScheme* s) { return new c(bs,s); } \
};

//! Abstract biological 3D structure OpenGL renderer
class BioStruct3DGLRenderer {

protected:
    const BioStruct3D& bioStruct;
    const BioStruct3DColorScheme* colorScheme;
    const BioStruct3DGLWidget* glWidget;

    unsigned int bigDL;

public:
    /*!
    * Constructor.
    */
    BioStruct3DGLRenderer(const BioStruct3D& _bioStruct, const BioStruct3DColorScheme* _s); 
    /*!
    * Destructor.
    */
    virtual ~BioStruct3DGLRenderer() { }
    /*!
    * Visualizes macromolecule.
    */
    virtual void drawBioStruct3D() = 0;
    /*!
    * Used to update current color scheme, whenever it is changed.
    */
    virtual void updateColorScheme();
    /*!
    * Sets new color scheme.
    */
    void setColorScheme(const BioStruct3DColorScheme* s);
};


} //namespace

#endif // _U2_BIOSTRUCT3D_RENDERER_H_
