#ifndef SOLVENTEXCLUDEDSURFACE_H
#define SOLVENTEXCLUDEDSURFACE_H

#include <U2Algorithm/MolecularSurface.h>

namespace U2
{

class Surface;

class SolventExcludedSurface : public MolecularSurface
{
public:
    SolventExcludedSurface();
    virtual void calculate(const QList<SharedAtom>& atoms, int& progress);
    virtual qint64 estimateMemoryUsage(int numberOfAtoms);
};

class SolventExcludedSurfaceFactory : public MolecularSurfaceFactory
{
public:
    virtual MolecularSurface *createInstance()const;
    bool hasConstraints(const BioStruct3D& biostruc) const;
};

} // namespace

#endif // SOLVENTEXCLUDEDSURFACE_H
