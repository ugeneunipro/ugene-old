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

#include "RSnpToolsTask.h"

namespace U2 {

RSnpToolsTask::RSnpToolsTask( const QString &_seq1, const QString &_seq2 )
    : BaseRequestForSnpTask( ), seq1( _seq1 ), seq2( _seq2 )
{

}

QString RSnpToolsTask::getReport( )
{
    // TODO: parse the `responseString` field
    return responseString;
}

QUrl RSnpToolsTask::createRequestString( )
{
    QUrl result( "http://samurai.bionet.nsc.ru/cgi-bin/03/programs/rsnp_lin/rsnpd.pl" );
    result.addQueryItem( "b1", "Calculate" );
    result.addQueryItem( "snpf1", seq1 );
    result.addQueryItem( "snpf2", seq2 );
    return result;
}

} // namespace U2