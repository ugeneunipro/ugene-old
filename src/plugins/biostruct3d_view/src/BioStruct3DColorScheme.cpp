/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <QtCore/QObject>
#include <QtAlgorithms>

#include "BioStruct3DColorScheme.h"
#include "BioStruct3DGLWidget.h"


namespace U2 { 


///////////////////////////////////////////////////////////////////////////////////////////
// BioStruct3DColorSchemeFactory

const QString ChainsColorScheme::schemeName(QObject::tr("chain colors"));
const QString ChemicalElemColorScheme::schemeName(QObject::tr("chemical element colors"));
const QString SecStructColorScheme::schemeName(QObject::tr("secondary structure colors"));

#define REGISTER_FACTORY(c) \
    map.insert(c::schemeName, new c::Factory)

QMap<QString,BioStruct3DColorSchemeFactory*> BioStruct3DColorSchemeFactory::createFactories()
{
    QMap<QString,BioStruct3DColorSchemeFactory*> map;
    REGISTER_FACTORY(ChainsColorScheme);
    REGISTER_FACTORY(SecStructColorScheme);
    REGISTER_FACTORY(ChemicalElemColorScheme);

    return map;
}

const QString BioStruct3DColorSchemeFactory::defaultFactoryName()
{
    return SecStructColorScheme::schemeName;
}


///////////////////////////////////////////////////////////////////////////////////////////
// BioStruct3DColorScheme

BioStruct3DColorScheme::BioStruct3DColorScheme(const BioStruct3DGLWidget* widget) : 
    glWidget(widget), defaultAtomColor(0.25f, 0.25f, 0.25f), selectionColor(1.0f, 1.0f, 0), unselectedShading(0.0)
{
}

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

void BioStruct3DColorScheme::setSelectionColor(QColor color)
{
    this->selectionColor=color;
}

void BioStruct3DColorScheme::setUnselectedShadingLevel(float shading)
{
    assert(shading >= 0.0 && shading <= 1.0);
    unselectedShading = shading;
}

void BioStruct3DColorScheme::updateSelectionRegion(int chainId, const QVector<U2Region>& added, const QVector<U2Region>& removed)
{
    // Residues in chain could start from any number
    int residueStartId = glWidget->getBioStruct3D().moleculeMap.value(chainId)->residueMap.begin().key();

    foreach( const U2Region region, removed) {
        int startPos = region.startPos;
        int endPos = region.endPos();
        for (int i = startPos; i < endPos; ++i) {
            int residueId = residueStartId + i;
            removeFromSelection(chainId, residueId);
        }
    }

    foreach( const U2Region region, added) {
        int startPos = region.startPos;
        int endPos = region.endPos();
        for (int i = startPos; i < endPos; ++i) {
            int residueId = residueStartId + i;
            addToSelection(chainId, residueId);
        }
    }
}

void BioStruct3DColorScheme::addToSelection(int chainId, int residueId)
{
    /*    //TODO: optimize this
    if (!curSelection.contains(SelectionId(chainId,residueId))) {
        curSelection.append(SelectionId(chainId, residueId));
    }

    //qSort(curSelection);
    */
    if (!selection.contains(chainId, residueId)) {
        selection.insert(chainId, residueId);
    }
}

void BioStruct3DColorScheme::removeFromSelection(int chainId, int residueId)
{
    selection.remove(chainId, residueId);
    /*curSelection.removeOne(QPair<int,int>(chainId, residueId));*/
}

bool BioStruct3DColorScheme::isInSelection( const SharedAtom& atom ) const
{
   /*
   QLinkedList<SelectionId>::const_iterator i;
   SelectionId selId(atom->chainIndex, atom->residueIndex);
   for ( i = curSelection.constBegin(); i != curSelection.constEnd(); ++i) {
        if ((*i) == selId)
            return true;
   }
   */
   return selection.contains(atom->chainIndex, atom->residueIndex);

//   qBinaryFind(curSelection.begin(), curSelection.end(), QPair<int,int> (chainId, residueId) );
//   if (i != curSelection.end()) {
//       curSelection.erase(i);
//   }

   return false;
}

Color4f BioStruct3DColorScheme::getSchemeAtomColor( const SharedAtom& atom ) const
{
    Q_UNUSED(atom);
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

ChemicalElemColorScheme::ChemicalElemColorScheme(const BioStruct3DGLWidget* widget)
        : BioStruct3DColorScheme(widget)
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

///////////////////////////////////////////////////////////////////////////////////////////
// ChainsColorScheme


ChainsColorScheme::ChainsColorScheme(const BioStruct3DGLWidget* widget) : BioStruct3DColorScheme(widget)
{
    const QMap<int, QColor> chainColors = glWidget->getChainColors();
    Q_ASSERT(!chainColors.empty());
    QMapIterator<int, QColor> i(chainColors);
    while (i.hasNext()) {
        i.next();
        chainColorMap.insert(i.key(), Color4f(i.value()));
    }
}
Color4f ChainsColorScheme::getSchemeAtomColor( const SharedAtom& atom ) const
{
    Color4f color;
    if (chainColorMap.contains(atom->chainIndex)) {
        return chainColorMap.value(atom->chainIndex);
    } else {
        return defaultAtomColor;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
// SecStrucColorScheme

SecStructColorScheme::SecStructColorScheme(const BioStruct3DGLWidget* widget) : BioStruct3DColorScheme(widget) {
    defaultAtomColor = Color4f(0.5f,0.9f,0.9f);
    const QMap<QString, QColor> secStrucColors = glWidget->getSecStructAnnotationColors();
    QMapIterator<QString, QColor> i(secStrucColors);
    while (i.hasNext()) {
        i.next();
        secStrucColorMap.insert(i.key().toAscii(), Color4f(i.value()));
    }

    foreach (const SharedSecondaryStructure& struc, widget->getBioStruct3D().secondaryStructures) {
        for (int index = struc->startSequenceNumber; index <= struc->endSequenceNumber; ++index ) {
            QByteArray type = BioStruct3D::getSecStructTypeName(struc->type).toAscii();
            Q_ASSERT( secStrucColorMap.contains(type));
            Q_ASSERT( struc->chainIndex != 0);
            molMap[struc->chainIndex].strucResidueTable.insert(index, type);
        }
    }
    
#ifdef _DEBUG
    // Verify indices with biostruct3d 
    const BioStruct3D& bioStruc = widget->getBioStruct3D();
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
