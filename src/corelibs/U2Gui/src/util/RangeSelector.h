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

#ifndef _U2_RANGE_SELECTOR_H_
#define _U2_RANGE_SELECTOR_H_ 



#include <U2Core/global.h>
#include <U2Core/U2Region.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QToolButton>
#include <QtGui/QDialog>
#else
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QDialog>
#endif

class Ui_RangeSelectionDialog;

namespace U2 {

class U2GUI_EXPORT RangeSelector : public QWidget {
    Q_OBJECT
public:
    RangeSelector(QWidget* p, int rangeStart, int rangeEnd);
    RangeSelector(QDialog* d, int rangeStart, int rangeEnd, int len, bool autoclose); 
    //RangeSelector(QDialog* d, const QVector<U2Region>& regions, int seqLen, bool autoclose); 

    ~RangeSelector();
    int getStart() const;
    int getEnd() const;

    //QVector<U2Region> getSelection() const;

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

    int                 rangeStart;
    int                 rangeEnd;
    int                 len;
    //QVector<U2Region>   selectedRanges;

    QLineEdit*          startEdit;
    QLineEdit*          endEdit;
    QToolButton*        minButton;
    QToolButton*        maxButton;
    QLabel*             rangeLabel;

//     QLabel*             multiRangeLabel;
//     QLineEdit*          multiRangeEdit;
//     QRadioButton*       singleRangeButton;
//     QRadioButton*       multipleRangeButton;
//     QButtonGroup*       buttonGroup;

    QDialog*            dialog;

    bool                autoclose;
    //RangeSelectorMode   rangeSelectorMode;
};



class U2GUI_EXPORT MultipleRangeSelector :public QDialog{
    Q_OBJECT
public:
    MultipleRangeSelector(QWidget* parent, const QVector<U2Region>& _regions, int _seqLen);
    ~MultipleRangeSelector();

    virtual void accept();

    QVector<U2Region> getSelectedRegions();

private:
    int                 seqLen;
    QVector<U2Region>   selectedRanges;

    Ui_RangeSelectionDialog* ui;

protected slots:
    void sl_buttonClicked(QAbstractButton* b);
    void sl_minButton();
    void sl_maxButton();
    void sl_returnPressed();
};

}//namespace

#endif
