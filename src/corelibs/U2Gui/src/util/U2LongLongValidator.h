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

#ifndef _U2_LONG_LONG_VALIDATOR_H_
#define _U2_LONG_LONG_VALIDATOR_H_

#include <QtGui/QValidator>

#include <U2Core/global.h>

namespace U2 {

class U2GUI_EXPORT U2LongLongValidator : public QValidator {
    Q_OBJECT
public:
    U2LongLongValidator(qint64 minimum, qint64 maximum, QObject *parent = 0);

    State validate(QString &input, int &pos) const;
    
private:
    qint64 minimum;
    qint64 maximum;
};

}   // namespace U2

#endif // _U2_LONG_LONG_VALIDATOR_H_
