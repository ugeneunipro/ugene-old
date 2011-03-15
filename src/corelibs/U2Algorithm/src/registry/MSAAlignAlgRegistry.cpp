/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "MSAAlignAlgRegistry.h"

#include <U2Algorithm/MSAAlignTask.h>
#include <U2View/MSAAlignGUIExtension.h>

namespace U2 {

MSAAlignAlgorithmEnv::MSAAlignAlgorithmEnv(const QString& _id, MSAAlignTaskFactory* _tf, 
                                                 MSAAlignGUIExtensionsFactory* _g)
: id(_id), taskFactory(_tf), guiExtFactory(_g)
{
}

MSAAlignAlgorithmEnv::~MSAAlignAlgorithmEnv() {
    delete taskFactory;
    delete guiExtFactory;
}

MSAAlignAlgRegistry::MSAAlignAlgRegistry( QObject* pOwn /*= 0*/ ) : QObject(pOwn) {
}

MSAAlignAlgRegistry::~MSAAlignAlgRegistry() {
    foreach( MSAAlignAlgorithmEnv* algo, algorithms.values()) {
        delete algo;
    }
}

bool MSAAlignAlgRegistry::registerAlgorithm(MSAAlignAlgorithmEnv* algo) {
    QMutexLocker locker(&mutex);

    if (algorithms.contains(algo->getId())){
        return false;
    }
    algorithms.insert(algo->getId(), algo);
    return true;

}

MSAAlignAlgorithmEnv* MSAAlignAlgRegistry::unregisterAlgorithm(const QString& id) {
    QMutexLocker locker(&mutex);
    
    if (!algorithms.contains(id)) {
        return NULL;
    }
    MSAAlignAlgorithmEnv* res = algorithms.value(id);
    algorithms.remove(id);
    return res;
}

MSAAlignAlgorithmEnv* MSAAlignAlgRegistry::getAlgorithm( const QString& id) const {
    QMutexLocker locker(&mutex);
    return algorithms.value(id);
}


QStringList MSAAlignAlgRegistry::getRegisteredAlgorithmIds() const {
    return algorithms.keys();
}

} //namespace

