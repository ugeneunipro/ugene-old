#include "BioStruct3DColorScheme.h"

#include <U2Core/AppContext.h>
#include <U2Core/BioStruct3DObject.h>
#include <U2Core/AnnotationSettings.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/AnnotationTableObject.h>

#include <QtCore/QObject>
#include <QtAlgorithms>

namespace U2 { 

/* class BioStruct3DColorSchemeRegistry */
const QString BioStruct3DColorSchemeRegistry::defaultFactoryName() {
    return SecStructColorScheme::schemeName;
}

const QList<QString> BioStruct3DColorSchemeRegistry::factoriesNames() {
    return getInstance()->factories.keys();
}

const BioStruct3DColorSchemeFactory* BioStruct3DColorSchemeRegistry::getFactory(const QString &name) {
    return getInstance()->factories.value(name, 0);
}

BioStruct3DColorScheme* BioStruct3DColorSchemeRegistry::createColorScheme(const QString &name, const BioStruct3DObject *biostruct) {
    const BioStruct3DColorSchemeFactory *fact = getFactory(name);

    if (fact) {
        return fact->createInstance(biostruct);
    }

    return 0;
}

BioStruct3DColorSchemeRegistry::BioStruct3DColorSchemeRegistry() {
    registerFactories();
}

BioStruct3DColorSchemeRegistry* BioStruct3DColorSchemeRegistry::getInstance() {
    static BioStruct3DColorSchemeRegistry *reg = new BioStruct3DColorSchemeRegistry();
    return reg;
}

#define REGISTER_FACTORY(c) factories.insert(c::schemeName, new c::Factory)
void BioStruct3DColorSchemeRegistry::registerFactories() {
    REGISTER_FACTORY(ChainsColorScheme);
    REGISTER_FACTORY(SecStructColorScheme);
    REGISTER_FACTORY(ChemicalElemColorScheme);
}


const QString ChainsColorScheme::schemeName(QObject::tr("chain colors"));
const QString ChemicalElemColorScheme::schemeName(QObject::tr("chemical element colors"));
const QString SecStructColorScheme::schemeName(QObject::tr("secondary structure colors"));


/*class BioStruct3DColorScheme */

BioStruct3DColorScheme::BioStruct3DColorScheme(const BioStruct3DObject *biostruct)
        : defaultAtomColor(0.25f, 0.25f, 0.25f), selectionColor(1.0f, 1.0f, 0),
          selection(biostruct->getBioStruct3D()),
          unselectedShading(0.0)
{}

Color4f BioStruct3DColorScheme::getAtomColor(const SharedAtom& atom) const {
    Color4f c;

    if (isInSelection(atom)) {
        c = selectionColor;
    }
    else {
        c = getSchemeAtomColor(atom);
        if (!selection.isEmpty() && unselectedShading > 0.0) {  // dim unselected
            c[3] *= (1.0 - unselectedShading);
        }
    }

    return c;
}

void BioStruct3DColorScheme::setSelectionColor(QColor color) {
    this->selectionColor = color;
}

void BioStruct3DColorScheme::setUnselectedShadingLevel(float shading) {
    assert(shading >= 0.0 && shading <= 1.0);
    unselectedShading = shading;
}

void BioStruct3DColorScheme::updateSelectionRegion(int chainId, const QVector<U2Region>& added, const QVector<U2Region>& removed) {
    selection.update(chainId, added, removed);
}

bool BioStruct3DColorScheme::isInSelection(const SharedAtom &atom) const {
   return selection.inSelection(atom->chainIndex, atom->residueIndex);
}

Color4f BioStruct3DColorScheme::getSchemeAtomColor(const SharedAtom&) const {
    return defaultAtomColor;
}

///////////////////////////////////////////////////////////////////////////////////////////
// ChemicalElemColorScheme

Color4f ChemicalElemColorScheme::getSchemeAtomColor( const SharedAtom& a ) const
{
    Color4f color;
    if (elementColorMap.contains(a->atomicNumber)) {
        return elementColorMap.value(a->atomicNumber);
    } else {
        return defaultAtomColor;
    }

}

ChemicalElemColorScheme::ChemicalElemColorScheme(const BioStruct3DObject *biostruct)
        : BioStruct3DColorScheme(biostruct)
{
    //CPK colors

    elementColorMap.insert(1, Color4f(1.0f, 1.0f, 1.0f));
    elementColorMap.insert(6, Color4f(0.8f, 0.8f, 0.8f));
    elementColorMap.insert(7, Color4f(0.7f, 0.7f, 1.0f));
    elementColorMap.insert(8, Color4f(0.95f, 0.0f , 0.0f));
    elementColorMap.insert(11, Color4f(0.0f, 0.0f , 1.0f));
    elementColorMap.insert(12, Color4f(0.16f, 0.5f , 0.16f));
    elementColorMap.insert(15, Color4f(1.0f, 0.63f , 0.0f));
    elementColorMap.insert(16, Color4f(1.0f, 0.8f , 0.2f));
    elementColorMap.insert(17, Color4f(0.0f, 1.0f , 0.0f));
    elementColorMap.insert(20, Color4f(0.5f, 0.5f , 0.5f));
    elementColorMap.insert(26, Color4f(1.0f, 0.63f , 0.0f));
    elementColorMap.insert(30, Color4f(0.63f, 0.2f , 0.2f));
    elementColorMap.insert(35, Color4f(0.63f, 0.2f , 0.2f));

}


/* class ChainsColorScheme : public BioStruct3DColorScheme */

const QMap<int, QColor> ChainsColorScheme::getChainColors(const BioStruct3DObject *biostruct) {
    QMap<int, QColor> colorMap;
    AnnotationSettingsRegistry* asr = AppContext::getAnnotationsSettingsRegistry();

    // bug-2857: GObject relations shoud be used
    Document *doc = biostruct->getDocument();
    foreach (GObject* obj, doc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE) ) {
        AnnotationTableObject* ao = qobject_cast<AnnotationTableObject*>(obj);
        assert(ao);

        foreach(Annotation* a, ao->getAnnotations()) {
            QString name = a->getAnnotationName();
            if (name.startsWith(BioStruct3D::MoleculeAnnotationTag)) {
                bool Ok = false;
                int chainId = a->findFirstQualifierValue(BioStruct3D::ChainIdQualifierName).toInt(&Ok);
                assert(Ok && chainId != 0);

                AnnotationSettings* as = asr->getAnnotationSettings(name);
                colorMap.insert(chainId, as->color);
            }
        }
    }

    return colorMap;
}

ChainsColorScheme::ChainsColorScheme(const BioStruct3DObject *biostruct)
        : BioStruct3DColorScheme(biostruct)
{
    const QMap<int, QColor> chainColors = getChainColors(biostruct);
    Q_ASSERT(!chainColors.empty());
    QMapIterator<int, QColor> i(chainColors);
    while (i.hasNext()) {
        i.next();
        chainColorMap.insert(i.key(), Color4f(i.value()));
    }
}

Color4f ChainsColorScheme::getSchemeAtomColor(const SharedAtom& atom) const
{
    Color4f color;
    if (chainColorMap.contains(atom->chainIndex)) {
        return chainColorMap.value(atom->chainIndex);
    } else {
        return defaultAtomColor;
    }
}


/* class SecStructColorScheme : public BioStruct3DColorScheme */

const QMap<QString, QColor> SecStructColorScheme::getSecStructAnnotationColors(const BioStruct3DObject *biostruct) {
    QMap<QString, QColor> colors;
    AnnotationSettingsRegistry* asr = AppContext::getAnnotationsSettingsRegistry();

    // bug-2857: GObject relations shoud be used
    Document *doc = biostruct->getDocument();
    foreach (GObject* obj, doc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE) ) {
        AnnotationTableObject *ao = qobject_cast<AnnotationTableObject*>(obj);
        assert(ao);

        foreach(Annotation *a, ao->getAnnotations()) {
            QString name = a->getAnnotationName();
            if (name == BioStruct3D::SecStructAnnotationTag) {
                QString ssName = a->getQualifiers().first().value;
                AnnotationSettings* as = asr->getAnnotationSettings(ssName);
                colors.insert(ssName, as->color);
            }
        }
    }

    return colors;
}

SecStructColorScheme::SecStructColorScheme(const BioStruct3DObject *biostruct)
        : BioStruct3DColorScheme(biostruct)
{
    defaultAtomColor = Color4f(0.5f,0.9f,0.9f);
    const QMap<QString, QColor> secStrucColors = getSecStructAnnotationColors(biostruct);
    QMapIterator<QString, QColor> i(secStrucColors);
    while (i.hasNext()) {
        i.next();
        secStrucColorMap.insert(i.key().toAscii(), Color4f(i.value()));
    }

    foreach (const SharedSecondaryStructure& struc, biostruct->getBioStruct3D().secondaryStructures) {
        for (int index = struc->startSequenceNumber; index <= struc->endSequenceNumber; ++index ) {
            QByteArray type = BioStruct3D::getSecStructTypeName(struc->type).toAscii();
            Q_ASSERT( secStrucColorMap.contains(type));
            Q_ASSERT( struc->chainIndex != 0);
            molMap[struc->chainIndex].strucResidueTable.insert(index, type);
        }
    }
    
#ifdef _DEBUG
    // Verify indices with biostruct3d 
    const BioStruct3D& bioStruc = biostruct->getBioStruct3D();
    QMapIterator<int,MolStructs> iter(molMap);
    while (iter.hasNext()) {
        iter.next();
        assert(bioStruc.moleculeMap.contains(iter.key()));
    }
#endif

}

Color4f SecStructColorScheme::getSchemeAtomColor( const SharedAtom& atom ) const
{
    Color4f c = defaultAtomColor;
    int residueIndex = atom->residueIndex;

    if (molMap.contains(atom->chainIndex)) {
        const QHash<int, QByteArray>& residueTable = molMap.value(atom->chainIndex).strucResidueTable;
        if (residueTable.contains(residueIndex)) {
            QByteArray type = residueTable.value(residueIndex);
            c = secStrucColorMap.value(type);
        }    
    }

    return c;
}


} //namespace
