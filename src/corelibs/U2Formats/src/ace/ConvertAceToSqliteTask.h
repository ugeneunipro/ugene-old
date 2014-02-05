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

#ifndef _U2_CONVERT_ACE_TO_SQLITE_TASK_H_
#define _U2_CONVERT_ACE_TO_SQLITE_TASK_H_

#include <U2Core/GUrl.h>
#include <U2Core/Task.h>
#include <U2Core/U2Assembly.h>
#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2OpStatusUtils.h>

#include "ace/AceImportUtils.h"

namespace U2 {

class ConvertAceToSqliteTask : public Task {
    Q_OBJECT
public:
    ConvertAceToSqliteTask(const GUrl &sourceUrl, const GUrl &destinationUrl);
    virtual void run();
    virtual ReportResult report();

    const GUrl &getDestinationUrl() const;

private:
    qint64 importAssemblies(IOAdapter &ioAdapter);
    qint64 packReads();
    void updateAttributeDbi();
    void removeCorruptedAssembly();

    bool append;        // if it is appended, it won't be removed on error
    const GUrl sourceUrl;
    const GUrl destinationUrl;
    U2Dbi* dbi;
    U2Assembly corruptedAssembly;

    QMap<int, Assembly::Sequence> references;
    QMap<int, U2Assembly> assemblies;
    QMap<int, U2AssemblyReadsImportInfo> importInfos;
};

}   // namespace U2

#endif // _U2_CONVERT_ACE_TO_SQLITE_TASK_H_
