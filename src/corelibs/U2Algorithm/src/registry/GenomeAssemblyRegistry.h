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

#ifndef GENOMEASSEMBLYREGISTRY_H
#define GENOMEASSEMBLYREGISTRY_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMutex>
#include <QtCore/QObject>

#include <U2Core/global.h>
#include <U2Core/GUrl.h>
#include <U2Core/Task.h>

class QWidget;

namespace U2 {

#define LIBRARY_SINGLE                "Single-end"
#define LIBRARY_PAIRED                "Paired-end"
#define LIBRARY_PAIRED_INTERLACED     "Paired-end (Interlaced)"
#define LIBRARY_PAIRED_UNPAIRED       "Paired-end (Unpaired files)"
#define LIBRARY_SANGER                "Sanger"
#define LIBRARY_PACBIO                "PacBio"

#define PAIR_TYPE_DEFAULT              "default"
#define PAIR_TYPE_MATE                 "mate"
#define PAIR_TYPE_MATE_HQ              "high-quality mate"

#define ORIENTATION_FR                 "fr"
#define ORIENTATION_RF                 "rf"
#define ORIENTATION_FF                 "ff"

class GenomeAssemblyAlgorithmMainWidget;

class U2ALGORITHM_EXPORT GenomeAssemblyGUIExtensionsFactory {
public:
    virtual ~GenomeAssemblyGUIExtensionsFactory() {}
    virtual GenomeAssemblyAlgorithmMainWidget* createMainWidget(QWidget* parent) = 0;
    virtual bool hasMainWidget() = 0;
};

class U2ALGORITHM_EXPORT GenomeAssemblyUtils {
public:

    static QStringList getPairTypes();
    static QStringList getOrientationTypes();
    static QString getDefaultOrientation(const QString& pairType);
    static QStringList getLibraryTypes();
    static bool isLibraryPaired(const QString& libName);
    static bool hasRightReads(const QString& libName);
    static QString getYamlLibraryName(const QString& libName, const QString& pairType);
};

/////////////////////////////////////////////////////////////
//Task

class U2ALGORITHM_EXPORT AssemblyReads {
public:
    AssemblyReads(const GUrl& left = GUrl(), const GUrl& right = GUrl(), const QString& libNumber = QString("1"), const QString& libType = PAIR_TYPE_DEFAULT, const QString& orientation = ORIENTATION_FR, const QString& libName = LIBRARY_SINGLE)
        :
         left(left)
        ,right(right)
        ,libNumber(libNumber)
        ,libType(libType)
        ,orientation(orientation)
        ,libName(libName)
        {}

        GUrl left;
        GUrl right;
        QString libNumber;
        QString libType;
        QString orientation;
        QString libName;
};

class U2ALGORITHM_EXPORT GenomeAssemblyTaskSettings {
public:
    GenomeAssemblyTaskSettings() : openView(false) {}

    void setCustomSettings(const QMap<QString, QVariant>& settings);
    QVariant getCustomValue(const QString& optionName, const QVariant& defaultVal) const;
    bool hasCustomValue(const QString & name) const;
    void setCustomValue(const QString& optionName, const QVariant& val);

public:
    QList<AssemblyReads> reads;
    GUrl outDir;
    QString algName;
    bool openView;

private:
    QMap<QString, QVariant> customSettings;
};

class U2ALGORITHM_EXPORT GenomeAssemblyTask : public Task {
    Q_OBJECT
public:
    GenomeAssemblyTask(const GenomeAssemblyTaskSettings& settings, TaskFlags flags = TaskFlags_FOSCOE);
    virtual ~GenomeAssemblyTask() {}
    bool hasResult() const {return !resultUrl.isEmpty();}
    QString getResultUrl() const;
    const GenomeAssemblyTaskSettings& getSettings() const{return settings;}

protected:
    GenomeAssemblyTaskSettings settings;
    QString resultUrl;
};

class U2ALGORITHM_EXPORT GenomeAssemblyTaskFactory {
public:
    virtual GenomeAssemblyTask* createTaskInstance(const GenomeAssemblyTaskSettings& settings) = 0;
    virtual ~GenomeAssemblyTaskFactory() {}
};

#define GENOME_ASSEMBLEY_TASK_FACTORY(c) \
public: \
    static const QString taskName; \
class Factory : public GenomeAssemblyTaskFactory { \
public: \
    Factory() { } \
    GenomeAssemblyTask* createTaskInstance(const GenomeAssemblyTaskSettings& s) { return new c(s); } \
};


///////////////////////////////////////////////////////////////
//Registry
class U2ALGORITHM_EXPORT GenomeAssemblyAlgorithmEnv {
public:
    GenomeAssemblyAlgorithmEnv(const QString &id,
        GenomeAssemblyTaskFactory *tf ,
        GenomeAssemblyGUIExtensionsFactory *guiExt,
        const QStringList &readsFormats);

    virtual ~GenomeAssemblyAlgorithmEnv();

    const QString& getId()  const {return id;}
    QStringList getReadsFormats() const { return readsFormats; }

    GenomeAssemblyTaskFactory* getTaskFactory() const {return taskFactory;}
    GenomeAssemblyGUIExtensionsFactory* getGUIExtFactory() const {return guiExtFactory;}

private:
    Q_DISABLE_COPY(GenomeAssemblyAlgorithmEnv);

protected:
    QString id;
    GenomeAssemblyTaskFactory* taskFactory;
    GenomeAssemblyGUIExtensionsFactory* guiExtFactory;
    QStringList readsFormats;
};
class U2ALGORITHM_EXPORT GenomeAssemblyAlgRegistry : public QObject {
    Q_OBJECT
public:
    GenomeAssemblyAlgRegistry(QObject* pOwn = 0);
    ~GenomeAssemblyAlgRegistry();

    bool registerAlgorithm(GenomeAssemblyAlgorithmEnv* env);
    GenomeAssemblyAlgorithmEnv* unregisterAlgorithm(const QString& id);
    GenomeAssemblyAlgorithmEnv* getAlgorithm(const QString& id) const;

    QStringList getRegisteredAlgorithmIds() const;
private:
    mutable QMutex mutex;
    QMap<QString, GenomeAssemblyAlgorithmEnv*> algorithms;

    Q_DISABLE_COPY(GenomeAssemblyAlgRegistry);
};

} // namespace

#endif // GENOMEASSEMBLYREGISTRY_H
