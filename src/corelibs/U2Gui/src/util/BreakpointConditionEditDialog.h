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

#ifndef _U2_BREAKPOINT_CONDITION_EDIT_DIALOG_H_
#define _U2_BREAKPOINT_CONDITION_EDIT_DIALOG_H_

#include <qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif

#include <U2Core/global.h>

class Ui_BreakpointConditionEditDialog;

namespace U2 {

class ScriptEditorWidget;

enum HitCondition {
    CONDITION_IS_TRUE, CONDITION_HAS_CHANGED
};

class U2GUI_EXPORT BreakpointConditionEditDialog : public QDialog {
    Q_OBJECT
public:
    BreakpointConditionEditDialog(QWidget *parent, const QString &variablesText, bool conditionEnabled = true,
        const QString &conditionText = QString(), HitCondition initCondition = CONDITION_IS_TRUE);
    ~BreakpointConditionEditDialog();

signals:
    void si_conditionTextChanged(const QString &text);
    void si_conditionParameterChanged(HitCondition newCondition);
    void si_conditionSwitched(bool enabled);

private slots:
    void sl_dialogAccepted();

private:
    const HitCondition initHitCondition;

    ScriptEditorWidget *scriptEdit;
    Ui_BreakpointConditionEditDialog *ui;
};

} //namespace U2

#endif // _U2_BREAKPOINT_CONDITION_EDIT_DIALOG_H_
