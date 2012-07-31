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

#ifndef ExternalToolCfg_h__
#define ExternalToolCfg_h__

#include <U2Lang/Datatype.h>
#include <U2Lang/ConfigurationEditor.h>

#include <QtCore/qglobal.h>
#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtCore/QMap>


namespace U2 {

#define SEQ_WITH_ANNS QString("Sequence_with_annotations")

class U2LANG_EXPORT DataConfig {
public:
    QString attrName;
    QString type;
    QString format;
    QString description;

    bool operator ==(const DataConfig &other) const;

    static DocumentFormatId StringValue;
    static DocumentFormatId OutputFileUrl;
};

class U2LANG_EXPORT AttributeConfig {
public:
    QString attrName;
    QString type;
    QString description;
    //PropertyDelegate *delegate;

    bool operator ==(const AttributeConfig &other) const;
};

class U2LANG_EXPORT ExternalProcessConfig {
public:
    QList<DataConfig> inputs;
    QList<DataConfig> outputs;
    QList<AttributeConfig> attrs;
    QString cmdLine;
    QString name;
    QString description;
    QString templateDescription;
    QString filePath;

    bool operator ==(const ExternalProcessConfig &other) const;
};


class U2LANG_EXPORT ExternalToolCfgRegistry: public QObject {
    Q_OBJECT
public:
    ExternalToolCfgRegistry(QObject *p = NULL): QObject(p) {}
    bool registerExternalTool(ExternalProcessConfig *cfg) {
        if(configs.contains(cfg->name)) {
            return false;
        } else {
            configs.insert(cfg->name, cfg);
            return true;
        }
    }
    ExternalProcessConfig *getConfigByName(const QString& name) const {
        if(configs.contains(name)) {
            return configs.value(name);
        } else {
            return NULL;
        }
    }
    void unregisterConfig(const QString &name) {
        configs.remove(name);
    }
    QList<ExternalProcessConfig*> getConfigs() const {
        return configs.values();
    }

private:
    QMap<QString, ExternalProcessConfig*> configs;

};

}

#endif // ExternalToolCfg_h__
