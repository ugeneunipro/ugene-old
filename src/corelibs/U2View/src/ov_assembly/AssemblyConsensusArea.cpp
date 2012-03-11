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
#include <QtGui/QMouseEvent>

#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Algorithm/AssemblyConsensusAlgorithmRegistry.h>
#include <U2Algorithm/BuiltInAssemblyConsensusAlgorithms.h>

namespace U2 {

AssemblyConsensusArea::AssemblyConsensusArea(AssemblyBrowserUi * ui)
    : AssemblySequenceArea(ui), consensusAlgorithmMenu(NULL), consensusAlgorithm(NULL)
{
    setToolTip(tr("Consensus sequence"));
    connect(&consensusTaskRunner, SIGNAL(si_finished()), SLOT(sl_redraw()));

    AssemblyConsensusAlgorithmRegistry * registry = AppContext::getAssemblyConsensusAlgorithmRegistry();
    QString defaultId = BuiltInAssemblyConsensusAlgorithms::DEFAULT_ALGO;
    AssemblyConsensusAlgorithmFactory * f = registry->getAlgorithmFactory(defaultId);
    SAFE_POINT(f != NULL, QString("consensus algorithm factory %1 not found").arg(defaultId),);
    consensusAlgorithm = QSharedPointer<AssemblyConsensusAlgorithm>(f->createAlgorithm());

    createContextMenu();
}

void AssemblyConsensusArea::createContextMenu() {
    contextMenu = new QMenu(this);

    contextMenu->addMenu(getConsensusAlgorithmMenu());
    QAction * diffAction = contextMenu->addAction(tr("Show difference from reference"));
    diffAction->setCheckable(true);
    diffAction->setChecked(false);
    connect(diffAction, SIGNAL(triggered(bool)), SLOT(sl_drawDifferenceChanged(bool)));
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
        settings.consensusAlgorithm = consensusAlgorithm;
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
    } else if(consensusTaskRunner.getResult().size() != getVisibleRegion().length) {
        launchConsensusCalculation();
    } else {
        AssemblySequenceArea::drawSequence(p);
    }
}

QMenu * AssemblyConsensusArea::getConsensusAlgorithmMenu() {
    if(consensusAlgorithmMenu == NULL) {
        consensusAlgorithmMenu = new QMenu(tr("Consensus algorithm"));

        AssemblyConsensusAlgorithmRegistry * registry = AppContext::getAssemblyConsensusAlgorithmRegistry();
        QList<AssemblyConsensusAlgorithmFactory*> factories = registry->getAlgorithmFactories();

        foreach(AssemblyConsensusAlgorithmFactory *f, factories) {
            QAction * action = consensusAlgorithmMenu->addAction(f->getName());
            action->setCheckable(true);
            action->setChecked(f == consensusAlgorithm->getFactory());
            action->setData(f->getId());
            connect(consensusAlgorithmMenu, SIGNAL(triggered(QAction*)), SLOT(sl_consensusAlgorithmChanged(QAction*)));
        }
    }
    return consensusAlgorithmMenu;
}

void AssemblyConsensusArea::sl_consensusAlgorithmChanged(QAction * action) {
    QString id = action->data().toString();
    AssemblyConsensusAlgorithmRegistry * registry = AppContext::getAssemblyConsensusAlgorithmRegistry();
    AssemblyConsensusAlgorithmFactory * f = registry->getAlgorithmFactory(id);
    SAFE_POINT(f != NULL, QString("cannot change consensus algorithm, invalid id %1").arg(id),);

    consensusAlgorithm = QSharedPointer<AssemblyConsensusAlgorithm>(f->createAlgorithm());

    foreach(QAction* a, consensusAlgorithmMenu->actions()) {
        a->setChecked(a == action);
    }

    launchConsensusCalculation();
}

void AssemblyConsensusArea::sl_drawDifferenceChanged(bool drawDifference) {
    if(drawDifference) {
        setDiffCellRenderer();
    } else {
        setNormalCellRenderer();
    }
    sl_redraw();
}

void AssemblyConsensusArea::mousePressEvent(QMouseEvent *e) {
    if(e->button() == Qt::RightButton) {
        contextMenu->exec(QCursor::pos());
    }
}

} //ns
