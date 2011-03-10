#include <U2Core/Log.h>
#include "BioStruct3D.h"

namespace U2 { 

QString BioStruct3D::MoleculeAnnotationTag("chain_info");
QString BioStruct3D::AlphaHelixAnnotationTag("alpha_helix");
QString BioStruct3D::BetaStrandAnnotationTag("beta_strand");
QString BioStruct3D::TurnAnnotationTag("turn");
QString BioStruct3D::ChainIdQualifierName("chain_id");
QString BioStruct3D::SecStructAnnotationTag("sec_struct");
QString BioStruct3D::SecStructTypeQualifierName("sec_struct_type");


void BioStruct3D::calcCenterAndMaxDistance() {

    Vector3D siteSum;
    Vector3D center;
    double dist;
    maxDistFromCenter = 0.0;
    int numberOfAtoms = 0;
    // loop trough all atoms twice - once to get average center, then once to
    // find max distance from this center
    for (int i = 0; i < 2; ++i) {
        foreach (SharedMolecule molecule, moleculeMap) {
            foreach(Molecule3DModel model3d, molecule->models) {
                foreach (const AtomData* atom, model3d.atoms) {
                    Vector3D site = atom->coord3d;
                    if (i==0) {
                        siteSum += atom->coord3d;
                        ++numberOfAtoms;
                    }
                    else {
                        dist = (site - center).length();
                        if (dist > maxDistFromCenter){
                            maxDistFromCenter = dist;
                        }
                    }
                }
            }
        }
        
        if (i == 0) {
            if (numberOfAtoms == 0) {
                algoLog.trace("Number of atoms is 0!");
            } else {
                center = siteSum / numberOfAtoms;
            }
        }

    }
    algoLog.trace(QString("center: (%1,%2,%3)\n maxDistFromCenter: %4").arg(center.x).arg(center.y).arg(center.z).arg(maxDistFromCenter) );

    rotationCenter = center;

} 

int BioStruct3D::getNumberOfAtoms() const
{
    // get first coordinates set
    const AtomCoordSet& coordSet = modelMap.begin().value();
    return coordSet.count();
}

void BioStruct3D::generateAnnotations()
{
    generateChainAnnotations();
    generateSecStructureAnnotations();
}

void BioStruct3D::generateChainAnnotations()
{
    const char* molNameQualifier = "molecule_name";
    //const char* pdbChainIdQualifier = "pdb_id";
    
    QMap<int, SharedMolecule>::iterator iter = moleculeMap.begin();
    while (iter != moleculeMap.end()) {
        int length = iter.value()->residueMap.size();
        SharedAnnotationData sd( new AnnotationData);
        sd->location->regions << U2Region(0,length);
        sd->name = BioStruct3D::MoleculeAnnotationTag;
        sd->qualifiers.append(U2Qualifier(ChainIdQualifierName, QString("%1").arg(iter.key()) ));
        sd->qualifiers.append(U2Qualifier(molNameQualifier, (*iter)->name));
        
        (*iter)->annotations.append(sd);
        ++iter;
    }
    
}

int BioStruct3D::getNumberOfResidues() const
{
    int numResidues = 0;

    foreach (SharedMolecule mol, moleculeMap) {
        numResidues += mol->residueMap.size();
    }

    return numResidues;
}

const SharedAtom BioStruct3D::getAtomById( int atomIndex, int modelIndex ) const
{
    if (modelMap.contains(modelIndex)) {
        const AtomCoordSet& coordSet = modelMap.value(modelIndex);
        if (coordSet.contains(atomIndex)) {
            return coordSet.value(atomIndex);
        }
    }   
       
    return SharedAtom(NULL);
}

const SharedResidue BioStruct3D::getResidueById( int chainIndex, int residueIndex ) const
{
    const SharedMolecule mol =  moleculeMap.value(chainIndex);
    if (mol->residueMap.contains(residueIndex)) {
        return mol->residueMap.value(residueIndex);
    }

    return SharedResidue(NULL);   
}


const QString BioStruct3D::getSecStructTypeName( SecondaryStructure::Type type )
{
    switch(type) {
    case SecondaryStructure::Type_AlphaHelix:
        return BioStruct3D::AlphaHelixAnnotationTag;
    case SecondaryStructure::Type_PiHelix:
        return "pi_helix";
    case SecondaryStructure::Type_310Helix:
        return "310_helix";
    case SecondaryStructure::Type_BetaStrand:
        return BioStruct3D::BetaStrandAnnotationTag;
    case SecondaryStructure::Type_BetaBridge:
        return "beta_bridge";
    case SecondaryStructure::Type_Turn:
        return BioStruct3D::TurnAnnotationTag;
    case SecondaryStructure::Type_BendRegion:
        return "bend_region";
    default:
        return QString("unknown");
    }

}



void BioStruct3D::generateSecStructureAnnotations()
{
    // TODO: issue 0000637
    if (moleculeMap.isEmpty())
        return;
    
    
    foreach (const SharedSecondaryStructure& struc, secondaryStructures) {
        SharedAnnotationData sd(NULL);
        int chainId = struc->chainIndex;
        assert(chainId != 0);
        int initResidueId = moleculeMap.value(chainId)->residueMap.constBegin().key();    
        sd = new AnnotationData;
        sd->name = BioStruct3D::SecStructAnnotationTag;
        U2Qualifier qual(SecStructTypeQualifierName, getSecStructTypeName(struc->type));
        sd->qualifiers.append(qual);
        int numResidues = struc->endSequenceNumber - struc->startSequenceNumber + 1;
        int startIndex = struc->startSequenceNumber - initResidueId;
        U2Region chainRegion(startIndex, numResidues);
        sd->location->regions << chainRegion;
        Q_ASSERT(moleculeMap.contains(chainId));
        moleculeMap[chainId]->annotations.append(sd);
    }

}

QByteArray BioStruct3D::getRawSequenceByChainId( int id ) const
{
    QByteArray sequence("");

    Q_ASSERT(moleculeMap.contains(id));
    const SharedMolecule molecule = moleculeMap.value(id);
    foreach (const SharedResidue residue, molecule->residueMap) {
       QChar c = residue->acronym;
       sequence.append(c);
    }
    
    return sequence;
}

} //namespace
