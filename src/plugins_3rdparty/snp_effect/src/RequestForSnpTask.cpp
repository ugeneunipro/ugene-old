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

#include "RequestForSnpTask.h"
#include "SnpRequestKeys.h"

#include <U2Core/ExternalToolRunTask.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
//BaseSnpAnnotationTask
BaseSnpAnnotationTask::BaseSnpAnnotationTask( const QVariantMap& _inputData, const U2Variant& var, const QString& description )
:Task(!description.isEmpty() ? description : QString("SNP Annotation task"), TaskFlag_NoRun)
,inputData(_inputData)
,variant(var)
{
    if (inputData.contains(SnpRequestKeys::SNP_FEATURE_ID_KEY)){
        featureId = inputData.take(SnpRequestKeys::SNP_FEATURE_ID_KEY).toByteArray();
    }
}


//////////////////////////////////////////////////////////////////////////
//RequestForSnpTask
RequestForSnpTask::RequestForSnpTask( const QString &_scriptPath, const QVariantMap &_inputData, const U2Variant& var )
:BaseSnpAnnotationTask(_inputData, var, "Request to remote server for SNP analysis")
,scriptPath( _scriptPath )
,requestTask( NULL )
,responseLogParser( )
{
    QStringList pythonArguments( scriptPath );
    foreach ( QString key, inputData.keys( ) ) {
        SAFE_POINT( inputData[key].canConvert<QString>( ), "Invalid argument passed to script", );
        pythonArguments << key << inputData[key].toString( );
    }
    requestTask = new ExternalToolRunTask( "python", pythonArguments, (&responseLogParser) );
    addSubTask( requestTask );
}

QVariantMap RequestForSnpTask::getResult( )
{
    return responseLogParser.getResult( );
}
//////////////////////////////////////////////////////////////////////////
//SnpResponseLogParser
SnpResponseLogParser::SnpResponseLogParser( )
    : ExternalToolLogParser( )
{

}

void SnpResponseLogParser::parseOutput( const QString &partOfLog )
{
    lastPartOfLog = partOfLog.split( "\n", QString::SkipEmptyParts );
    foreach ( QString line, lastPartOfLog ) {
        line = line.trimmed( );
        if ( line.isEmpty( ) ) {
            continue;
        }
        int lineSeparatorPos = line.indexOf( SnpResponseKeys::DEFAULT_SEPARATOR );
        QString key = line.left( lineSeparatorPos );
        QString value = line.mid( lineSeparatorPos + 1 );
        result[key] = value;
        qDebug() << key << ":" << value;
    }
}

QVariantMap SnpResponseLogParser::getResult( )
{
    return result;
}

} // namespace U2
