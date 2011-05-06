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

#ifndef __ASSEMBLY_BROWSER_H__
#define __ASSEMBLY_BROWSER_H__

#include <assert.h>

#include <QtCore/QSharedPointer>
#include <QtCore/QByteArray>
#include <QtCore/QListIterator>
#include <U2Core/U2Dbi.h>
#include <U2Core/U2Assembly.h>
#include <U2Core/AssemblyObject.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Gui/MainWindow.h>
#include <U2Gui/ObjectViewModel.h>

#include "AssemblyModel.h"
#include "CoveredRegionsManager.h"

namespace U2 {

class AssemblyBrowserUi;
class PositionSelector;

class AssemblyBrowser : public GObjectView {
    Q_OBJECT
public:
    AssemblyBrowser(AssemblyObject * o);
    
    // from GObjectView
    virtual void buildStaticToolbar(QToolBar* tb);
    virtual void buildStaticMenu(QMenu* m);
    
    void setGlobalCoverageInfo(const CoverageInfo & info);
    QList<CoveredRegion> getCoveredRegions() const;

    // asm coords <-> pix coords functions
    qint64 calcPixelCoord(qint64 asmCoord) const;
    qint64 calcAsmCoordX(qint64 pixCoord) const;
    qint64 calcAsmCoordY(qint64 pixCoord) const;
    qint64 calcAsmPosX(qint64 pixPosX) const;
    qint64 calcAsmPosY(qint64 pixPosY) const;
    qint64 calcPainterOffset(qint64 xAsmCoord) const;
    
    // cells utility functions
    int getCellWidth() const;
    qint64 basesCanBeVisible() const;
    qint64 rowsCanBeVisible() const;

    qint64 basesVisible() const;
    qint64 rowsVisible() const;

    bool areReadsVisible() const;
    bool areCellsVisible() const;
    bool areLettersVisible() const;

    // offsets in assembly
    inline qint64 getXOffsetInAssembly() const {return xOffsetInAssembly; }
    inline qint64 getYOffsetInAssembly() const {return yOffsetInAssembly; }
    
    void setXOffsetInAssembly(qint64 x); 
    void setYOffsetInAssembly(qint64 y);
    void setOffsetsInAssembly(qint64 x, qint64 y);
    qint64 normalizeXoffset(qint64 x)const;
    qint64 normalizeYoffset(qint64 y)const;

    void adjustOffsets(qint64 dx, qint64 dy);

    void navigateToRegion(const U2Region & region);

    // utility functions for zooming
    bool canPerformZoomIn() const {return zoomInAction->isEnabled();}
    bool canPerformZoomOut() const {return zoomOutAction->isEnabled();}
    int zoomInFromSize(int oldCellSize);
    int zoomOutFromSize(int oldCellSize);
    void updateZoomingActions();
    
    // other
    inline QSharedPointer<AssemblyModel> getModel() const {return model;}
    inline QFont getFont() const {return font;}
    void setFocusToPosSelector();

public slots:
    void sl_zoomIn(const QPoint & pos = QPoint());
    void sl_zoomOut(const QPoint & pos = QPoint());

signals:
    void si_offsetsChanged();
    void si_zoomOperationPerformed();

protected:
    virtual QWidget * createWidget();
    virtual bool eventFilter(QObject*, QEvent*);
    
private slots:
    void sl_assemblyLoaded();
    void sl_onPosChangeRequest(int);
    void sl_changeOverviewType();
    void sl_onShowCoordsOnRulerChanged();
    void sl_saveScreenshot();
    void sl_showContigInfo();
    
private:
    void initFont();
    void setupActions();
    void updateOverviewTypeActions();
    void clear();
    // returns error string
    QString tryAddObject(GObject * obj);

private:
    AssemblyBrowserUi * ui;

    AssemblyObject * gobject;
    U2OpStatusImpl dbiOpStatus;
    QSharedPointer<AssemblyModel> model;

    double zoomFactor;
    QFont font;

    qint64 xOffsetInAssembly;
    qint64 yOffsetInAssembly;

    CoverageInfo coverageInfo;
    CoveredRegionsManager coveredRegionsManager;
    
    QAction * zoomInAction;
    QAction * zoomOutAction;
    QAction * posSelectorAction;
    PositionSelector * posSelector;
    QList<QAction*> overviewScaleTypeActions;
    QAction * showCoordsOnRulerAction;
    QAction * saveScreenShotAction;
    QAction * showInfoAction;
    
    const static int MAX_CELL_WIDTH = 300;
    const static double INITIAL_ZOOM_FACTOR;
    const static double ZOOM_MULT;

    const static int LETTER_VISIBLE_WIDTH = 7;
    const static int CELL_VISIBLE_WIDTH = 1;
}; 


class AssemblyOverview;
class ZoomableAssemblyOverview;
class AssemblyReferenceArea;
class AssemblyDensityGraph;
class AssemblyRuler;
class AssemblyReadsArea;

class AssemblyBrowserUi : public QWidget {
    Q_OBJECT
public:
    AssemblyBrowserUi(AssemblyBrowser * browser);

    inline QSharedPointer<AssemblyModel> getModel() const {return browser->getModel();}
    inline AssemblyBrowser * getWindow() const {return browser;}

    inline AssemblyReadsArea * getReadsArea() const {return readsArea;}
    inline ZoomableAssemblyOverview * getOverview() const {return zoomableOverview;}
    inline AssemblyRuler * getRuler() const {return ruler;}
    inline AssemblyReferenceArea * getReferenceArea() const {return referenceArea;}
    
private:
    ZoomableAssemblyOverview * zoomableOverview;        
    AssemblyReferenceArea * referenceArea;
    AssemblyDensityGraph * densityGraph;
    AssemblyRuler * ruler;
    AssemblyReadsArea * readsArea;
    
    AssemblyBrowser * browser;
};


} //ns

#endif 
