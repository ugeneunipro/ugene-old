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

#ifndef CREATEELEMENTWITHSCRIPTDIALOGFILLER_H
#define CREATEELEMENTWITHSCRIPTDIALOGFILLER_H

#include "GTUtilsDialog.h"

namespace U2{

class CreateElementWithScriptDialogFiller : public Filler
{
public:
    CreateElementWithScriptDialogFiller(U2OpStatus& os, QString _name):Filler(os, "CreateScriptBlockDialog"),
        name(_name){}
    virtual void run();
private:
    QString name;
};

class ScriptEditorDialogFiller: public Filler
{
public:
    ScriptEditorDialogFiller(U2OpStatus& os, QString _url = "", QString _text = ""): Filler(os, "ScriptEditorDialog"),
        url(_url),
        text(_text){}
    virtual void run();
private:
    QString url,text;
};

class ScriptEditorDialogSyntaxChecker: public Filler
{
public:
    ScriptEditorDialogSyntaxChecker(U2OpStatus& os, QString _text = "", QString _message = ""): Filler(os, "ScriptEditorDialog"),
        text(_text),
        message(_message){}
    virtual void run();
private:
    QString text, message;
};
}

#endif // CREATEELEMENTWITHSCRIPTDIALOGFILLER_H
