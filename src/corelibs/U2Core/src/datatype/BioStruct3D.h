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

#ifndef _U2_BIOSTRUCT3D_H_
#define _U2_BIOSTRUCT3D_H_

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QSharedData>

#include <U2Core/Vector3D.h>
#include <U2Core/global.h>
#include <U2Core/AnnotationData.h>

namespace U2 {

/* Represents biopolimer residue */
class ResidueData : public QSharedData    
{ 
public:
    enum Type {
        TYPE_UNKNOWN, TYPE_PROTEIN, TYPE_DNA, TYPE_RNA
    };
    Type type;
    QByteArray name;
    char acronym;
    int chainIndex;
    ResidueData() : type(TYPE_UNKNOWN), acronym(' '), chainIndex(0) {}
};

typedef QSharedDataPointer<ResidueData> SharedResidue;


/* Represents atom */
class AtomData : public QSharedData
{
public:
    int atomicNumber;
    int chainIndex;
    int residueIndex;
    QByteArray name;
    Vector3D coord3d;
    float occupancy, temperature;

public:
    AtomData() {
        chainIndex = 0;
        residueIndex = 0;
        atomicNumber = 0;
        temperature = -1.0;
        occupancy = 0;
    }

};

typedef QSharedDataPointer<AtomData> SharedAtom;

/* Represents chemical bond between atoms */
class Bond
{
    SharedAtom atom1;
    SharedAtom atom2;
public:
    Bond(const SharedAtom& a1, const SharedAtom& a2) : atom1(a1), atom2(a2) { }
    const SharedAtom getAtom1() { return atom1; }
    const SharedAtom getAtom2() { return atom2; }
};

/* Represents protein secondary structure: alpha-helix, beta-strand, turn */
class SecondaryStructure : public QSharedData {
public:
    enum Type {
        Type_None = -1, Type_AlphaHelix, Type_PiHelix, Type_310Helix,
        Type_BetaStrand, Type_BetaBridge, Type_Turn, Type_BendRegion
    };

    Type type;
    char chainIdentifier;
    int chainIndex;
    int startSequenceNumber;
    int endSequenceNumber;

public:
    SecondaryStructure() : type(Type_None)
    {
        startSequenceNumber = 0;
        endSequenceNumber = 0;
        chainIndex = 0;
        chainIdentifier = 0;
    }
};

typedef QSharedDataPointer<SecondaryStructure> SharedSecondaryStructure;

//! Model 3D
/*!
    Represents collection of molecule atomic coordinates and bonds
*/
class Molecule3DModel
{
public:
    QList<SharedAtom> atoms;
    QList<Bond> bonds;
};

//! Molecule
/*!
    Represents molecule (biopolymer or any other) 
*/
class MoleculeData : public QSharedData {
public:
    MoleculeData(): engineered(false)  {}
    MoleculeData(const QString& molName) : name(molName), engineered(false) {}
    QMap<int, SharedResidue> residueMap;
    QList<Molecule3DModel> models;
    QList<SharedAnnotationData> annotations;
    QString name;
    bool engineered;
};

typedef QSharedDataPointer<MoleculeData> SharedMolecule;
typedef QHash<int, SharedAtom> AtomCoordSet; 

/* Biological 3D structure */
class U2CORE_EXPORT BioStruct3D {

private:
    void generateChainAnnotations();
    void generateSecStructureAnnotations();

public:
    static QString SecStructAnnotationTag;
    static QString MoleculeAnnotationTag;
    static QString AlphaHelixAnnotationTag;
    static QString BetaStrandAnnotationTag;
    static QString TurnAnnotationTag;
    static QString ChainIdQualifierName;
    static QString SecStructTypeQualifierName;


    QMap <int, SharedMolecule> moleculeMap;
    QMap <int, AtomCoordSet> modelMap;
    QList<SharedAnnotationData> annotations;
    QList<SharedSecondaryStructure> secondaryStructures;
    QList<Bond> interMolecularBonds;
    QString descr;
    QByteArray pdbId;
    double maxDistFromCenter;
    Vector3D rotationCenter;

    BioStruct3D() { }
    // Const member functions
    Vector3D getCenter() const { return rotationCenter; }
    double getMaxDistFromCenter()  const { return maxDistFromCenter; }
    QByteArray getRawSequenceByChainId(int id) const;
    int getNumberOfAtoms() const;
    int getNumberOfResidues() const;
    const SharedAtom getAtomById(int index, int modelIndex) const;
    const SharedResidue getResidueById(int chainIndex, int residueIndex) const;
    // Modifiers
    void calcCenterAndMaxDistance();
    void generateAnnotations();
    // Static methods
    static const QString getSecStructTypeName(SecondaryStructure::Type type);

};

} //namespace



#endif // _U2_BIOSTRUCT3D_H_
