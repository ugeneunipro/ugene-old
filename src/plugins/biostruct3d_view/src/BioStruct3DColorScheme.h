#ifndef _U2_BIOSTRUCT3D_COLORSCHEME_H
#define _U2_BIOSTRUCT3D_COLORSCHEME_H

#include <U2Core/BioStruct3D.h>
#include <U2Core/U2Region.h>
#include <QtGui/QColor>
#include <QtCore/QMap>
#include <QtCore/QHash>
#include <QtCore/QPair>
#include <QtCore/QLinkedList>

#include "GraphicUtils.h"

namespace U2 { 

class BioStruct3DGLWidget;
class BioStruct3DColorScheme;
class BioStruct3DColorSchemeFactory;
class LRegionsSelection;

//!  Abstract factory for BioStruct3DColorScheme
/*!
A more elaborate class description.
*/
class BioStruct3DColorSchemeFactory {
public:
    //! Constructor.
    BioStruct3DColorSchemeFactory() { }
    virtual ~BioStruct3DColorSchemeFactory() { }
    virtual BioStruct3DColorScheme* createInstance(const BioStruct3DGLWidget* widget) = 0;
    //! Method creates factories 
    /*! 
        It returns map of registered BioStruct3DColorScheme factories associated with object name
    */
    static QMap<QString,BioStruct3DColorSchemeFactory*> createFactories();
    //! Method returns default BioStruct3DColorSchemeFactory name
    static const QString defaultFactoryName();
};

#define COLOR_SCHEME_FACTORY(c) \
public: \
    static const QString schemeName; \
    class Factory : public BioStruct3DColorSchemeFactory { \
    public: \
    Factory() { } \
    BioStruct3DColorScheme* createInstance(const BioStruct3DGLWidget* widget) { return new c(widget); } \
    };

//! Abstract BioStruct3DColorScheme
/*!
    Color scheme determines 3d structure coloring style. For example, we could use colors to mark chemical
    elements or secondary structure.
*/
class BioStruct3DColorScheme : public QObject {
public:
    BioStruct3DColorScheme(const BioStruct3DGLWidget* widget);

    Color4f getAtomColor(const SharedAtom& atom) const;
    void setSelectionColor(QColor color);
    void updateSelectionRegion(int chainID, const QVector<U2Region>& added, const QVector<U2Region>& removed);
    const BioStruct3DGLWidget* getBioStruct3DGlWidget() const { return glWidget; }

    bool isInSelection(const SharedAtom& atom) const;

    void setUnselectedShadingLevel(float shading);

protected:
    virtual Color4f getSchemeAtomColor(const SharedAtom& atom) const;

protected:
    const BioStruct3DGLWidget* glWidget;
    Color4f defaultAtomColor;
    Color4f selectionColor;

    QMultiMap<int, int> selection;

    void addToSelection(int chainId, int residueId);
    void removeFromSelection(int chainId, int residueId);

private:
    float unselectedShading;
};  // class BioStruct3DColorScheme

class ChemicalElemColorScheme : public BioStruct3DColorScheme {
    QHash<int, Color4f> elementColorMap;
    ChemicalElemColorScheme(const BioStruct3DGLWidget* widget);
    virtual Color4f getSchemeAtomColor(const SharedAtom& atom) const;
public:
    COLOR_SCHEME_FACTORY(ChemicalElemColorScheme)
};

class ChainsColorScheme : public BioStruct3DColorScheme {
    QMap<int, Color4f> chainColorMap;
    ChainsColorScheme(const BioStruct3DGLWidget* widget);
    virtual Color4f getSchemeAtomColor(const SharedAtom& atom) const;
public:
    COLOR_SCHEME_FACTORY(ChainsColorScheme)
};


class SecStructColorScheme : public BioStruct3DColorScheme {
    struct MolStructs {
        QHash<int, QByteArray> strucResidueTable;
    };
    QMap<QByteArray, Color4f> secStrucColorMap;
    QMap<int, MolStructs> molMap;
    SecStructColorScheme(const BioStruct3DGLWidget* widget);
    virtual Color4f getSchemeAtomColor(const SharedAtom& atom) const;
public:
    COLOR_SCHEME_FACTORY(SecStructColorScheme)
};




} //namespace

#endif //_U2_BIOSTRUCT3D_COLORSCHEME_H_
