/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _U2_MSA_EXPORT_CONSENSUS_TAB_H_
#define _U2_MSA_EXPORT_CONSENSUS_TAB_H_

#include <U2Gui/U2SavableWidget.h>

#include "ui/ui_ExportConsensusWidget.h"

namespace U2 {

class MSAEditor;

class MSAExportConsensusTab : public QWidget, private Ui_ExportConsensusWidget {
    Q_OBJECT
public:
    MSAExportConsensusTab(MSAEditor* msa_);

    void showHint(bool showHint);

private slots:
    void sl_browseClicked();
    void sl_exportClicked();
    void sl_formatChanged();
    void sl_consensusChanged(const QString& algoId);
private:
    void checkEmptyFilepath() const;
    MSAEditor *msa;
    U2SavableWidget savableWidget;
};

} // namespace

#endif
