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

#include "AssemblyConsensusArea.h"
#include "AssemblyConsensusTask.h"
#include "AssemblyBrowser.h"

#include <QtGui/QPainter>

#include <U2Core/U2AssemblyUtils.h>

#include <U2Algorithm/AssemblyConsensusAlgorithmRegistry.h>
#include <U2Algorithm/BuiltInAssemblyConsensusAlgorithms.h>

namespace U2 {


AssemblyConsensusArea::AssemblyConsensusArea(AssemblyBrowserUi * ui)
    : AssemblySequenceArea(ui)
{
    setToolTip(tr("Consensus sequence"));
    connect(&consensusTaskRunner, SIGNAL(si_finished()), SLOT(sl_redraw()));
}

bool AssemblyConsensusArea::canDrawSequence() {
    return !getModel()->isEmpty();
}

QByteArray AssemblyConsensusArea::getSequenceRegion(U2OpStatus &os) {
    Q_UNUSED(os);
    return consensusTaskRunner.getResult();
}

void AssemblyConsensusArea::launchConsensusCalculation() {
    if(areCellsVisible()) {
        AssemblyConsensusTaskSettings settings;
        settings.region = getVisibleRegion();
        settings.model = getModel();
        
        // TODO: ability to choose algorithm
        AssemblyConsensusAlgorithmRegistry * registry = AppContext::getAssemblyConsensusAlgorithmRegistry();
        AssemblyConsensusAlgorithmFactory * f = registry->getAlgorithmFactory(BuiltInAssemblyConsensusAlgorithms::DEFAULT_ALGO);
        settings.consensusAlgorithm = f->createAlgorithm();
        previousRegion = settings.region;
        consensusTaskRunner.run(new AssemblyConsensusTask(settings));
    }
    sl_redraw();
}

void AssemblyConsensusArea::sl_offsetsChanged() {
    if(areCellsVisible() && getVisibleRegion() != previousRegion) {
        launchConsensusCalculation();
    }
}

void AssemblyConsensusArea::sl_zoomPerformed() {
    launchConsensusCalculation();
}

void AssemblyConsensusArea::drawSequence(QPainter &p) {
    if(! consensusTaskRunner.isFinished()) {
        p.drawText(rect(), Qt::AlignCenter, tr("Calculating consensus..."));
    } else {
        AssemblySequenceArea::drawSequence(p);
    }
}


} //ns
