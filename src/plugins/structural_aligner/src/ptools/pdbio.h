// $Id: pdbio.h 666 2009-01-23 10:24:54Z asaladin $
#ifndef PDBIO_H
#define PDBIO_H

#include <cctype>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>
#include <stdexcept>


#include "rigidbody.h"

namespace PTools
{



void ReadPDB(std::ifstream& fichier,Rigidbody& protein ); ///< read a PDB file from a file pointer and load datas in Rigidbody
void ReadPDB(const std::string name,Rigidbody& protein ); ///< read a PDB file from a filename and load datas in Rigidbody
void WritePDB(const Rigidbody& rigid, std::string filename); ///< write a PDB file given a Rigidbody and a filename

}

#endif //#ifndef PDBIO_H


