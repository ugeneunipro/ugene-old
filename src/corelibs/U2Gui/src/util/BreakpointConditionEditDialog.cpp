/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "ScriptEditorWidget.h"

#include "ui/ui_BreakpointConditionEditDialog.h"
#include "BreakpointConditionEditDialog.h"

namespace U2 {

BreakpointConditionEditDialog::BreakpointConditionEditDialog(QWidget *parent,
    const QString &variablesText, bool conditionEnabled, const QString &conditionText,
    HitCondition initCondition)
    : QDialog(parent), initHitCondition(initCondition)
{
    ui = new Ui_BreakpointConditionEditDialog();
    ui->setupUi(this);

    scriptEdit = new ScriptEditorWidget(this);
    scriptEdit->setVariablesText(variablesText);
    scriptEdit->setScriptText(conditionText);

    ui->editorWidgetLayout->addWidget(scriptEdit);

    switch (initHitCondition) {
    case CONDITION_IS_TRUE:
        ui->isTrueButton->setChecked(true);
        break;
    case CONDITION_HAS_CHANGED:
        ui->hasChangedButton->setChecked(true);
        break;
    default:
        Q_ASSERT(false);
    }

    ui->conditionBox->setChecked(conditionEnabled);

    connect(ui->buttonBox, SIGNAL(accepted()), SLOT(sl_dialogAccepted()));
}

BreakpointConditionEditDialog::~BreakpointConditionEditDialog() {
    delete ui;
}

void BreakpointConditionEditDialog::sl_dialogAccepted() {
    emit si_conditionSwitched(ui->conditionBox->isChecked());

    HitCondition hitCondition;
    if (ui->isTrueButton->isChecked()) {
        hitCondition = CONDITION_IS_TRUE;
    } else if (ui->hasChangedButton->isChecked()) {
        hitCondition = CONDITION_HAS_CHANGED;
    }
    if (initHitCondition != hitCondition) {
        emit si_conditionParameterChanged(hitCondition);
    }
    emit si_conditionTextChanged(scriptEdit->scriptText());
}

} // namespace U2
