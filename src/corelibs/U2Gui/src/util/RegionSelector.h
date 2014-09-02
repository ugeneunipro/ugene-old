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

#ifndef _U2_REGION_SELECTOR_H_
#define _U2_REGION_SELECTOR_H_

#include <U2Core/DNASequenceSelection.h>
#include <U2Core/U2Region.h>
#include <U2Core/global.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialog>
#include <QtGui/QLineEdit>
#else
#include <QtWidgets/QDialog>
#include <QtWidgets/QLineEdit>
#endif

class QComboBox;

namespace U2 {

class AnnotatedDNAView;
class RegionLineEdit;

struct RegionPreset {
    RegionPreset() {}
    RegionPreset(const QString &text, const U2Region &region) : text(text), region(region) {}
    QString text;
    U2Region region;
};

class U2GUI_EXPORT RegionSelector : public QWidget {
    Q_OBJECT
public:
    RegionSelector(QWidget* p, qint64 len, bool isVertical = false, DNASequenceSelection* selection = NULL, QList<RegionPreset> presetRegions = QList<RegionPreset>());

    U2Region getRegion(bool *ok = NULL) const;

    void setMaxLength(qint64 length);
    void setCustomRegion(const U2Region& value);
    void setSequenceSelection(DNASequenceSelection* selection);
    void updateSelectedRegion(const U2Region &selectedRegion);
    void setWholeRegionSelected();
    void reset();

    void showErrorMessage();

signals:
    void si_regionChanged(const U2Region& newRegion);

private slots:
    void sl_onComboBoxIndexChanged(int index);
    void sl_onRegionChanged();
    void sl_onValueEdited();
    void sl_onSelectionChanged(GSelection* selection);

private:
    void initLayout();
    void init(const QList<RegionPreset> &presetRegions);
    void connectSignals();

    qint64                maxLen;
    RegionLineEdit *      startEdit;
    RegionLineEdit *      endEdit;
    QComboBox *           comboBox;
    bool                  isVertical;
    QString               defaultItemText;
    DNASequenceSelection *selection;

    static const QString WHOLE_SEQUENCE;
    static const QString SELECTED_REGION;
    static const QString CUSTOM_REGION;
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
