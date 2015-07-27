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

#include <QMainWindow>

#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceUtils.h>
#include <U2Gui/MainWindow.h>
#include <U2Core/U2SafePoints.h>

#include <U2Core/QObjectScopedPointer.h>

#include "PrimerGroupBox.h"
#include "PrimerLibrarySelector.h"
#include "PrimerStatistics.h"

namespace U2 {

PrimerGroupBox::PrimerGroupBox(QWidget *parent)
: QWidget(parent)
{
    setupUi(this);

    connect(primerEdit, SIGNAL(textChanged(const QString &)), SLOT(sl_onPrimerChanged(const QString &)));
    connect(reverseComplementButton, SIGNAL(clicked()), SLOT(sl_translate()));
    connect(browseButton, SIGNAL(clicked()), SLOT(sl_browse()));
}

void PrimerGroupBox::sl_onPrimerChanged(const QString &sequence) {
    QString characteristics;
    if (!sequence.isEmpty()) {
        characteristics += getTmString(sequence) + ", ";
        characteristics += QString::number(sequence.length()) + tr("-mer");
    }
    characteristicsLabel->setText(characteristics);

    emit si_primerChanged();
}

QByteArray PrimerGroupBox::getPrimer() const {
    return primerEdit->text().toLocal8Bit();
}

uint PrimerGroupBox::getMismatches() const {
    int value = mismatchesSpinBox->value();
    SAFE_POINT(value >= 0, "Negative mismatches count", 0);
    return uint(value);
}

void PrimerGroupBox::sl_translate() {
    const QByteArray translation = DNASequenceUtils::reverseComplement(primerEdit->text().toLocal8Bit());
    primerEdit->setInvalidatedText(translation);
}

void PrimerGroupBox::sl_browse() {
    QObjectScopedPointer<PrimerLibrarySelector> dlg = new PrimerLibrarySelector(AppContext::getMainWindow()->getQMainWindow());
    dlg->exec();
    CHECK(!dlg.isNull(), );
    CHECK(QDialog::Accepted == dlg->result(), );
    Primer result = dlg->getResult();
    primerEdit->setInvalidatedText(result.sequence);
}

QString PrimerGroupBox::getDoubleStringValue(double value) {
    QString result = QString::number(value, 'f', 2);
    result.remove(QRegExp("\\.?0+$"));
    return result;
}

QString PrimerGroupBox::getTmString(const QString &sequence) {
    double tm = PrimerStatistics::getMeltingTemperature(sequence.toLocal8Bit());
    QString tmString = getDoubleStringValue(tm);
    return tr("Tm = ") + tmString + QString::fromLatin1("\x00B0") + "C";
}

} // U2
