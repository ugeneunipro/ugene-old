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

#ifndef _U2_BAM_CONVERT_TO_SQLITE_TASK_H_
#define _U2_BAM_CONVERT_TO_SQLITE_TASK_H_

#include <U2Core/GUrl.h>
#include <U2Core/Task.h>
#include <U2Core/U2Assembly.h>
#include <U2Core/U2Type.h>

#include "LoadBamInfoTask.h"

namespace U2 {

class U2DbiRef;

namespace BAM {

class ConvertToSQLiteTask : public Task {
    Q_OBJECT
public:
    ConvertToSQLiteTask(const GUrl &sourceUrl, const U2DbiRef &dstDbiRef, BAMInfo& bamInfo, bool sam);
    virtual void run();

    GUrl getDestinationUrl() const;
    QList<U2Assembly> getAssemblies() const;

private:
    const GUrl sourceUrl;
    const U2DbiRef dstDbiRef;
    BAMInfo bamInfo;

    bool sam;

    QMap<int, U2Assembly> assemblies;
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_CONVERT_TO_SQLITE_TASK_H_
