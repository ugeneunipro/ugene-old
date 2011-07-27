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

#include "ScriptEditorDialog.h"
#include "ui/ui_ScriptEditorDialog.h"

#include <U2Core/L10n.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/DialogUtils.h>

#include <QtCore/QFile>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtScript/QScriptEngine>
#include <QtGui/QMouseEvent>

namespace U2 {

ScriptEditorDialog::ScriptEditorDialog(QWidget* w, const QString& roHeaderText, const QString & scriptText) 
:QDialog(w)
{
    ui = new Ui_ScriptEditorDialog();
    ui->setupUi(this);

    setModal(true);
    ui->headerEdit->setText(roHeaderText);
    ui->headerEdit->setReadOnly(true);
    
    ScriptHighlighter* h1 = new ScriptHighlighter(ui->headerEdit->document());
    h1->setParent(this);
    ScriptHighlighter* h2 = new ScriptHighlighter(ui->scriptEdit->document());
    h2->setParent(this);
    ui->scriptEdit->setText(scriptText);
    
    connect(ui->checkButton, SIGNAL(clicked()), SLOT(sl_checkSyntax()));
    connect(ui->openButton, SIGNAL(clicked()), SLOT(sl_openScript()));
    connect(ui->saveButton, SIGNAL(clicked()), SLOT(sl_saveScript()));
    connect(ui->saveAsButton, SIGNAL(clicked()), SLOT(sl_saveAsScript()));
    connect(ui->scriptPathEdit, SIGNAL(textChanged(const QString&)), SLOT(sl_nameChanged(const QString&)));
    connect(ui->scriptEdit, SIGNAL(textChanged()), SLOT(sl_scriptChanged()));
    connect(ui->scriptEdit, SIGNAL(cursorPositionChanged()), SLOT(sl_cursorPositionChanged()));

    updateState();
}

void ScriptEditorDialog::sl_nameChanged(const QString&) {
    updateState();
}

void ScriptEditorDialog::sl_scriptChanged() {
    updateState();
}


void ScriptEditorDialog::updateState() {
    bool hasScript = !ui->scriptEdit->toPlainText().simplified().isEmpty();
    bool hasPath = !ui->scriptPathEdit->text().isEmpty();
    ui->saveButton->setEnabled(hasPath && hasScript);
}

void ScriptEditorDialog::setScriptText(const QString& text) {
    ui->scriptPathEdit->clear();
    ui->scriptEdit->setText(text);
}

void ScriptEditorDialog::setScriptPath(const QString& path) {
    QFile file(path);
    bool ok = file.open(QFile::ReadOnly);
    if (file.size() > 100*1000) {
        QMessageBox::critical(this, L10N::errorTitle(), tr("File is too large %1").arg(path));
        return;
    }
    if (!ok) {
        QMessageBox::critical(this, L10N::errorTitle(), L10N::errorOpeningFileRead(path));
        return;
    }
    QByteArray script = file.readAll();
    ui->scriptEdit->setText(script);
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
    QString script = ui->scriptEdit->toPlainText();
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
    QString script = ui->scriptEdit->toPlainText();
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
    QString header = ui->headerEdit->toPlainText();
    QString scriptText = header + "\n" + ui->scriptEdit->toPlainText();
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

ScriptEditorDialog::~ScriptEditorDialog()
{
    delete ui;
}

QString ScriptEditorDialog::getScriptText() const
{
    return ui->scriptEdit->toPlainText();
}

void ScriptEditorDialog::sl_cursorPositionChanged(){
    int lineNumber = ui->scriptEdit->textCursor().block().blockNumber();
    ui->lineInfo->setText("Line: " + QString::number(lineNumber + 1));
}
//////////////////////////////////////////////////////////////////////////
// Script highlighter
ScriptHighlighter::ScriptHighlighter(QTextDocument *parent)
: QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bvar\\b" << "\\bArray\\b" << "\\bfunction\\b"
                    << "\\breturn\\b" << "\\barguments\\b" << "\\bif\\b"
                    << "\\belse\\b" << "\\bfor\\b" << "\\bswitch\\b"
                    << "\\bcase\\b" << "\\bbreak\\b" << "\\bwhile\\b";


    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }



    // Values
    QTextCharFormat valueFormat;
    valueFormat.setForeground(Qt::blue);
    rule.format = valueFormat;
    rule.pattern = QRegExp("\\btrue\\b|\\bfalse\\b|\\b[0-9]+\\b");
    highlightingRules.append(rule);
    
    QTextCharFormat functionFormat;
    functionFormat.setForeground(Qt::darkBlue);
    rule.format = functionFormat;
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    highlightingRules.append(rule);
    
    // Quotation

    QTextCharFormat quotationFormat;
    quotationFormat.setForeground(Qt::blue);
    rule.format = quotationFormat;
    rule.pattern = QRegExp("\"[^\"]*\"");
    highlightingRules.append(rule);
    
    // Single Line Comments
    QTextCharFormat singleLineCommentFormat;
    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.format = singleLineCommentFormat;
    rule.pattern = QRegExp("//[^\n]*");
    highlightingRules.append(rule);
    
    multiLineCommentFormat.setForeground(Qt::darkGreen);

    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");
}

void ScriptHighlighter::highlightBlock(const QString &text) {
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1) {
        startIndex = commentStartExpression.indexIn(text);
    }

    while (startIndex >= 0) {
        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }
}

}// namespace
