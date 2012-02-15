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

#ifndef _U2_SMITH_WATERMAN_DIALOG_IMPL_H_
#define _U2_SMITH_WATERMAN_DIALOG_IMPL_H_

#include "SmithWatermanDialog.h"
#include <ui/ui_SmithWatermanDialogBase.h>

#include <QDialog>
#include <QCloseEvent>

#include <U2Algorithm/SmithWatermanTaskFactory.h>
#include <U2Core/SequenceWalkerTask.h>

#include <U2Algorithm/SubstMatrixRegistry.h>
#include <U2Algorithm/SWResultFilterRegistry.h>
#include <U2Algorithm/SmithWatermanTaskFactoryRegistry.h>

#include <U2Gui/RegionSelector.h>

namespace U2 {

class SmithWatermanDialog: public QDialog, public Ui::SmithWatermanDialogBase {
    Q_OBJECT
public:
    SmithWatermanDialog(QWidget* p, 
                        ADVSequenceObjectContext* ctx,
                        SWDialogConfig* dialogConfig);

private slots:
    void sl_bttnViewMatrix();

    void sl_bttnRun();
    void sl_translationToggled(bool toggled);

    //void sl_remoteRunButtonClicked();
    void sl_patternChanged();
    
private:
    void clearAll();
    void loadDialogConfig();
    void saveDialogConfig();
    bool readParameters();
    void updateVisualState();

    bool readPattern(DNATranslation* aminoTT);
    bool readRegion();
    bool readRealization();
    bool readSubstMatrix();
    bool readResultFilter();
    bool readGapModel();

    void setParameters();
    void connectGUI();
    void addAnnotationWidget();

    SubstMatrixRegistry* substMatrixRegistry;
    SWResultFilterRegistry* swResultFilterRegistry;
    SmithWatermanTaskFactoryRegistry* swTaskFactoryRegistry;

    SWDialogConfig* dialogConfig;
    SmithWatermanSettings config;
    SmithWatermanTaskFactory* realization;

    ADVSequenceObjectContext* ctxSeq;
    CreateAnnotationWidgetController* ac;

    RegionSelector* rs;
};

} // namespace

#endif

