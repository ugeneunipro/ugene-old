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

#ifndef _U2_EXPORT_COVERAGE_DIALOG_FILLER_H_
#define _U2_EXPORT_COVERAGE_DIALOG_FILLER_H_

#include "GTUtilsDialog.h"

namespace U2 {

class ExportCoverageDialogFiller : public Filler {
public:
    enum ActionType {                   // An appropriate action data
        EnterFilePath,                  // QString with a path (do not forget about native separators)
        SelectFile,                     // QString with a path
        SetCompress,                    // bool with a new checkbox state
        SetExportCoverage,              // bool with a new checkbox state
        SetExportBasesCount,            // bool with a new checkbox state
        SetThreshold,                   // int with a new threshold value
        CheckFilePath,                  // QString with an expected path (do not forget about native separators)
        CheckCompress,                  // bool with an expected checkbox state
        CheckExportCoverage,            // bool with an expected checkbox state
        CheckExportBasesCount,          // bool with an expected checkbox state
        CheckThreshold,                 // int with an expected spinbox value
        CheckThresholdBounds,           // QVariantMap that contains two items: key: "minimum", value: int; key: "maximum", value: int
        ExpectMessageBox,               // ignored
        ClickOk,                        // ignored
        ClickCancel                     // ignored
    };

    typedef QPair<ActionType, QVariant> Action;

    ExportCoverageDialogFiller(U2OpStatus &os, const QList<Action> &actions);

    void run();

    const static QString SPINBOX_MINIMUM;
    const static QString SPINBOX_MAXIMUM;

private:
    void enterFilePath(const QVariant &actionData);
    void selectFile(const QVariant &actionData);
    void setCompress(const QVariant &actionData);
    void setExportCoverage(const QVariant &actionData);
    void setExportBasesCount(const QVariant &actionData);
    void setThreshold(const QVariant &actionData);
    void checkFilePath(const QVariant &actionData);
    void checkCompress(const QVariant &actionData);
    void checkExportCoverage(const QVariant &actionData);
    void checkExportBasesCount(const QVariant &actionData);
    void checkThreshold(const QVariant &actionData);
    void checkThresholdBounds(const QVariant &actionData);
    void expectMessageBox();
    void clickOk();
    void clickCancel();

    QWidget *dialog;
    const QList<Action> actions;
};

}   // namespace U2

#endif // _U2_EXPORT_COVERAGE_DIALOG_FILLER_H_
