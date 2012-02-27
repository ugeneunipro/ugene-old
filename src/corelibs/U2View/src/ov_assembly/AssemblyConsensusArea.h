/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#ifndef __ASSEMBLY_CONSENSUS_AREA_H__
#define __ASSEMBLY_CONSENSUS_AREA_H__

#include "AssemblyReferenceArea.h"

#include <U2Core/BackgroundTaskRunner.h>

namespace U2 {

class AssemblyBrowserUi;

class AssemblyConsensusArea: public AssemblySequenceArea {
    Q_OBJECT
public:
    AssemblyConsensusArea(AssemblyBrowserUi * ui);

protected:
    virtual QByteArray getSequenceRegion(U2OpStatus &os);
    virtual bool canDrawSequence();
    virtual void drawSequence(QPainter &p);

protected slots:
    virtual void sl_offsetsChanged();
    virtual void sl_zoomPerformed();

private:
    void launchConsensusCalculation();

    U2Region previousRegion;
    BackgroundTaskRunner<QByteArray> consensusTaskRunner;
};

} //ns

#endif
