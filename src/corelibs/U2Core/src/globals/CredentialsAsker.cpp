/**
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 * UGENE - Integrated Bioinformatics Tools.
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
#include <U2Core/PasswordStorage.h>
#include <U2Core/U2SafePoints.h>

#include "CredentialsAsker.h"

namespace U2 {

CredentialsAsker::~CredentialsAsker() {

}

void CredentialsAsker::saveCredentials(const QString &resourceUrl, const QString &password, bool remember) {
    PasswordStorage *storage = AppContext::getPasswordStorage();
    CHECK(NULL != storage, );

    storage->addEntry(resourceUrl, password, remember);
}

} // namespace U2
