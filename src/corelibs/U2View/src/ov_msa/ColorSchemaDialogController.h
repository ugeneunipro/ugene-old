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

#ifndef _U2_COLOR_SCHEMA_DIALOG_CONTROLLER_H_
#define _U2_COLOR_SCHEMA_DIALOG_CONTROLLER_H_

#include "ui/ui_ColorSchemaDialog.h"

#include <QtCore/QMap>

#include <QtGui/QPixmap>
#include <QtGui/QMouseEvent>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif

namespace U2{

class ColorSchemaDialogController : public QDialog, public Ui_ColorSchemaDialog{
    Q_OBJECT
public:
    ColorSchemaDialogController(QMap<char, QColor>& colors);
    ~ColorSchemaDialogController();
    int adjustAlphabetColors();
protected:
    void mouseReleaseEvent(QMouseEvent *);
    void paintEvent(QPaintEvent*);
private slots:
    void sl_onClear();
    void sl_onRestore();
private:
    QPixmap* alphabetColorsView;
    QMap<char, QColor>& newColors;
    QMap<char, QColor> storedColors;
    QMap<char, QRect> charsPlacement;

};

}

#endif // _U2_COLOR_SCHEMA_DIALOG_CONTROLLER_H_
