// $Id: rigidbody.cpp 699 2009-05-24 21:30:19Z asaladin $
/****************************************************************************
 *   Copyright (C) 2006-2008   Adrien Saladin                               *
 *   adrien.saladin@gmail.com                                               *
 *   Copyright (C) 2008   Pierre Poulain                                    *
 *   Copyright (C) 2008   Sebastien Fiorucci                                *
 *   Copyright (C) 2008   Chantal Prevost                                   *
 *   Copyright (C) 2008   Martin Zacharias                                  *
 *                                                                          *
 *   This program is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by   *
 *   the Free Software Foundation, either version 3 of the License, or      *
 *   (at your option) any later version.                                    *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 *                                                                          *
 ***************************************************************************/


#include "rigidbody.h"
#include "atomselection.h"
#include "geometry.h"
#include "pdbio.h"


namespace PTools{


Rigidbody::Rigidbody()
{
    ResetMatrix();
}



Rigidbody::Rigidbody(std::string filename)
{
    ReadPDB(filename,*this);
    ResetMatrix();
}


Rigidbody::Rigidbody(const Rigidbody& model)
        : CoordsArray(model)
{
//this copy constructor is needed because dbl[4][4] is not
// automatically copied with the default copy constructor
//TODO: verifier si c'est toujours le cas ...

    this->mForces = model.mForces;
    this->mAtomProp = model.mAtomProp;
    this-> _description = model._description;

}


void Rigidbody::AddAtom(const Atomproperty& at, Coord3D co)
{
    mAtomProp.push_back(at);
    AddCoord(co);
}


Atom Rigidbody::CopyAtom(uint i) const
{
    Atom at(mAtomProp[i],GetCoords(i));
    return at;
}

void Rigidbody::SetAtom(uint pos, const Atom& atom)
{

   if (/*pos<0  always true||*/ pos >= this->Size())
   {
      std::string message = "SetAtom: position ";
      message += pos;
      message += " is out of range";
      throw std::out_of_range(message);
   }
   Atomproperty atp(atom);
   Coord3D co(atom.GetCoords());
   SetAtomProperty(pos, atp);
   SetCoords(pos,co);
}



void Rigidbody::AddAtom(const Atom& at)
{
    Atomproperty atp(at);
    Coord3D co = at.GetCoords();
    AddAtom(atp,co);
}


Coord3D Rigidbody::FindCenter() const
{
    Coord3D center(0.0,0.0,0.0);
    for (uint i=0; i< this->Size() ; i++)
    {
        center =  center + GetCoords(i);
    }
    return ( (1.0/(dbl)this->Size())*center);
}


void Rigidbody::CenterToOrigin()
{
    Coord3D c = FindCenter();
    Translate(Coord3D()-c);
}

dbl Rigidbody::RadiusGyration()
{
    Coord3D c = this->FindCenter();
    dbl r=0.0;
    for (uint i=0; i< this->Size(); i++)
    {
        r += Norm2( c - this->GetCoords(i) );
    }

    dbl result = sqrt( r/ (double) this->Size() );
    return result;
}

dbl Rigidbody::Radius()
{
    Coord3D center = this->FindCenter();
    uint size = this->Size();
    dbl radius = 0.0;
    for (uint i=0; i < size; i++)
    {
        dbl rad=Norm(center - this->GetCoords(i));
        if (radius < rad) {radius=rad;}
    }
    return radius;
}


void Rigidbody::Translate(const Coord3D& tr)
{
    CoordsArray::Translate(tr);
}

void Rigidbody::AttractEulerRotate(dbl phi, dbl ssi, dbl rot)
{
   CoordsArray::AttractEulerRotate(phi, ssi, rot);
}





AtomSelection Rigidbody::SelectAllAtoms() const
{
    AtomSelection newsel;
    newsel.SetRigid(*this);
    for (uint i=0; i < Size(); i++)
    {
        newsel.AddAtomIndex(i);
    }


    return newsel;

}


AtomSelection Rigidbody::SelectAtomType(std::string atomtype)
{
    AtomSelection newsel;
    newsel.SetRigid(*this);

    for (uint i=0; i<Size(); i++)
    {
        if ( mAtomProp[i].GetType()==atomtype)
            newsel.AddAtomIndex(i);
    }

    return newsel;
}


AtomSelection Rigidbody::SelectResidType(std::string residtype)
{
    AtomSelection newsel;
    newsel.SetRigid(*this);

    for (uint i=0; i<Size(); i++)
    {
        if (mAtomProp[i].GetResidType()==residtype)
            newsel.AddAtomIndex(i);
    }
    return newsel;
}


AtomSelection Rigidbody::SelectChainId(std::string chainId) {
    AtomSelection newsel;
    newsel.SetRigid(*this);
    for (uint i=0; i<Size(); i++)
    {
        if (mAtomProp[i].GetChainId()==chainId)
            newsel.AddAtomIndex(i);
    }
    return newsel;
}

AtomSelection Rigidbody::SelectResRange(uint start, uint stop)
{
    AtomSelection newsel;
    newsel.SetRigid(*this);

    for (uint i=0; i < Size(); i++)
    {
        Atomproperty& atp ( mAtomProp[i] );
        if (atp.GetResidId() >=start && atp.GetResidId() <= stop) newsel.AddAtomIndex(i);
    }
    return newsel;
}


AtomSelection Rigidbody::CA() {
    return SelectAtomType("CA");
}

bool isBackbone(const std::string &  atomtype)
{

    const std::string bbtypes[] = {"N", "CA", "C", "O"};
    int const bbsize = sizeof(bbtypes)/sizeof(std::string);

    for (int i =0; i<bbsize; i++)
    {
        if (atomtype == bbtypes[i] ) return true;
    }

    return false;
}


AtomSelection Rigidbody::Backbone()
{
    AtomSelection newsel;
    newsel.SetRigid(*this);

    for (uint i=0; i<this->Size(); i++)
    {
        if (isBackbone(CopyAtom(i).GetType()) )
        {
            newsel.AddAtomIndex(i);
        }

    }
    return newsel;
}


/// operator +
Rigidbody Rigidbody::operator+(const Rigidbody& rig) {
    Rigidbody rigFinal(*this);
    for (uint i=0; i< rig.Size() ; i++) {
        rigFinal.AddCoord(rig.GetCoords(i));
        rigFinal.mAtomProp.push_back(rig.mAtomProp[i]);
    }
    return rigFinal;
}


void Rigidbody::ABrotate(const Coord3D& A, const Coord3D& B, dbl theta)
{
    PTools::ABrotate(A,B, *this, theta);
}



// unused by UGENE
/*std::string Rigidbody::PrintPDB() const
{
    uint size=this->Size();

    std::string output;
    for (uint i=0; i < size ; i++)
    {
         Atom at(mAtomProp[i], this->GetCoords(i));
         output = output + at.ToPdbString();
    }
    return output;
}*/


void Rigidbody::ApplyMatrix(const Matrix& mat)
{

   dbl mat44[4][4];
   for(uint i=0; i<4;i++)
    for(uint j=0;j<4;j++)
      mat44[i][j]=mat(i,j);
   CoordsArray::MatrixMultiply(mat44);
}




} //namespace PTools
