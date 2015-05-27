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

#include <QPainter>
#include <QColorDialog>

#include <U2Core/U2SafePoints.h>

#include <U2Gui/HelpButton.h>
#include <U2Core/QObjectScopedPointer.h>

#include "ColorSchemaDialogController.h"

namespace U2{

ColorSchemaDialogController::ColorSchemaDialogController(QMap<char, QColor>& colors) :
    QDialog(),
    alphabetColorsView(NULL),
    newColors(colors),
    storedColors(colors)
{

}

int ColorSchemaDialogController::adjustAlphabetColors(){
    setupUi(this);
    new HelpButton(this, buttonBox, "16122228");
    alphabetColorsView = new QPixmap(alphabetColorsFrame->size());
    connect(clearButton, SIGNAL(clicked()), SLOT(sl_onClear()));
    connect(restoreButton, SIGNAL(clicked()), SLOT(sl_onRestore()));

    update();

    return exec();
}

ColorSchemaDialogController::~ColorSchemaDialogController(){
    delete alphabetColorsView;
}

void ColorSchemaDialogController::paintEvent(QPaintEvent*){
    QPainter dialogPainter(this);
    const int columns = 6;

    const int rect_width = static_cast<double> (alphabetColorsFrame->size().width()) / columns ;
    if(rect_width == 0){return;}
    int rect_width_rest = alphabetColorsFrame->size().width() % columns;

    const int rows = (newColors.size() / columns) + ((newColors.size() % columns) ?  1 : 0);
    const int rect_height = static_cast<double> (alphabetColorsFrame->size().height() ) / rows;
    if(rect_height == 0){return;}
    int rect_height_rest = alphabetColorsFrame->size().height() % rows;

    delete alphabetColorsView;
    alphabetColorsView = new QPixmap(alphabetColorsFrame->size());

    QPainter painter(alphabetColorsView);
    QFont font;
    font.setFamily("Verdana");
    font.setPointSize(qMin(rect_width, rect_height) / 2);
    painter.setFont(font);

    QMapIterator<char, QColor> it(newColors);

    int hLineY = 0;
    for(int i = 0; i < rows; ++i){
        int rh = rect_height;
        rect_width_rest = alphabetColorsFrame->size().width() % columns;
        if(rect_height_rest > 0){
            rh++;
            rect_height_rest--;
        }
        painter.drawLine(0, hLineY, alphabetColorsView->size().width(), hLineY);
        int vLineX = 0;
        for(int j = 0; j < columns; ++j){
            if(!it.hasNext()){break;}

            it.next();
            int rw = rect_width;
            if(rect_width_rest > 0){
                rw++;
                rect_width_rest--;
            }
            QRect nextRect(vLineX, hLineY + 1, rw, rh - 1);
            painter.fillRect(nextRect, it.value());
            painter.drawText(nextRect, Qt::AlignCenter, QString(it.key()));
            painter.drawLine(vLineX, hLineY, vLineX, hLineY + rh);
            painter.drawLine(vLineX + rw, hLineY, vLineX + rw, hLineY + rh);
            vLineX += rw;
            charsPlacement[it.key()] = nextRect;
        }
        hLineY += rh;
        if(!it.hasNext()) {
            painter.fillRect(vLineX + 1, hLineY - rh + 1, alphabetColorsView->size().width() - vLineX - 1, rh - 1, dialogPainter.background());
            break;
        }
    }

    painter.drawLine(2, alphabetColorsView->size().height()-2, alphabetColorsView->size().width()-2, alphabetColorsView->size().height()-2);
    painter.drawLine(alphabetColorsView->size().width()-2, 2, alphabetColorsView->size().width()-2, alphabetColorsView->size().height()-2);

    painter.drawLine(alphabetColorsView->size().width()-2, alphabetColorsView->size().height()-2, 2, alphabetColorsView->size().height()-2);
    painter.drawLine(alphabetColorsView->size().width()-2, alphabetColorsView->size().height()-2, alphabetColorsView->size().width()-2, 2);

    dialogPainter.drawPixmap(alphabetColorsFrame->geometry().x(), alphabetColorsFrame->geometry().y(), *alphabetColorsView);
}

void ColorSchemaDialogController::sl_onClear(){
    storedColors = newColors;

    QMapIterator<char, QColor> it(newColors);
    while(it.hasNext()){
        it.next();
        newColors[it.key()] = QColor(Qt::white);
    }

    update();
}

void ColorSchemaDialogController::sl_onRestore(){
    newColors = storedColors;
    update();
}

void ColorSchemaDialogController::mouseReleaseEvent(QMouseEvent * event){
    QMapIterator<char, QRect> it(charsPlacement);

    while(it.hasNext()){
        it.next();
        if(it.value().contains(event->pos().x() - alphabetColorsFrame->geometry().x(), event->pos().y() - alphabetColorsFrame->geometry().y())){
            QObjectScopedPointer<QColorDialog> d = new QColorDialog(this);
#ifdef Q_OS_MAC
            // A workaround because of UGENE-2263
            // Another way should be found.
            // I suppose, that it is bug in the Qt libraries (Qt-4.8.5 for mac)
            d->setOption(QColorDialog::DontUseNativeDialog);
#endif
            const int res = d->exec();
            CHECK(!d.isNull(), );

            if(res == QDialog::Accepted){
                newColors[it.key()] = d->selectedColor();
            }
            break;
        }
    }

    update();
}

} // namespace
