/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <math.h>

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPalette>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

#include <U2Core/L10n.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/GUIUtils.h>
#include <U2Core/QObjectScopedPointer.h>

#include "RegionSelector.h"

namespace U2 {
////////////////////////////////////////
// RangeSelectorWidget
const QString RegionSelector::WHOLE_SEQUENCE = QApplication::translate("RegionSelector", "Whole sequence");
const QString RegionSelector::SELECTED_REGION = QApplication::translate("RegionSelector", "Selected region");
const QString RegionSelector::CUSTOM_REGION = QApplication::translate("RegionSelector", "Custom region");

RegionSelector::RegionSelector(QWidget* p, qint64 len, bool isVertical,
                               DNASequenceSelection* selection,
                               bool isCircularSelectionAvailable,
                               QList<RegionPreset> presetRegions) :
    QWidget(p),
    maxLen(len),
    startEdit(NULL),
    endEdit(NULL),
    isVertical(isVertical),
    selection(selection),
    isCircularSelectionAvailable(isCircularSelectionAvailable)
{
    defaultItemText = WHOLE_SEQUENCE;

    if (selection != NULL && !selection->isEmpty()) {
        U2Region region = getOneRegionFromSelection();
        presetRegions.prepend(RegionPreset(SELECTED_REGION, region));
        defaultItemText = SELECTED_REGION;
    }

    presetRegions.prepend(RegionPreset(WHOLE_SEQUENCE, U2Region(0, maxLen)));

    initLayout();
    init(presetRegions);
    connectSignals();
    reset();
}

U2Region RegionSelector::getRegion(bool *_ok) const {
    bool ok = false;
    qint64 v1 = startEdit->text().toLongLong(&ok) - 1;

    if (!ok || v1 < 0 || v1 > maxLen) {
        if (_ok != NULL) {
            *_ok = false;
        }
        return U2Region();
    }

    int v2 = endEdit->text().toLongLong(&ok);

    if (!ok || v2 <= 0 || v2 > maxLen) {
        if (_ok != NULL) {
            *_ok = false;
        }
        return U2Region();
    }

    if (v1 > v2 && !isCircularSelectionAvailable) { // start > end
        if (_ok != NULL) {
            *_ok = false;
        }
        return U2Region();
    }

    if (_ok != NULL) {
        *_ok = true;
    }

    if (v1 < v2) {
        return U2Region(v1, v2 - v1);
    } else {
        return U2Region(v1, v2 + maxLen - v1);
    }
}

bool RegionSelector::isWholeSequenceSelected() const {
    return comboBox->currentText() == WHOLE_SEQUENCE;
}

void RegionSelector::setMaxLength(qint64 length) {
    maxLen = length;
    const int wholeSequenceIndex = comboBox->findText(WHOLE_SEQUENCE);
    comboBox->setItemData(wholeSequenceIndex, qVariantFromValue(U2Region(0, length)));
    if (wholeSequenceIndex == comboBox->currentIndex()) {
        sl_onComboBoxIndexChanged(wholeSequenceIndex);
    }
}

void RegionSelector::setCustomRegion(const U2Region& value) {
    if (value.startPos < 0 || value.length > maxLen) {
        return;
    }

    if (value == getRegion()) {
        return;
    }

    startEdit->setText(QString::number(value.startPos + 1));
    endEdit->setText(QString::number(value.endPos()));

    if (U2Region(0, maxLen) != value) {
        comboBox->setCurrentIndex(comboBox->findText(CUSTOM_REGION));
    }

    emit si_regionChanged(value);
}

void RegionSelector::setSequenceSelection(DNASequenceSelection *_selection) {
    disconnect(this, SLOT(sl_onSelectionChanged(GSelection*)));
    selection = _selection;
    if (NULL != selection) {
        connect(selection, SIGNAL(si_onSelectionChanged(GSelection*)), SLOT(sl_onSelectionChanged(GSelection*)));
        sl_onSelectionChanged(selection);
    }
}

void RegionSelector::setWholeRegionSelected() {
    comboBox->setCurrentIndex(comboBox->findText(WHOLE_SEQUENCE));
}

void RegionSelector::setCircularSelectionAvailable(bool allowCircSelection) {
    isCircularSelectionAvailable = allowCircSelection;
}

void RegionSelector::reset() {
    comboBox->setCurrentIndex(comboBox->findText(defaultItemText));
}

void RegionSelector::showErrorMessage() {
    QObjectScopedPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::NoIcon, L10N::errorTitle(), tr("Invalid sequence region!"), QMessageBox::Ok);

    //set focus to error field
    bool ok = false;
    qint64 v1 = startEdit->text().toLongLong(&ok) - 1;
    if (!ok || v1 <= 0 || v1 > maxLen) {
        msgBox->setInformativeText(tr("Invalid Start position of region"));
        msgBox->exec();
        CHECK(!msgBox.isNull(), );
        startEdit->setFocus();
        return;
    }

    int v2 = endEdit->text().toLongLong(&ok);
    if (!ok || v2 <= 0 || v2 > maxLen) {
        msgBox->setInformativeText(tr("Invalid End position of region"));
        msgBox->exec();
        CHECK(!msgBox.isNull(), );
        endEdit->setFocus();
        return;
    }

    if (v1 > v2 && !isCircularSelectionAvailable) { // start > end
        msgBox->setInformativeText(tr("Start position is greater than End position"));
        msgBox->exec();
        CHECK(!msgBox.isNull(), );
        startEdit->setFocus();
        return;
    }

    msgBox->exec();
}

void RegionSelector::sl_onComboBoxIndexChanged(int index) {
    if (index == comboBox->findText(CUSTOM_REGION)) {
        return;
    }

    const U2Region region = comboBox->itemData(index).value<U2Region>();
    qint64 start = region.startPos + 1;
    qint64 end = region.endPos();
    startEdit->setText(QString::number(start));
    endEdit->setText(QString::number(end > maxLen ? end - maxLen : end));
    sl_onValueEdited();
    sl_onRegionChanged();
}

void RegionSelector::sl_onRegionChanged() {
    bool ok = false;

    int v1 = startEdit->text().toInt(&ok);
    if (!ok || v1 < 1 || v1 > maxLen) {
        return;
    }

    int v2 = endEdit->text().toInt(&ok);
    if (!ok || v2 < 1 || v2 > maxLen) {
        return;
    }
    if (!isCircularSelectionAvailable && v2 < v1) {
        return;
    }

    U2Region r;
    if (v1 <= v2) {
        r = U2Region(v1 - 1, v2 - (v1 - 1));
    } else {
        r = U2Region(v1 - 1, v2 + maxLen - (v1 - 1));
    }
    emit si_regionChanged(r);
}

void RegionSelector::sl_onValueEdited() {
    if (startEdit->text().isEmpty() || endEdit->text().isEmpty()) {
        GUIUtils::setWidgetWarning(startEdit, startEdit->text().isEmpty());
        GUIUtils::setWidgetWarning(endEdit, endEdit->text().isEmpty());
        // select "custom" in combobox
        comboBox->setCurrentIndex(comboBox->findText(CUSTOM_REGION));
        return;
    }

    const U2Region region = getRegion();
    if (region != comboBox->itemData(comboBox->currentIndex()).value<U2Region>()) {
        int currentIndex = comboBox->findText(CUSTOM_REGION);
        for (int i = 0; i < comboBox->count(); i++) {
            if (region == comboBox->itemData(i).value<U2Region>()) {
                currentIndex = i;
                break;
            }
        }
        comboBox->setCurrentIndex(currentIndex);
    }

    GUIUtils::setWidgetWarning(startEdit, region.isEmpty());
    GUIUtils::setWidgetWarning(endEdit, region.isEmpty());
}

void RegionSelector::sl_onSelectionChanged(GSelection *_selection) {
    SAFE_POINT(selection == _selection, "Invalid sequence selection", );
    int selectedRegionIndex = comboBox->findText(SELECTED_REGION);
    if (-1 == selectedRegionIndex) {
        selectedRegionIndex = comboBox->findText(WHOLE_SEQUENCE) + 1;
        comboBox->insertItem(selectedRegionIndex, SELECTED_REGION);
    }

    U2Region region = getOneRegionFromSelection();
    if (region != comboBox->itemData(selectedRegionIndex).value<U2Region>()) {
        comboBox->setItemData(selectedRegionIndex, qVariantFromValue(region));
        if (selectedRegionIndex == comboBox->currentIndex()) {
            sl_onComboBoxIndexChanged(selectedRegionIndex);
        }
    }
}

void RegionSelector::initLayout() {
    int w = qMax(((int)log10((double)maxLen))*10, 50);

    comboBox = new QComboBox(this);

    startEdit = new RegionLineEdit(this, tr("Set minimum"), 1);
    startEdit->setValidator(new QIntValidator(1, maxLen, startEdit));
    startEdit->setMinimumWidth(w);
    startEdit->setAlignment(Qt::AlignRight);

    endEdit = new RegionLineEdit(this, tr("Set maximum"), maxLen);
    endEdit->setValidator(new QIntValidator(1, maxLen, startEdit));
    endEdit->setMinimumWidth(w);
    endEdit->setAlignment(Qt::AlignRight);

    if (isVertical) {
        QGroupBox* gb = new QGroupBox(this);
        gb->setTitle(tr("Region"));

        QGridLayout* l = new QGridLayout(gb);
        l->setSizeConstraint(QLayout::SetMinAndMaxSize);
        gb->setLayout(l);

        l->addWidget(comboBox, 0, 0, 1, 3);
        l->addWidget(startEdit, 1, 0);
        l->addWidget(new QLabel(tr("-"), gb), 1, 1);
        l->addWidget(endEdit, 1, 2);

        QVBoxLayout* rootLayout = new QVBoxLayout(this);
        rootLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        rootLayout->setMargin(0);
        setLayout(rootLayout);
        rootLayout->addWidget(gb);
    } else {
        QHBoxLayout* l = new QHBoxLayout(this);
        l->setMargin(0);
        setLayout(l);

        QLabel* rangeLabel = new QLabel(tr("Region"), this);
        rangeLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

        l->addWidget(rangeLabel);
        l->addWidget(comboBox);
        l->addWidget(startEdit);
        l->addWidget(new QLabel(tr("-"), this));
        l->addWidget(endEdit);
    }

    startEdit->setObjectName("start_edit_line");
    endEdit->setObjectName("end_edit_line");
    comboBox->setObjectName("region_type_combo");
    setObjectName("range_selector");
}

void RegionSelector::init(const QList<RegionPreset> &presetRegions) {
    foreach(const RegionPreset &presetRegion, presetRegions) {
        comboBox->addItem(presetRegion.text, qVariantFromValue(presetRegion.region));
    }
    comboBox->addItem(CUSTOM_REGION);

    const U2Region region = comboBox->itemData(comboBox->findText(defaultItemText)).value<U2Region>();
    startEdit->setText(QString::number(region.startPos + 1));

    endEdit->setText(QString::number(region.endPos() > maxLen ? region.endPos() - maxLen : region.endPos()));
}

void RegionSelector::connectSignals() {
    connect(comboBox,  SIGNAL(currentIndexChanged(int)),           SLOT(sl_onComboBoxIndexChanged(int)));
    connect(startEdit, SIGNAL(editingFinished()),                  SLOT(sl_onRegionChanged()));
    connect(startEdit, SIGNAL(textEdited(const QString &)),        SLOT(sl_onValueEdited()));
    connect(startEdit, SIGNAL(textChanged(QString)),               SLOT(sl_onRegionChanged()));

    connect(endEdit,   SIGNAL(editingFinished()),                  SLOT(sl_onRegionChanged()));
    connect(endEdit,   SIGNAL(textEdited(const QString &)),        SLOT(sl_onValueEdited()));
    connect(endEdit,   SIGNAL(textChanged(QString)),               SLOT(sl_onRegionChanged()));

    if (NULL != selection) {
        connect(selection, SIGNAL(si_onSelectionChanged(GSelection*)), SLOT(sl_onSelectionChanged(GSelection*)));
    }
}

U2Region RegionSelector::getOneRegionFromSelection() const {
    U2Region region = selection->getSelectedRegions().isEmpty()
            ? U2Region(0, maxLen)
            : selection->getSelectedRegions().first();
    if (selection->getSelectedRegions().size() == 2) {
        U2Region secondReg = selection->getSelectedRegions().last();
        bool circularSelection = (region.startPos == 0 && secondReg.endPos() == maxLen)
                || (region.endPos() == maxLen && secondReg.startPos == 0);
        if (circularSelection) {
            if (secondReg.startPos == 0) {
                region.length += secondReg.length;
            } else {
                region.startPos = secondReg.startPos;
                region.length += secondReg.length;
            }
        }
    }

    return region;
}

///////////////////////////////////////
//RegionLineEdit
//only for empty field highlight
void RegionLineEdit::focusOutEvent ( QFocusEvent * event) {
    bool ok = false;
    int value = text().toInt(&ok);
    Q_UNUSED(value);
    if (!ok) {
        QPalette p = palette();
        p.setColor(QPalette::Base, QColor(255,200,200));//pink color
        setPalette(p);
    }
    QLineEdit::focusOutEvent(event);
}
void RegionLineEdit::contextMenuEvent(QContextMenuEvent *event){
        QMenu *menu = createStandardContextMenu();
        QAction* setDefaultValue=new QAction(actionName,this);
        connect(setDefaultValue,SIGNAL(triggered()),this,SLOT(sl_onSetMinMaxValue()));
        menu->insertSeparator(menu->actions().first());
        menu->insertAction(menu->actions().first(),setDefaultValue);
        menu->exec(event->globalPos());
        delete menu;
}
void RegionLineEdit::sl_onSetMinMaxValue(){
    setText(QString::number(defaultValue));
    emit textEdited(QString::number(defaultValue));
}
} //namespace
