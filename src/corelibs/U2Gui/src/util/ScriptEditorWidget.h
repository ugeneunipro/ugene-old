/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_SCRIPT_EDITOR_WIDGET_H_
#define _U2_SCRIPT_EDITOR_WIDGET_H_

#include <qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QWidget>
#else
#include <QtWidgets/QWidget>
#endif

#include <U2Core/global.h>
#include "ScriptEditorDelegate.h"

class QSplitter;
class QTextEdit;

namespace U2 {

class ScriptEditorWidget : public QWidget {
    Q_OBJECT
public:
    ScriptEditorWidget(QWidget *parent, ScriptEditorType typeOfField = TEXT_EDIT);

    void setVariablesText(const QString &variablesText);
    QString variablesText() const;
    void setScriptText(const QString &text);
    QString scriptText() const;
    int scriptEditCursorLineNumber() const;

signals:
    void si_textChanged();
    void si_cursorPositionChanged();

private:
    QSplitter *scriptContainer;
    QTextEdit *variablesEdit;
    AbstractScriptEditorDelegate *scriptEdit;
};

} // namespace U2

#endif // _U2_SCRIPT_EDITOR_WIDGET_H_
