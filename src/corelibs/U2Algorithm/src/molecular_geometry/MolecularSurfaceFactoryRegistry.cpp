#include "VanDerWaalsSurface.h"
#include "MolecularSurfaceFactoryRegistry.h"

namespace U2 {

MolecularSurfaceFactoryRegistry::MolecularSurfaceFactoryRegistry( QObject* pOwn /*= 0*/ ) : QObject(pOwn)
{
    registerSurfaceFactory(new VanDerWaalsSurfaceFactory(),QString("VanDerWaals"));
}

MolecularSurfaceFactoryRegistry::~MolecularSurfaceFactoryRegistry()
{
    foreach( MolecularSurfaceFactory* factory, surfMap.values()) {
        delete factory;
    }
}

bool MolecularSurfaceFactoryRegistry::registerSurfaceFactory( MolecularSurfaceFactory* surf, const QString& surfId )
{
    if (surfMap.contains(surfId)){
        return false;
    }
    surfMap.insert(surfId, surf);
    return true;

}

bool MolecularSurfaceFactoryRegistry::hadRegistered( const QString& surfId )
{
    return surfMap.contains(surfId);
}

MolecularSurfaceFactory* MolecularSurfaceFactoryRegistry::getSurfaceFactory( const QString& surfId )
{
    if (surfMap.contains(surfId)) {
        return surfMap.value(surfId);
    } else {
        return NULL;
    }
}

QStringList MolecularSurfaceFactoryRegistry::getSurfNameList()
{
    return surfMap.keys();
}

} // namespace U2
