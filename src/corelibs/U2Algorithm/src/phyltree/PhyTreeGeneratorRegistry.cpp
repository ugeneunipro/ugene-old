#include  <U2Algorithm/PhyTreeGeneratorRegistry.h>

namespace U2 {

    PhyTreeGeneratorRegistry::PhyTreeGeneratorRegistry( QObject* pOwn /*= 0*/ ) : QObject(pOwn)
    {}

    PhyTreeGeneratorRegistry::~PhyTreeGeneratorRegistry()
    {
        foreach( PhyTreeGenerator* generator, genMap.values()) {
            delete generator;
        }
    }

    bool PhyTreeGeneratorRegistry::registerPhyTreeGenerator( PhyTreeGenerator* generator , const QString& gen_id)
    {
        if (genMap.contains(gen_id)){
            return false;
        }
        genMap.insert(gen_id, generator);
        return true;

    }

    bool PhyTreeGeneratorRegistry::hadRegistered( const QString& gen_id)
    {
        return genMap.contains(gen_id);
    }

    PhyTreeGenerator* PhyTreeGeneratorRegistry::getGenerator(const QString& gen_id)
    {
        if (genMap.contains(gen_id)) {
            return genMap.value(gen_id);
        } else {
            return NULL;
        }
    }

    QStringList PhyTreeGeneratorRegistry::getNameList()
    {
        return genMap.keys();
    }

} // namespace U2
