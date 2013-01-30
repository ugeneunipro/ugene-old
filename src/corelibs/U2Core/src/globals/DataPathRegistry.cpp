/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "DataPathRegistry.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>

#include <QtCore/QFile>

namespace U2 {

////////////////////////////////////////
//U2DataPath
U2DataPath::U2DataPath(const QString& _name, const QString& _path, bool _folders, bool _recurcive, const QString& _descr)
: name(_name)
, path(_path)
, description (_descr)
, valid(false)
, folders(_folders)
{
    init(_recurcive);
}

QString U2DataPath::getPathByName(const QString& name) const{
    QString res = "";

    if (dataItems.contains(name)){
        res = dataItems.value(name, "");
    }

    return res;
}

void U2DataPath::init(bool recurcive){
    if(path.isEmpty() || !QFile::exists(path)){
        valid = false;
        return;
    }

    QFileInfo fi(path);
    QString filePath = fi.absoluteFilePath();
    path = filePath;

    if (fi.isDir()){
        fillDataItems(fi.absoluteFilePath(), recurcive);
        
    }else if (fi.isFile()){
        if (!folders){
            QString fileName = chopExtention(fi.fileName());
            dataItems.insertMulti(fileName, filePath);
        }
    }

    valid = true;
}

void U2DataPath::fillDataItems( const QDir& dir, bool recurcive ){
    QFileInfoList infoList = dir.entryInfoList( QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files);

    foreach(QFileInfo fi, infoList){
        if(fi.isFile()){
            if (!folders){
                QString fileName = chopExtention(fi.fileName());
                QString filePath = fi.absoluteFilePath();

                dataItems.insertMulti(fileName, filePath);
            }
        }else if (fi.isDir()){
            if (folders){
                QString fileName = fi.fileName();
                QString filePath = fi.absoluteFilePath();

                dataItems.insertMulti(fileName, filePath);
            }
            if(recurcive){
                fillDataItems(fi.absoluteFilePath(), recurcive);
            }
        }
    }
}
QVariantMap U2DataPath::getDataItemsVariantMap() const{
    QVariantMap vm;

    const QList<QString>& keys = dataItems.keys();
    foreach(const QString& key, keys){
        vm.insert(key, dataItems[key]);
    }

    return vm;
}

QString U2DataPath::chopExtention( QString name ){
    if (name.endsWith(".gz")) {
        name.chop(3);
    }
    int dot = name.lastIndexOf('.');
    if (dot > 0) {
        name.chop(name.size() - dot);
    }

    return name;
}

////////////////////////////////////////
//U2DataPathRegistry
U2DataPathRegistry::~U2DataPathRegistry() {
    qDeleteAll(registry.values());
}

U2DataPath* U2DataPathRegistry::getDataPathByName(const QString& name)
{
    return registry.value(name, NULL);
}

bool U2DataPathRegistry::registerEntry(U2DataPath *dp){
    if (registry.contains(dp->getName()) || !dp->isValid()) {
        return false;
    } else {
        registry.insert(dp->getName(), dp);
    }
    return true;
}

void U2DataPathRegistry::unregisterEntry(const QString &name){
    delete registry.take(name);
}

QList<U2DataPath*> U2DataPathRegistry::getAllEntries() const{
    return registry.values();
}
}//namespace
