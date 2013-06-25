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

#include <U2Core/U2SafePoints.h>

#include "TBP_TATA.h"
#include "AnalyzeTataBoxesTask.h"

namespace U2 {

AnalyzeTataBoxesTask::AnalyzeTataBoxesTask( const QString &_seq )
    : Task( "Get SNP effect on tata boxes", TaskFlag_None ), seq( _seq )
{
    SAFE_POINT( !seq.isEmpty( ), "Empty sequence is supplied", );
}

void AnalyzeTataBoxesTask::run( )
{
    QByteArray tmpSequence = seq.toLocal8Bit( );
    // TODO: check if the other function from "TBP_TATA.h" is useful too
    double unknownNumber = TBP_NatTATA_95conf_interval( tmpSequence.data( ) );
    // TODO: construct report here
    report = "Some important number: " + QString::number( unknownNumber );
}

QString AnalyzeTataBoxesTask::getReport( )
{
    return report;
}

} // namespace U2