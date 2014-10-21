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

#ifndef _U2_PCR_GROUP_BOX_H_
#define _U2_PCR_GROUP_BOX_H_

#include <QWidget>

#include "ui_PrimerGroupBox.h"

namespace U2 {

class PrimerGroupBox : public QWidget, public Ui::PrimerGroupBox {
    Q_OBJECT
public:
    PrimerGroupBox(QWidget *parent);

    void setTitle(const QString &title);

    QByteArray getPrimer() const;
    uint getMismatches() const;

    static QString getDoubleStringValue(double value);

signals:
    void si_primerChanged();

private slots:
    void sl_onPrimerChanged(const QString &sequence);
    void sl_translate();

private:
    static QString getTmString(const QString &sequence);
};

} // U2

#endif // _U2_PCR_GROUP_BOX_H_
