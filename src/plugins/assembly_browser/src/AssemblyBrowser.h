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
#include <U2Core/U2Dbi.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2Assembly.h>
#include <U2Core/AssemblyObject.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Gui/MainWindow.h>
#include <U2Gui/ObjectViewModel.h>

namespace U2 {

class AssemblyModel {
public:
    //TODO refactor 
    AssemblyModel(const DbiHandle & dbiHandle);

    bool isEmpty() const;

    QList<U2AssemblyRead> getReadsFromAssembly(int assIdx, const U2Region & r, qint64 minRow, qint64 maxRow, U2OpStatus & os);

    qint64 countReadsInAssembly(int assIdx, const U2Region & r, U2OpStatus & os);

    qint64 getModelLength(U2OpStatus & os);

    qint64 getModelHeight(U2OpStatus & os);

    void addAssembly(U2AssemblyDbi * dbi, const U2Assembly & assm);

    bool hasReference() const;

    void setReference(U2SequenceDbi * dbi, const U2Sequence & seq);

    QByteArray getReferenceRegion(const U2Region& region, U2OpStatus& os);

    const DbiHandle & getDbiHandle() const {return dbiHandle;}

private:
    const static qint64 NO_VAL = -1;
    //TODO: track model changes and invalidate caches accordingly
    qint64 cachedModelLength;
    qint64 cachedModelHeight;

    U2Sequence reference;
    U2SequenceDbi * referenceDbi;

    QList<U2Assembly> assemblies;
    QList<U2AssemblyDbi *> assemblyDbis;

    DbiHandle dbiHandle; 
};

class AssemblyBrowserUi;
class PositionSelector;

class AssemblyBrowser : public GObjectView {
    Q_OBJECT
public:
    AssemblyBrowser(AssemblyObject * o);
    
    // from GObjectView
    virtual void buildStaticToolbar(QToolBar* tb);
    virtual void buildStaticMenu(QMenu* m);
    
    // returns error string
    QString tryAddObject(GObject * obj);
    
    int getCellWidth() const;
    qint64 calcPixelCoord(qint64 asmCoord) const;
    qint64 calcAsmCoordX(qint64 pixCoord) const;
    qint64 calcAsmCoordY(qint64 pixCoord) const;
    qint64 calcAsmPosX(qint64 pixPosX) const;
    qint64 calcAsmPosY(qint64 pixPosY) const;
    qint64 calcPainterOffset(qint64 xAsmCoord) const;
    
    qint64 basesCanBeVisible() const;
    qint64 rowsCanBeVisible() const;

    qint64 basesVisible() const;
    qint64 rowsVisible() const;

    bool areReadsVisible() const;
    bool areCellsVisible() const;
    bool areLettersVisible() const;

    inline QSharedPointer<AssemblyModel> getModel() const {return model;}
    inline double getZoomFactor() const {return zoomFactor;}
    inline QFont getFont() const {return font;}

    inline qint64 getXOffsetInAssembly() const {return xOffsetInAssembly; }
    inline qint64 getYOffsetInAssembly() const {return yOffsetInAssembly; }

    void setXOffsetInAssembly(qint64 x); 
    void setYOffsetInAssembly(qint64 y);
    void setOffsetsInAssembly(qint64 x, qint64 y);
    qint64 normalizeXoffset(qint64 x)const;
    qint64 normalizeYoffset(qint64 y)const;
    
    void adjustOffsets(qint64 dx, qint64 dy);
    
    void setFocusToPosSelector();
    
signals:
    void si_zoomOperationPerformed();
    void si_offsetsChanged();

protected:
    virtual QWidget * createWidget();
    virtual bool eventFilter(QObject*, QEvent*);
    
private slots:
    void sl_loadAssembly();
    void sl_assemblyLoaded();
    void sl_onPosChangeRequest(int);
    void sl_changeOverviewType();
    void sl_onShowCoordsOnRulerChanged();
    
public slots:
    void sl_zoomIn();
    void sl_zoomOut();

private:
    void initFont();
    void setupActions();
    void updateZoomingActions(bool enableZoomIn);
    void updateOverviewTypeActions();
    void clear();
    int zoomInFromSize(int oldCellSize);
    
    AssemblyBrowserUi * ui;

    AssemblyObject * gobject;
    U2OpStatusImpl dbiOpStatus;
    QSharedPointer<AssemblyModel> model;

    double zoomFactor;
    QFont font;

    qint64 xOffsetInAssembly;
    qint64 yOffsetInAssembly;

    QAction * zoomInAction;
    QAction * zoomOutAction;
    QAction * posSelectorAction;
    PositionSelector * posSelector;
    QList<QAction*> overviewScaleTypeActions;
    QAction * showCoordsOnRulerAction;
    
    const static double INITIAL_ZOOM_FACTOR;
    const static double ZOOM_MULT;

    const static int MAX_CELL_WIDTH = 300;
    const static int LETTER_VISIBLE_WIDTH = 7;
    const static int CELL_VISIBLE_WIDTH = 1;
}; 


class AssemblyOverview;
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
    inline AssemblyOverview * getOverview() const {return overview;}
    inline AssemblyRuler * getRuler() const {return ruler;}
    inline AssemblyReferenceArea * getReferenceArea() const {return referenceArea;}
    
private:
    AssemblyOverview * overview;        
    AssemblyReferenceArea * referenceArea;
    AssemblyDensityGraph * densityGraph;
    AssemblyRuler * ruler;
    AssemblyReadsArea * readsArea;
    
    AssemblyBrowser * browser;
};

/**
 * Dumps error to log and returns true if status contains an erro.r 
 */
bool checkAndLogError(const U2OpStatusImpl & status);

/**
 * Returns read length calculated with respect to CIGAR.
 */
qint64 countReadLength(qint64 realLen, const QList<U2CigarToken> & cigar);

/**
 * Returns read sequence. If read has no embedded sequence -> gets it from dbi.
 */
QByteArray getReadSequence(U2Dbi * dbi, const U2AssemblyRead & read, U2OpStatus & os);

} //ns

#endif 
