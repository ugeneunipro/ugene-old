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

#ifdef __GNUC__
    #include <stdlib.h>
#endif

#include <U2Core/U2SafePoints.h>
#include <U2Lang/WorkflowRunSerializedSchemeTask.h>

#include "SchemeWrapper.h"
#include "TextConversionUtils.h"

#include "SchemeHandle.h"

extern U2ErrorType processTask( U2::Task *task );

extern "C" {

U2SCRIPT_EXPORT U2ErrorType createScheme( const char *pathToScheme, SchemeHandle *scheme ) {
    U2ErrorType result = U2_OK;
    try {
        *scheme = new U2::SchemeWrapper( pathToScheme, &result );
    } catch ( const std::bad_alloc & ) {
        delete reinterpret_cast<U2::SchemeWrapper *>( *scheme );
        result = U2_NOT_ENOUGH_MEMORY;
    }
    return result;
}

U2SCRIPT_EXPORT U2ErrorType addReaderToScheme( SchemeHandle _scheme, const char *readerType,
    const char *inputFilePath, int maxExpectedNameLength, char *name )
{
    U2::SchemeWrapper *scheme = reinterpret_cast<U2::SchemeWrapper *>( _scheme );
    CHECK( NULL != scheme, U2_INVALID_SCHEME );
    CHECK( NULL != readerType && NULL != inputFilePath && !QString( inputFilePath ).isEmpty( )
        && NULL != name, U2_INVALID_STRING );
    QString readerName;
    U2ErrorType result = scheme->addReaderAndGetItsName( readerType, inputFilePath, readerName );
    CHECK( U2_OK == result, result );
    return U2::TextConversionUtils::qstringToCstring( readerName, maxExpectedNameLength, name );
}

U2SCRIPT_EXPORT U2ErrorType addWriterToScheme( SchemeHandle _scheme, const char *writerType,
    const char *outputFilePath, int maxExpectedNameLength, char *name )
{
    U2::SchemeWrapper *scheme = reinterpret_cast<U2::SchemeWrapper *>( _scheme );
    CHECK( NULL != scheme, U2_INVALID_SCHEME );
    CHECK( NULL != writerType && NULL != outputFilePath && !QString( outputFilePath ).isEmpty( )
        && NULL != name, U2_INVALID_STRING );
    QString writerName;
    U2ErrorType result = scheme->addWriterAndGetItsName( writerType, outputFilePath, writerName );
    CHECK( U2_OK == result, result );
    return U2::TextConversionUtils::qstringToCstring( writerName, maxExpectedNameLength, name );
}

U2SCRIPT_EXPORT U2ErrorType setSchemeElementAttribute( SchemeHandle _scheme,
    const char *elementName, const char *attributeName, const char *attributeValue )
{
    U2::SchemeWrapper *scheme = reinterpret_cast<U2::SchemeWrapper *>( _scheme );
    CHECK( NULL != scheme, U2_INVALID_SCHEME );
    CHECK( NULL != elementName && NULL != attributeName && NULL != attributeValue,
        U2_INVALID_STRING );
    return scheme->setElementAttribute( elementName, attributeName, attributeValue );
}

U2SCRIPT_EXPORT U2ErrorType getSchemeElementAttribute(   SchemeHandle _scheme,
    const char *elementName, const char *attributeName, int maxExpectedNameLength,
    char *attributeValue )
{
    U2::SchemeWrapper *scheme = reinterpret_cast<U2::SchemeWrapper *>( _scheme );
    CHECK( NULL != scheme, U2_INVALID_SCHEME );
    CHECK( NULL != elementName && NULL != attributeName && NULL != attributeValue,
        U2_INVALID_STRING );
    QString value;
    U2ErrorType result = scheme->getElementAttribute( elementName, attributeName, value );
    CHECK( U2_OK == result, result );
    return U2::TextConversionUtils::qstringToCstring( value, maxExpectedNameLength, attributeValue );
}

U2SCRIPT_EXPORT U2ErrorType addElementToScheme( SchemeHandle _scheme, const char *elementType,
    int maxExpectedNameLength, char *name )
{
    U2::SchemeWrapper *scheme = reinterpret_cast<U2::SchemeWrapper *>( _scheme );
    CHECK( NULL != scheme, U2_INVALID_SCHEME );
    CHECK( NULL != elementType, U2_INVALID_STRING );

    QString elementName;
    U2ErrorType result = scheme->addNewElementAndGetItsName( elementType, elementName );
    CHECK( U2_OK == result, result );
    return U2::TextConversionUtils::qstringToCstring( elementName, maxExpectedNameLength, name );
}

U2SCRIPT_EXPORT U2ErrorType addFlowToScheme( SchemeHandle _scheme, const char *srcElementName,
    const char *srcPortName, const char *dstElementName, const char *dstPortName )
{
    U2::SchemeWrapper *scheme = reinterpret_cast<U2::SchemeWrapper *>( _scheme );
    CHECK( NULL != scheme, U2_INVALID_SCHEME );
    CHECK( NULL != srcElementName && NULL != srcPortName && NULL != dstElementName
        && NULL != dstPortName, U2_INVALID_STRING );
    return scheme->addFlow( srcElementName, srcPortName, dstElementName, dstPortName );
}

U2SCRIPT_EXPORT U2ErrorType addSchemeActorsBinding( SchemeHandle _scheme,
    const char *srcElementName, const char *srcSlotName, const char *dstElementName,
    const char *dstPortAndSlotNames )
{
    U2::SchemeWrapper *scheme = reinterpret_cast<U2::SchemeWrapper *>( _scheme );
    CHECK( NULL != scheme, U2_INVALID_SCHEME );
    CHECK( NULL != srcElementName && NULL != srcSlotName && NULL != dstElementName
        && NULL != dstPortAndSlotNames, U2_INVALID_STRING );
    return scheme->addActorsBinding( srcElementName, srcSlotName, dstElementName,
        dstPortAndSlotNames );
}

U2SCRIPT_EXPORT U2ErrorType saveSchemeToFile( SchemeHandle _scheme, const char *path ) {
    U2::SchemeWrapper *scheme = reinterpret_cast<U2::SchemeWrapper *>( _scheme );
    CHECK( NULL != scheme, U2_INVALID_SCHEME );
    CHECK( NULL != path, U2_INVALID_STRING );
    QString pathString( path );
    return scheme->saveToFile( pathString );
}

U2SCRIPT_EXPORT void releaseScheme( SchemeHandle _scheme ) {
    U2::SchemeWrapper *scheme = reinterpret_cast<U2::SchemeWrapper *>( _scheme );
    delete scheme;
}

U2SCRIPT_EXPORT U2ErrorType createSas( const char *algorithmType, const char *inputPath,
    const char *outputPath, SchemeHandle *scheme )
{
    CHECK( NULL != algorithmType, U2_INVALID_STRING );
    return U2::SchemeWrapper::createSas( algorithmType, inputPath, outputPath,
        reinterpret_cast<U2::SchemeWrapper **>( scheme ) );
}

U2SCRIPT_EXPORT U2ErrorType launchScheme( SchemeHandle _scheme, int *outputFilesCount,
    char ***outputFileNames )
{
    U2::SchemeWrapper *scheme = reinterpret_cast<U2::SchemeWrapper *>( _scheme );
    if ( NULL == scheme ) {
        return U2_INVALID_SCHEME;
    }
    QString pathToScheme;
    U2ErrorType result = scheme->saveToFile( pathToScheme );
    CHECK( U2_OK == result, result );
    U2::BaseSerializedSchemeRunner *launchSchemeTask = NULL;
    try {
        QStringList outputFiles;
        launchSchemeTask = new U2::WorkflowRunSerializedSchemeTask( pathToScheme, NULL,
            outputFiles );
        result = processTask( launchSchemeTask );
        CHECK( U2_OK == result, result );
        const int countOfFiles = outputFiles.size( );
        *outputFileNames = ( char ** )malloc( countOfFiles * sizeof( char ** ) );
        if ( NULL == *outputFileNames ) {
            throw std::bad_alloc();
        }
        for ( int i = 0; i < countOfFiles; ++i ) {
            const int pathLength = outputFiles[i].size( ) + 1;
            ( *outputFileNames )[i] = ( char * )malloc( pathLength );
            // handle failed memory allocation
            if ( NULL == ( *outputFileNames )[i] ) {
                for ( int j = 0; j < i; ++j ) {
                    free( ( *outputFileNames )[i] );
                }
                free( *outputFileNames );
                throw std::bad_alloc();
            }
            QByteArray tmp = outputFiles[i].toLocal8Bit( );
            memcpy( ( *outputFileNames )[i], tmp.constData( ), pathLength );
        }
        *outputFilesCount = countOfFiles;
    } catch ( const std::bad_alloc & ) {
        delete launchSchemeTask;
        return U2_NOT_ENOUGH_MEMORY;
    }
    return U2_OK;
}

U2SCRIPT_EXPORT U2ErrorType launchSas( const char *algorithmType, const char *inputPath,
    const char *outputPath, int *outputFilesCount, char ***outputFileNames )
{
    CHECK( NULL != algorithmType && NULL != inputPath && NULL != outputPath, U2_INVALID_STRING );
    U2::SchemeWrapper *scheme = NULL;
    U2ErrorType result = U2::SchemeWrapper::createSas( algorithmType, inputPath, outputPath,
        &scheme );
    CHECK( U2_OK == result, result );
    result = launchScheme( scheme, outputFilesCount, outputFileNames );

    delete scheme;
    return U2_OK;
}

}