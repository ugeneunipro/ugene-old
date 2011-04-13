// $Id: atom.h 614 2008-11-25 12:26:40Z asaladin $


#ifndef ATOM_H
#define ATOM_H

#include "basetypes.h"

#include <string>
#include <iostream>
#include "coord3d.h"

namespace PTools{


class Atomproperty {
private:
    std::string mAtomType;  ///< CA, N, HN1, ...
    std::string mAtomElement; ///< C, N, H, O, etc.
    std::string mResidType; ///< LEU, ARG, ...
    std::string mChainId; ///< A, B, etc.
    uint mResidId; ///< residue number
    uint mAtomId; ///< atom number
    dbl mAtomCharge; ///< charge of the atom
    std::string mExtra; ///< extra data

public:
    /// default constructor
    Atomproperty()
    {
        mAtomType="X";
        mAtomElement="X";
        mResidType="XXX";
        mChainId="X";
        mResidId=1;
        mAtomId=1;
        mAtomCharge=0.0;
    };

    /// return atom type (CA, CB, O, N...)
    std::string GetType() const  {return mAtomType;};

    /// define atom type (CA, CB, O, N...)
    void SetType(std::string newtype) { mAtomType = newtype;};

    /// return residue type (LEU, ARG...)
    std::string GetResidType() const {return mResidType;};

    /// define residue type (LEU, ARG...)
    void SetResidType(std::string residtype){mResidType=residtype;};

    /// return atom charge
    inline dbl GetAtomCharge() const {return mAtomCharge;};

    /// define atom charge
    inline void SetAtomCharge(dbl ch) {mAtomCharge=ch;};

    /// return chain ID (A, B...)
    inline std::string GetChainId() const {return mChainId;};

    /// define chain ID (A, B...)
    inline void SetChainId(std::string chainid) {mChainId=chainid;};

    /// return residue ID (1, 2...)
    inline uint GetResidId() const {return mResidId;};

    /// define residue ID (1, 2...)
    inline void SetResidId(uint id) {mResidId = id;};

    /// return atom ID (1, 2...)
    inline uint GetAtomId() const {return mAtomId;};

    /// define atom ID (1, 2...)
    inline void SetAtomId(uint atomnumber) {mAtomId=atomnumber;};

    /// set the extra data field
    inline void SetExtra(std::string extra){mExtra=extra;};

    /// get the extra data field
    inline std::string GetExtra() const {return mExtra;};

};


class Atom : public Atomproperty
{

private:
    Coord3D mCoords; ///< Atom cartesian coordinates

public:

    Atom(Atomproperty ap, Coord3D co)
            : Atomproperty(ap), mCoords(co) {};
    Coord3D GetCoords() const; ///< return atom coordinates

    /// define atom coordinates
    inline void SetCoords(const Coord3D& coords) {mCoords=coords;};

    // unused by UGENE
    /// convert atom (properties and coordinates) to std::string
    //std::string ToString() const;

    // unused by UGENE
    /// convert atom (properties and coordinates) to classical PDB-like string
    //std::string ToPdbString() const ;

    /// translation of an atom
    void Translate(const Coord3D& tr);

};


/// distance between two atoms
inline dbl Dist(const Atom& at1, const Atom& at2)
{
    return Norm(at1.GetCoords()-at2.GetCoords());
}

/// distance**2 between two atoms
inline dbl Dist2(const Atom& at1, const Atom& at2)
{
    return Norm2(at1.GetCoords()-at2.GetCoords());
}


}

#endif


