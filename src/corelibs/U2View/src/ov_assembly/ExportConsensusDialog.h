/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#ifndef _U2_EXPORT_CONSENSUS_DIALOG_H__
#define _U2_EXPORT_CONSENSUS_DIALOG_H__

#include "ExportConsensusTask.h"
#include <ui_ExportConsensusDialog.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif

namespace U2 {

class SaveDocumentController;
class RegionSelector;

class ExportConsensusDialog : public QDialog, Ui_ExportConsensusDialog {
    Q_OBJECT
public:
    ExportConsensusDialog(QWidget * p, const ExportConsensusTaskSettings & settings, const U2Region & visibleRegion);

    virtual void accept();

    const ExportConsensusTaskSettings & getSettings() const { return settings; }

private:
    void initSaveController();

    ExportConsensusTaskSettings settings;
    SaveDocumentController*     saveController;
    RegionSelector*             regionSelector;
};

} // namespace

#endif // _U2_EXPORT_CONSENSUS_DIALOG_H__
