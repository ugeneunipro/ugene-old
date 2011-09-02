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

#ifndef _U2_REGION_SELECTOR_H_
#define _U2_REGION_SELECTOR_H_

#include <U2Core/global.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/U2Region.h>

#include <QtGui/QLineEdit>
#include <QtGui/QDialog>
#include <QtGui/QComboBox>

namespace U2 {

class RegionLineEdit;
class U2GUI_EXPORT RegionSelector : public QWidget {
    Q_OBJECT
public:
    RegionSelector(QWidget* p, qint64 len, bool isVertical = false, DNASequenceSelection* selection = NULL);

    ~RegionSelector();
    U2Region getRegion(bool *ok = NULL) const;

    void setRegion(U2Region value);
    void reset();

    void showErrorMessage();

signals:
    void si_rangeChanged(int startPos, int endPos);

private slots:
    void sl_onComboBoxIndexChanged(int);
    void sl_onRangeChanged();
    void sl_onValueEdited();

private:
    void init();

    qint64          len;
    RegionLineEdit* startEdit;
    RegionLineEdit* endEdit;
    QComboBox*      comboBox;
    U2Region        region;
    bool            isVertical;
    bool            needAddSelectionButton;

    DNASequenceSelection* selection;
};

class RegionLineEdit : public QLineEdit {
    Q_OBJECT
public:
    RegionLineEdit(QWidget* p, QString actionName, qint64 defVal) : QLineEdit(p), actionName(actionName), defaultValue(defVal){}
protected:
    void focusOutEvent ( QFocusEvent * event );
    void contextMenuEvent(QContextMenuEvent *);

private slots:
    void sl_onSetMinMaxValue();
private:
    const QString actionName;
    qint64 defaultValue;
};
}//namespace

#endif
