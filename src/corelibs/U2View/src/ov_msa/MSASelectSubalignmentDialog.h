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

#ifndef _U2_MSA_SELECT_SUBALIGNMENT_DIALOG_H_
#define _U2_MSA_SELECT_SUBALIGNMENT_DIALOG_H_

#include "ui_SelectSubalignmentDialog.h"

#include <U2Core/global.h>
#include <U2Core/MAlignmentObject.h>

namespace U2 {

class MSAEditorUI;

class SelectSubalignmentDialog : public QDialog, Ui_SelectSubalignmentDialog {
    Q_OBJECT
public:
    SelectSubalignmentDialog( MSAEditorUI *ui, const U2Region& region = U2Region(), const QList<qint64>& selectedIndexes = QList<qint64>(), QWidget *p = NULL);

    void accept();

    const U2Region getRegion() const { return window; }

    const QStringList& getSelectedSeqNames() const { return selectedNames; }
    const QList<qint64>& getSelectedSeqIndexes() const { return selectedIndexes; }

public slots:
    void sl_allButtonClicked();
    void sl_noneButtonClicked();
    void sl_invertButtonClicked();

private:
    void init();

    MSAEditorUI *ui;

    U2Region window;
    QStringList selectedNames;
    QList<qint64> selectedIndexes;
};

} // namespace

#endif // _U2_MSA_SELECT_SUBALIGNMENT_DIALOG_H_
