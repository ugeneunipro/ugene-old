/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef __ASSEMBLY_INFO_WIDGET_H__
#define __ASSEMBLY_INFO_WIDGET_H__

#include <QtGui/QtGui>
#include <U2Core/U2Region.h>

namespace U2 {

class AssemblyBrowser;

class CoveredRegionsLabel : public QLabel {
    Q_OBJECT
public:
    CoveredRegionsLabel(AssemblyBrowser * browser, QWidget *p = NULL);

    void setAdditionalText(QString prefix, QString postfix);

public slots:
    void sl_updateContent();

private:
    AssemblyBrowser * browser;
    QString prefix;
    QString postfix;
};

class AssemblyInfoWidget : public QWidget {
    Q_OBJECT
public:
    AssemblyInfoWidget(AssemblyBrowser * browser, QWidget *p = NULL);

private slots:
    void sl_copyInfo(QString info);
};

} // namespace

#endif // #ifndef __ASSEMBLY_INFO_WIDGET_H__
