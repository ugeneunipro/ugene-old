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

#include <U2Core/AppContext.h>
#include "CMDLineUtils.h"
#include "CMDLineCoreOptions.h"

namespace U2 {

/***************************************************
* CMDLineRegistryUtils
***************************************************/
int CMDLineRegistryUtils::getParameterIndex( const QString & paramName, int startWith ) {
    QList<StringPair> params;
    setCMDLineParams( params );
    int sz = params.size();
    for( int i = qMax( 0, startWith ); i < sz; ++i ) {
        if( params[i].first == paramName ) {
            return i;
        }
    }
    return -1;
}

QStringList CMDLineRegistryUtils::getParameterValues( const QString & paramName, int startWith ) {
    QList<StringPair> params;
    setCMDLineParams( params );
    QStringList res;
    int sz = params.size();
    int paramIdx = getParameterIndex( paramName, startWith );
    if( -1 == paramIdx ) { // no such parameter
        return res;
    }
    for( int i = paramIdx; i < sz; ++i ) {
        res << params[i].second;
        if( i + 1 < sz && !params[i + 1].first.isEmpty() ) {
            break;
        }
    }
    return res;
}

QStringList CMDLineRegistryUtils::getParameterValuesByWords( const QString & paramName, int startWith ) {
    QStringList words;
    QStringList res = getParameterValues( paramName, startWith );
    QStringList::const_iterator it = res.constBegin();
    while( it != res.constEnd() ) {
        words << it->split( QRegExp("\\s"), QString::SkipEmptyParts );
        ++it;
    }
    return words;
}

QStringList CMDLineRegistryUtils::getPureValues( int startWithIdx ) {
    QList<StringPair> params;
    setCMDLineParams( params );
    QStringList res;
    int sz = params.size();
    for( int i = qMax( 0, startWithIdx ); i < sz; ++i ) {
        const StringPair & currentPair = params[i];
        if( currentPair.first.isEmpty() ) {
            res << currentPair.second;
        } else {
            break;
        }
    }
    return res;
}

void CMDLineRegistryUtils::setCMDLineParams( QList<StringPair> & to ) {
    CMDLineRegistry * cmdlineRegistry = AppContext::getCMDLineRegistry();
    if( cmdlineRegistry != NULL ) {
        to = cmdlineRegistry->getParameters();
    }
}

/***************************************************
* CMDLineUtils
***************************************************/
void CMDLineUtils::init() {
    CMDLineCoreOptions::initHelp();
}

} // U2
