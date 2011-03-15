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

#include "DotPlotDialog.h"

#include <U2Gui/CreateAnnotationWidgetController.h>
#include <U2View/ADVSequenceObjectContext.h>

#include <QtGui/QColorDialog>

namespace U2 {

DotPlotDialog::DotPlotDialog(QWidget *parent, const QList<ADVSequenceObjectContext *> &seq, int minLen, int identity, ADVSequenceObjectContext *sequenceX, ADVSequenceObjectContext *sequenceY, bool dir, bool inv, const QColor &dColor, const QColor &iColor)
: QDialog(parent), sequences(seq), directColor(dColor), invertedColor(iColor)
{
    setupUi(this);

    if (sequences.size() <= 0) {
        return;
    }

    directCheckBox->setChecked(dir);
    invertedCheckBox->setChecked(inv);

    updateColors();

    // set algorithms
    Q_ASSERT(algoCombo);
    algoCombo->addItem(tr("Auto"), RFAlgorithm_Auto);
    algoCombo->addItem(tr("Suffix index"), RFAlgorithm_Suffix);
    algoCombo->addItem(tr("Diagonals"), RFAlgorithm_Diagonal);

    Q_ASSERT(xAxisCombo);
    Q_ASSERT(yAxisCombo);

    int xSeqIndex=-1, ySeqIndex=-1;
    int curIndex = 0;
    foreach (ADVSequenceObjectContext *s, sequences) {
        Q_ASSERT(s);

        xAxisCombo->addItem(s->getSequenceGObject()->getGObjectName());
        yAxisCombo->addItem(s->getSequenceGObject()->getGObjectName());

        if (sequenceX == s) {
            xSeqIndex = curIndex;
        }
        if (sequenceY == s) {
            ySeqIndex = curIndex;
        }

        curIndex++;
    }

    // choose the second sequence for Y axis
    if (sequences.size() > 1) {
        yAxisCombo->setCurrentIndex(1);
    }

    if (xSeqIndex >= 0) {
        xAxisCombo->setCurrentIndex(xSeqIndex);
    }
    if (ySeqIndex >= 0) {
        yAxisCombo->setCurrentIndex(ySeqIndex);
    }

    Q_ASSERT(minLenBox);
    Q_ASSERT(identityBox);
    minLenBox->setValue(minLen);
    identityBox->setValue(identity);

    connect(minLenHeuristicsButton, SIGNAL(clicked()), SLOT(sl_minLenHeuristics()));
    connect(hundredPercentButton, SIGNAL(clicked()), SLOT(sl_hundredPercent()));

    connect(directCheckBox, SIGNAL(clicked()), SLOT(sl_directInvertedCheckBox()));
    connect(invertedCheckBox, SIGNAL(clicked()), SLOT(sl_directInvertedCheckBox()));

    connect(directColorButton, SIGNAL(clicked()), SLOT(sl_directColorButton()));
    connect(invertedColorButton, SIGNAL(clicked()), SLOT(sl_invertedColorButton()));

    connect(directDefaultColorButton, SIGNAL(clicked()), SLOT(sl_directDefaultColorButton()));
    connect(invertedDefaultColorButton, SIGNAL(clicked()), SLOT(sl_invertedDefaultColorButton()));
}

void DotPlotDialog::accept() {

    xSeq = sequences.at(xAxisCombo->currentIndex());
    ySeq = sequences.at(yAxisCombo->currentIndex());

    Q_ASSERT(xSeq);
    Q_ASSERT(ySeq);

    QDialog::accept();
}

void DotPlotDialog::sl_minLenHeuristics() {
    Q_ASSERT(identityBox);
    identityBox->setValue(100);

    // formula used here: nVariations / lenVariations = wantedResCount (==1000)
    // where nVariations == area size
    // lenVariations = 4^len where len is result
    // so we have len = ln(nVariations/wantedResCount)/ln(4)

    Q_ASSERT(xAxisCombo);
    Q_ASSERT(yAxisCombo);
    ADVSequenceObjectContext *xSequence = sequences.at(xAxisCombo->currentIndex());
    ADVSequenceObjectContext *ySequence = sequences.at(yAxisCombo->currentIndex());

    Q_ASSERT(xSequence);
    Q_ASSERT(ySequence);
    qint64 xSeqLen = xSequence->getSequenceLen();
    qint64 ySeqLen = ySequence->getSequenceLen();

    double nVariations = xSeqLen*ySeqLen;
    double resCount = 1000;
    Q_ASSERT(resCount);
    double len = log(nVariations / resCount) / log(double(4));

    Q_ASSERT(minLenBox);
    minLenBox->setValue((int)len);
}

void DotPlotDialog::sl_hundredPercent() {

    Q_ASSERT(identityBox);
    identityBox->setValue(100);
}

int DotPlotDialog::getMismatches() const {

    Q_ASSERT(identityBox);
    Q_ASSERT(minLenBox);
    return (100-identityBox->value()) * minLenBox->value()/ 100;
}

// which algorithm
RFAlgorithm DotPlotDialog::getAlgo() const {

    Q_ASSERT(algoCheck);
    if (algoCheck->isChecked()) {

        Q_ASSERT(algoCombo);
        int index = algoCombo->currentIndex();
        return RFAlgorithm(algoCombo->itemData(index).toInt());
    }

    return RFAlgorithm_Auto;
}

int DotPlotDialog::getMinLen() const {

    Q_ASSERT(minLenBox);
    return minLenBox->value();
}

bool DotPlotDialog::isDirect() const {

    return directCheckBox->isChecked();
}

bool DotPlotDialog::isInverted() const {

    return invertedCheckBox->isChecked();
}

void DotPlotDialog::sl_directInvertedCheckBox() {

    startButton->setEnabled(directCheckBox->isChecked() || invertedCheckBox->isChecked());
}

static const QString COLOR_STYLE("QPushButton { background-color: %1 }");

void DotPlotDialog::sl_directColorButton() {

    QColorDialog d(directColor, this);

    if (d.exec()) {
        directColor = d.selectedColor();
        directCheckBox->setChecked(true);
    }

    updateColors();
}

void DotPlotDialog::sl_invertedColorButton() {

    QColorDialog d(invertedColor, this);

    if (d.exec()) {
        invertedColor = d.selectedColor();
        invertedCheckBox->setChecked(true);
    }

    updateColors();
}

void DotPlotDialog::sl_directDefaultColorButton() {

    directColor = QColor();
    directCheckBox->setChecked(true);
    updateColors();
}

void DotPlotDialog::sl_invertedDefaultColorButton() {

    invertedColor = QColor();
    invertedCheckBox->setChecked(true);
    updateColors();
}

void DotPlotDialog::updateColors() {

    directColorButton->setStyleSheet(COLOR_STYLE.arg(directColor.name()));
    invertedColorButton->setStyleSheet(COLOR_STYLE.arg(invertedColor.name()));
}

}//namespace
