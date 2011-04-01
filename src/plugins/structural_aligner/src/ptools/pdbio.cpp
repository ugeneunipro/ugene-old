// $Id: pdbio.cpp 666 2009-01-23 10:24:54Z asaladin $
#include <algorithm>
#include <cctype>
#include <iostream>
#include <fstream>

#include "atom.h"
#include "stdio.h"
#include "pdbio.h"

using namespace std;

namespace PTools{


std::string strip( std::string const& str, char const* sepSet ) {
    std::string::size_type const first = str.find_first_not_of(sepSet);
    return ( first==std::string::npos )
           ? std::string()
           : str.substr(first, str.find_last_not_of(sepSet)-first+1);
}


bool isAtom(const std::string & line ) {
    //cout << ligne.size()<< endl ;
    if (line.size()<10 ) return false;

    if (line.substr(0,6)==(std::string) "ATOM  " ) return true ;
    return false;
}


bool isHeteroAtom(const std::string& line) {
    if (line.size()<10) return false;

    if (line.substr(0,6) == "HETATM") return true;
    return false;
}




std::string readatomtype(const std::string &ligne) {
    std::string type="" ;
    int i=12 ;
    while (ligne[i]==' ')
    {
        i++;
        if (i>15) return type ;
    }

    int j=i ;
    while (ligne[j]!=' ') j++ ;

    type=ligne.substr(i,j-i) ;
    std::transform(type.begin(),type.end(),
                   type.begin(), //destination
                   (int(*)(int)) toupper //to upper: convert to upper case
                  );

    //cout << type <<"a" ;



    return type;
}


std::string readresidtype(const std::string &ligne) {
    std::string type="" ;
    int i=17 ;
    while (ligne[i]==' ')
    {
        i++;
        if (i>19) return type ;
    }

    int j=i ;
    while (ligne[j]!=' ') j++ ;

    type=ligne.substr(i,j-i) ;
    std::transform(type.begin(),type.end(),
                   type.begin(), //destination
                   (int(*)(int)) toupper //to upper: convert to upper case
                  );

    //cout << type <<"a" ;



    return type;
}


void ReadPDB(ifstream& fichier, Rigidbody& protein) {

    std::string ligne ;
    int compteur = 0 , compteur1=0;
    //std::cout << "file opening  " << nomfich.c_str() << std::endl ;


    while ( std::getline(fichier, ligne))
    {
        compteur++ ;
        if (isAtom(ligne))
        {
            // cout <<ligne <<endl;
            compteur1++ ;

            std::string sx,sy,sz;

            sx=ligne.substr(30,8);
            sy=ligne.substr(38,8);
            sz=ligne.substr(46,8);

            Coord3D pos;
            pos.x=atof(sx.c_str());
            pos.y=atof(sy.c_str());
            pos.z=atof(sz.c_str());

            Atomproperty a;
            a.SetType( readatomtype(ligne));
            a.SetResidType(readresidtype(ligne));
            a.SetChainId(ligne.substr(21,1));
            a.SetResidId(atoi(ligne.substr(22,4).c_str()));
            a.SetAtomId(atoi(ligne.substr(6,5).c_str()));
            std::string extra = ligne.substr(54,ligne.size()-1-54+1); //extracts everything after the position 27 to the end of line
            a.SetExtra(extra);

            protein.AddAtom(a,pos);

        }


    }

}



void ReadPDB(const std::string name,Rigidbody& protein ) {
    std::string nomfich=name ;
	// pointer toward the filename given in the constructor argument
    ifstream fichier(nomfich.c_str()); 
    if (!fichier)
    {
        std::ostringstream oss;
        throw std::invalid_argument("##### ReadPDB:Could not open file \"" + nomfich + "\" #####") ;
//        exit(-1);
    }

    ReadPDB(fichier, protein);
    fichier.close();
    return;

}

void WritePDB(const Rigidbody& rigid, std::string filename)
{

    FILE* file= fopen(filename.c_str(),"w") ;

    for (uint i=0; i<rigid.Size();i++)
    {

        const char * chainID="A" ;

        Atom at = rigid.CopyAtom(i);
        const char* atomname=at.GetType().c_str();
        const char* residName=at.GetResidType().c_str();
        int residnumber = at.GetResidId();
        chainID=at.GetChainId().c_str();

        int atomnumber = at.GetAtomId();

        Coord3D coord = at.GetCoords();
        dbl x = coord.x;
        dbl y = coord.y;
        dbl z = coord.z ;



        fprintf(file,"ATOM  %5d  %-4s%3s %1s%4d    %8.3f%8.3f%8.3f%s",atomnumber,atomname,residName,chainID,residnumber,real(x),real(y),real(z),at.GetExtra().c_str());
        fprintf(file,"\n");
    }

    fclose(file);
}

} //namespace PTools



