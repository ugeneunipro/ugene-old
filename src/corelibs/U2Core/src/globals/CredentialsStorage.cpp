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

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/U2SafePoints.h>

#include "CredentialsStorage.h"

namespace U2 {

const char Credentials::SEPARATOR = '|';

Credentials::Credentials()
{
}

Credentials::Credentials(const QString& login, const QString& password) :
    login(login),
    password(password)
{
}

bool Credentials::isValid() const {
    return !login.isNull() && !password.isNull();
}

QByteArray Credentials::serialize() const {
    QByteArray res = login.toLatin1().toBase64() + SEPARATOR + password.toLatin1().toBase64();
    return res;
}

Credentials Credentials::deserialize(const QByteArray& data) {
    Credentials res;

    const QList<QByteArray> parts = data.split(SEPARATOR);
    CHECK(parts.size() == 2, res);

    res.login = QByteArray::fromBase64(parts.first());
    res.password = QByteArray::fromBase64(parts.last());

    return res;
}

const QString CredentialsStorage::SETTINGS_PATH = "/user_credentials/";

CredentialsStorage::CredentialsStorage() {
    QStringList keys = AppContext::getSettings()->getAllKeys(SETTINGS_PATH);
    foreach(const QString& key, keys) {
        Credentials value = Credentials::deserialize(AppContext::getSettings()->getValue(SETTINGS_PATH + key).toByteArray());
        if (value.isValid()) {
            registry.insert(key, value);
        }
    }
}

void CredentialsStorage::addEntry(const QString& resourceId, const Credentials& entry, bool rememberEntry) {
    registry.insert(resourceId, entry);
    if (rememberEntry) {
        remember(resourceId, entry);
    } else {
        forget(resourceId);
    }
}

void CredentialsStorage::removeEntry(const QString& resourceId) {
    registry.remove(resourceId);
    forget(resourceId);
}

Credentials CredentialsStorage::getEntry(const QString& resourceId) const {
    return registry.value(resourceId);
}

bool CredentialsStorage::contains(const QString& resourceId) const {
    return registry.contains(resourceId);
}

bool CredentialsStorage::isRemembered(const QString& resourceId) const {
    return AppContext::getSettings()->contains(SETTINGS_PATH + resourceId);
}

void CredentialsStorage::remember(const QString& resourceId, const Credentials& entry) {
    AppContext::getSettings()->setValue(SETTINGS_PATH + resourceId, entry.serialize());
}

void CredentialsStorage::forget(const QString resourceId) {
    AppContext::getSettings()->remove(SETTINGS_PATH + resourceId);
}

}   // namespce U2
