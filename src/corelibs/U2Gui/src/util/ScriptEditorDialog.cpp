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

#include <QtCore/QFile>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtScript/QScriptEngine>
#include <QtGui/QMouseEvent>

#include <U2Core/L10n.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/DialogUtils.h>
#include "ScriptEditorWidget.h"

#include "ui/ui_ScriptEditorDialog.h"
#include "ScriptEditorDialog.h"

namespace U2 {

ScriptEditorDialog::ScriptEditorDialog(QWidget* w, const QString& roHeaderText, const QString & scriptText) 
: QDialog(w), ui(new Ui_ScriptEditorDialog())
{
    ui->setupUi(this);
    setModal(true);

    scriptEdit = new ScriptEditorWidget(this);
    scriptEdit->setVariablesText(roHeaderText);
    scriptEdit->setScriptText(scriptText);

    ui->scriptEditorLayout->addWidget(scriptEdit);

    connect(ui->checkButton, SIGNAL(clicked()), SLOT(sl_checkSyntax()));
    connect(ui->openButton, SIGNAL(clicked()), SLOT(sl_openScript()));
    connect(ui->saveButton, SIGNAL(clicked()), SLOT(sl_saveScript()));
    connect(ui->saveAsButton, SIGNAL(clicked()), SLOT(sl_saveAsScript()));
    connect(ui->scriptPathEdit, SIGNAL(textChanged(const QString&)), SLOT(sl_nameChanged(const QString&)));
    connect(scriptEdit, SIGNAL(si_textChanged()), SLOT(sl_scriptChanged()));
    connect(scriptEdit, SIGNAL(si_cursorPositionChanged()), SLOT(sl_cursorPositionChanged()));

    updateState();
}

ScriptEditorDialog::~ScriptEditorDialog() {
    delete ui;
}

void ScriptEditorDialog::sl_nameChanged(const QString&) {
    updateState();
}

void ScriptEditorDialog::sl_scriptChanged() {
    updateState();
}


void ScriptEditorDialog::updateState() {
    bool hasScript = !scriptEdit->scriptText().simplified().isEmpty();
    bool hasPath = !ui->scriptPathEdit->text().isEmpty();
    ui->saveButton->setEnabled(hasPath && hasScript);
}

void ScriptEditorDialog::setScriptText(const QString& text) {
    ui->scriptPathEdit->clear();
    scriptEdit->setScriptText(text);
}

void ScriptEditorDialog::setScriptPath(const QString& path) {
    QFile file(path);
    bool ok = file.open(QFile::ReadOnly);
    if (file.size() > 100 * 1000) {
        QMessageBox::critical(this, L10N::errorTitle(), tr("File is too large %1").arg(path));
        return;
    }
    if (!ok) {
        QMessageBox::critical(this, L10N::errorTitle(), L10N::errorOpeningFileRead(path));
        return;
    }
    QByteArray script = file.readAll();
    scriptEdit->setScriptText(script);
    ui->scriptPathEdit->setText(path);
}


#define SCRIPTS_DOMAIN QString("scripts")

void ScriptEditorDialog::sl_openScript() {
    LastUsedDirHelper ld(SCRIPTS_DOMAIN);
    ld.url = QFileDialog::getOpenFileName(this, tr("Select script to open"), ld.dir, getScriptsFileFilter());
    if (ld.url.isEmpty()) {
        return;
    }
    setScriptPath(ld.url);
}

void ScriptEditorDialog::sl_saveScript() {
    QString url = ui->scriptPathEdit->text();
    if (url.isEmpty()) {
        sl_saveAsScript();
    } else {
        save(url);
    }
}

void ScriptEditorDialog::sl_saveAsScript() {
    QString script = scriptEdit->scriptText();
    if (script.simplified().isEmpty()) {
        QMessageBox::critical(this, L10N::errorTitle(), tr("Script is empty!"));
        return;
    }
    LastUsedDirHelper ld(SCRIPTS_DOMAIN);
    ld.url = QFileDialog::getSaveFileName(this, tr("Save script to file"), ld.dir, getScriptsFileFilter());
    if (ld.url.isEmpty()) {
        return;
    }
    save(ld.url);
}

void ScriptEditorDialog::save(const QString& url) {
    QString script = scriptEdit->scriptText();
    QFile file(url);
    bool ok = file.open(QFile::WriteOnly | QFile::Truncate);
    if (!ok) {
        QMessageBox::critical(this, L10N::errorTitle(), L10N::errorWritingFile(url));
        return;
    }
    file.write(script.toLocal8Bit());
    ui->scriptPathEdit->setText(url);
}

QString ScriptEditorDialog::getScriptsFileFilter() {
    return DialogUtils::prepareFileFilter(tr("Script files"), QStringList("js"), true);
}

void ScriptEditorDialog::sl_checkSyntax() {
    QScriptEngine engine;
    QString header = scriptEdit->variablesText();
    QString scriptText = header + "\n" + scriptEdit->scriptText();
    QScriptSyntaxCheckResult syntaxResult = engine.checkSyntax(scriptText);
    if (syntaxResult.state() != QScriptSyntaxCheckResult::Valid) {
        int line = syntaxResult.errorLineNumber();
        line -= header.split("\n").size();
        QString errorMessage = tr("Script syntax check failed! Line: %1, error: %2").arg(line).arg(syntaxResult.errorMessage());
        QMessageBox::warning(this, tr("Check result"), errorMessage);
    } else {
        QMessageBox::information(this, tr("Check result"), tr("Syntax is OK!"));
    }

}

QString ScriptEditorDialog::getScriptText() const
{
    return scriptEdit->scriptText();
}

void ScriptEditorDialog::sl_cursorPositionChanged(){
    int lineNumber = scriptEdit->scriptEditCursorLineNumber();
    ui->lineInfo->setText("Line: " + QString::number(lineNumber + 1));
}

}// namespace
