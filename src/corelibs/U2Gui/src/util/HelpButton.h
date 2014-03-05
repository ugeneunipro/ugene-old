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

#include "U2Core/global.h"

#include <QtCore/QString>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QWidget>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QAbstractButton>
#else
#include <QtWidgets/QWidget>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QAbstractButton>
#endif

namespace U2 {

class U2GUI_EXPORT HelpButton: public QObject {
    Q_OBJECT

public:
    HelpButton(QObject *parent, QDialogButtonBox *b, const QString& pageId);
    HelpButton(QObject *parent, QAbstractButton *b, const QString& pageId);

private slots:
    void sl_buttonClicked();

private:
    QString pageId;
};

}
