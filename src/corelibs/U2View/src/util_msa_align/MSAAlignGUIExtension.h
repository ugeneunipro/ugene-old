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

#ifndef _U2_MSA_ALIGN_GUI_EXTENSION_H_
#define _U2_MSA_ALIGN_GUI_EXTENSION_H_


#include <U2Core/global.h>
#include <U2Core/GUrl.h>
#include <U2Core/Task.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QWidget>
#else
#include <QtWidgets/QWidget>
#endif

namespace U2 {

// These classes are intended for extending standard MSA Align dialog GUI
// with options specific to the align algorithm

class MSAAlignAlgorithmMainWidget : public QWidget {
public:
    MSAAlignAlgorithmMainWidget(QWidget* parent) : QWidget(parent) {}
    virtual QMap<QString,QVariant> getMSAAlignCustomSettings() = 0;
};

class MSAAlignGUIExtensionsFactory {
public:
    virtual ~MSAAlignGUIExtensionsFactory() {}
    virtual MSAAlignAlgorithmMainWidget* createMainWidget(QWidget* parent) = 0;
    virtual bool hasMainWidget() = 0;
};

} // U2

#endif // _U2_MSA_ALIGN_GUI_EXTENSION_H_
