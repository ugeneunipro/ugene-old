/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include "GenomeAssemblyRegistry.h"

namespace U2 {

GenomeAssemblyTask::GenomeAssemblyTask( const GenomeAssemblyTaskSettings& s, TaskFlags _flags)
    : Task("GenomeAssemblyTask", _flags), settings(s), resultUrl("˚"){
}

QString GenomeAssemblyTask::getResultUrl() const{
    return resultUrl;
}

QVariant GenomeAssemblyTaskSettings::getCustomValue( const QString& optionName, const QVariant& defaultVal ) const {
    if (customSettings.contains(optionName)) {
        return customSettings.value(optionName);
    } else {
        return defaultVal;
    }
}

bool GenomeAssemblyTaskSettings::hasCustomValue(const QString & name) const {
    return customSettings.contains(name);
}

void GenomeAssemblyTaskSettings::setCustomValue( const QString& optionName, const QVariant& val ) {
    customSettings.insert(optionName,val);
}

void GenomeAssemblyTaskSettings::setCustomSettings( const QMap<QString, QVariant>& settings ) {
    customSettings = settings;
}

GenomeAssemblyAlgorithmEnv::GenomeAssemblyAlgorithmEnv(
    const QString &id,
    GenomeAssemblyTaskFactory *taskFactory,
    GenomeAssemblyGUIExtensionsFactory *guiExtFactory,
    const QStringList &readsFormats)
: id(id), taskFactory(taskFactory), guiExtFactory(guiExtFactory),
readsFormats(readsFormats)
{

}

GenomeAssemblyAlgorithmEnv::~GenomeAssemblyAlgorithmEnv() {
    delete taskFactory;
    delete guiExtFactory;
}

GenomeAssemblyAlgRegistry::GenomeAssemblyAlgRegistry( QObject* pOwn /* = 0*/ ) : QObject(pOwn) {
}

GenomeAssemblyAlgRegistry::~GenomeAssemblyAlgRegistry() {
    foreach( GenomeAssemblyAlgorithmEnv* algo, algorithms.values()) {
        delete algo;
    }
}

bool GenomeAssemblyAlgRegistry::registerAlgorithm(GenomeAssemblyAlgorithmEnv* algo) {
    QMutexLocker locker(&mutex);

    if (algorithms.contains(algo->getId())){
        return false;
    }
    algorithms.insert(algo->getId(), algo);
    return true;

}

GenomeAssemblyAlgorithmEnv* GenomeAssemblyAlgRegistry::unregisterAlgorithm(const QString& id) {
    QMutexLocker locker(&mutex);

    if (!algorithms.contains(id)) {
        return NULL;
    }
    GenomeAssemblyAlgorithmEnv* res = algorithms.value(id);
    algorithms.remove(id);
    return res;
}

GenomeAssemblyAlgorithmEnv* GenomeAssemblyAlgRegistry::getAlgorithm( const QString& id) const {
    QMutexLocker locker(&mutex);
    return algorithms.value(id);
}


QStringList GenomeAssemblyAlgRegistry::getRegisteredAlgorithmIds() const {
    return algorithms.keys();
}

QStringList GenomeAssemblyUtils::getPairTypes(){
    return QStringList() << PAIR_TYPE_DEFAULT << PAIR_TYPE_MATE << PAIR_TYPE_MATE_HQ;
}

QStringList GenomeAssemblyUtils::getOrientationTypes(){
    return QStringList() << ORIENTATION_FR << ORIENTATION_RF << ORIENTATION_FF;
}

QString GenomeAssemblyUtils::getDefaultOrientation(const QString &pairType){
    if(pairType == PAIR_TYPE_MATE || pairType == PAIR_TYPE_MATE_HQ){
        return ORIENTATION_RF;
    }
    return ORIENTATION_FR;
}

QStringList GenomeAssemblyUtils::getLibraryTypes(){
    QStringList res;
    res << LIBRARY_SINGLE << LIBRARY_PAIRED << LIBRARY_PAIRED_INTERLACED << LIBRARY_PAIRED_UNPAIRED << LIBRARY_SANGER << LIBRARY_PACBIO;
    return res;
}

bool GenomeAssemblyUtils::isLibraryPaired(const QString& libName){
    if (libName == LIBRARY_PAIRED || libName == LIBRARY_PAIRED_INTERLACED || libName == LIBRARY_PAIRED_UNPAIRED){
        return true;
    }
    return false;
}

bool GenomeAssemblyUtils::hasRightReads(const QString& libName){
    if (libName == LIBRARY_PAIRED){
        return true;
    }
    return false;
}


QString GenomeAssemblyUtils::getYamlLibraryName(const QString &libName, const QString& paiType){
    QString result = "";
    if(libName == LIBRARY_SINGLE){
        result = "single";
    }else if (libName == LIBRARY_SANGER){
        result = "sanger";
    }else if (libName == LIBRARY_PACBIO){
        result = "pacbio";
    }else{
        if(paiType == PAIR_TYPE_DEFAULT){
            result = "paired-end";
        }else if (paiType == PAIR_TYPE_MATE){
            result = "mate-pairs";
        }else if (paiType == PAIR_TYPE_MATE_HQ){
            result = "hq-mate-pairs";
        }
    }

    return result;
}


} //namespace


