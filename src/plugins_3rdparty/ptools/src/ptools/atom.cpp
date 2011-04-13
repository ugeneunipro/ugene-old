// $Id: atom.cpp 699 2009-05-24 21:30:19Z asaladin $
#include <sstream>
#include <stdio.h>

#include "atom.h"
#include "coord3d.h"

using namespace std;

namespace PTools{

Coord3D Atom::GetCoords() const {return mCoords;}

// unused by UGENE
//! Convert an atom to a string
/*std::string Atom::ToString() const {
    std::stringstream info;
    info<<GetAtomId()<<" "<<GetType()<<" ";
    info<<GetResidType()<<" "<<GetChainId()<<" "<<GetResidId();
    Coord3D coord = GetCoords();
    info<<PrintCoord(coord);
    return info.str();
}*/


// unused by UGENE
//! convert an atom to a string in PDB format
/*std::string Atom::ToPdbString() const
{
    char output[81];
    const char* atomname = GetType().c_str();
    const char* residName = GetResidType().c_str();
    int residnumber = GetResidId();
    const char* chainID = GetChainId().c_str();

    int atomnumber = GetAtomId();

    Coord3D coord = GetCoords();
    double x = coord.x;
    double y = coord.y;
    double z = coord.z ;

    snprintf(output,80,"ATOM  %5d  %-4s%3s %1s%4d    %8.3f%8.3f%8.3f%s\n",atomnumber,atomname,residName,chainID,residnumber,x,y,z,GetExtra().c_str());
    output[79]='\n';
    output[80]='\0';
    return std::string(output);
}*/

//! translate an atom with a Coord3D vector
void Atom::Translate(const Coord3D& tr)
{
    mCoords=mCoords+tr;
}


} // namespace PTools

