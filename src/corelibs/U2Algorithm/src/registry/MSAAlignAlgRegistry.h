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

#ifndef _U2_MSA_ALIGN_ALG_REGISTRY_H_
#define _U2_MSA_ALIGN_ALG_REGISTRY_H_

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QMutex>
#include <QtCore/QObject>

#include <U2Core/global.h>

namespace U2 {

class MSAAlignTaskFactory;
class MSAAlignGUIExtensionsFactory;
class MSAAlignAlgorithmEnv;


class U2ALGORITHM_EXPORT MSAAlignAlgRegistry : public QObject {
    Q_OBJECT
public:
    MSAAlignAlgRegistry(QObject* pOwn = 0);
    ~MSAAlignAlgRegistry();
    
    bool registerAlgorithm(MSAAlignAlgorithmEnv* env);
    MSAAlignAlgorithmEnv* unregisterAlgorithm(const QString& id);
    MSAAlignAlgorithmEnv* getAlgorithm(const QString& id) const;
    
    QStringList getRegisteredAlgorithmIds() const;
private:
    mutable QMutex mutex;
    QMap<QString, MSAAlignAlgorithmEnv*> algorithms;
    
    Q_DISABLE_COPY(MSAAlignAlgRegistry);
};

class U2ALGORITHM_EXPORT MSAAlignAlgorithmEnv {
public:
    MSAAlignAlgorithmEnv(const QString& id, MSAAlignTaskFactory* tf , MSAAlignGUIExtensionsFactory* guiExt);
    virtual ~MSAAlignAlgorithmEnv();
    
    const QString& getId()  const {return id;}
    
    MSAAlignTaskFactory* getTaskFactory() const {return taskFactory;}
    MSAAlignGUIExtensionsFactory* getGUIExtFactory() const {return guiExtFactory;}

private:
    Q_DISABLE_COPY(MSAAlignAlgorithmEnv);

protected:
    QString id;
    MSAAlignTaskFactory* taskFactory;
    MSAAlignGUIExtensionsFactory* guiExtFactory;
};

} // namespace

#endif
