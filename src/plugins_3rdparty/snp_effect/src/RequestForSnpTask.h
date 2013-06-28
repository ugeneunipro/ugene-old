/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_REQUEST_FOR_SNP_TASK_
#define _U2_REQUEST_FOR_SNP_TASK_

#include <QtCore/QUrl>

#include <U2Core/ExternalToolRunTask.h>
#include <U2Core/Task.h>
#include <U2Core/U2Variant.h>

namespace U2 {

class SnpResponseLogParser : public ExternalToolLogParser
{
public:
    SnpResponseLogParser( );

    void                parseOutput( const QString &partOfLog );
    QVariantMap         getResult( );

private:
    QVariantMap         result;
};

class RequestForSnpTask :       public Task
{
public:
                                RequestForSnpTask( const QString &scriptPath,
                                    const QVariantMap &inputData, const U2Variant& var );

    virtual QVariantMap         getResult( );
    const U2Variant&            getVariant( ){return variant;};

private:

    const QVariantMap           inputData;
    const QString               scriptPath;
    ExternalToolRunTask *       requestTask;
    SnpResponseLogParser        responseLogParser;
    U2Variant                   variant;
};

} // U2

#endif // _U2_REQUEST_FOR_SNP_TASK_