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

#ifndef _U2_RANGE_SELECTOR_H_
#define _U2_RANGE_SELECTOR_H_ 

#include <U2Core/global.h>

#include <QtGui/QLineEdit>
#include <QtGui/QDialog>

namespace U2 {

class U2GUI_EXPORT RangeSelector : public QWidget {
    Q_OBJECT
public:
    RangeSelector(QWidget* p, int rangeStart, int rangeEnd);
    RangeSelector(QDialog* d, int rangeStart, int rangeEnd, int len, bool autoclose);

    ~RangeSelector();
    int getStart() const;
    int getEnd() const;

signals:
    void si_rangeChanged(int startPos, int endPos);

private slots:
    void sl_onGoButtonClicked(bool);
    void sl_onMinButtonClicked(bool);
    void sl_onMaxButtonClicked(bool);
    void sl_onReturnPressed();

private:
    void init();
    void exec();

    int         rangeStart;
    int         rangeEnd;
    int         len;
    QLineEdit*  startEdit;
    QLineEdit*  endEdit;
    QDialog*    dialog;
    bool        autoclose;
};

}//namespace

#endif
