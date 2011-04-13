// $Id: rmsd.h 644 2008-12-22 15:05:30Z asaladin $
#ifndef RMSD_H
#define RMSD_H

#include "rigidbody.h"
#include "atomselection.h"
#include "coord3d.h"
#include "screw.h"
#include "basetypes.h"





namespace PTools
{

/// root mean square deviation without superposition
dbl Rmsd(const AtomSelection& atsel1, const AtomSelection& atsel2);


}

#endif //RSMD_H

