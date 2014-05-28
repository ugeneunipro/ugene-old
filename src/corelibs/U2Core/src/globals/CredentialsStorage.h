/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_CREDENTIALS_STORAGE_H_
#define _U2_CREDENTIALS_STORAGE_H_

#include <QtCore/QMap>
#include <QtCore/QString>

#include <U2Core/global.h>

namespace U2 {

class U2CORE_EXPORT Credentials {
public:
    Credentials();
    Credentials(const QString& login, const QString& password);

    bool isValid() const;

    QByteArray serialize() const;
    static Credentials deserialize(const QByteArray& data);

    QString login;
    QString password;

private:
    static const char SEPARATOR;
};

class U2CORE_EXPORT CredentialsStorage {
public:
    CredentialsStorage();

    void addEntry(const QString& resourceId, const Credentials& entry, bool remember);
    void removeEntry(const QString& resourceId);
    Credentials getEntry(const QString& resourceId) const;
    bool contains(const QString& resourceId) const;
    bool isRemembered(const QString& resourceId) const;

private:
    static void remember(const QString& resourceId, const Credentials& entry);
    static void forget(const QString resourceId);

    QMap<QString, Credentials> registry;

    static const QString SETTINGS_PATH;
};

}   // namespace U2

#endif // _U2_CREDENTIALS_STORAGE_H_
