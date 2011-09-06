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

#ifndef _CREATEPHYTREEWIDGET_H_
#define _CREATEPHYTREEWIDGET_H_

#include <U2Core/global.h>
#include <U2Algorithm/CreatePhyTreeSettings.h>
#include <QtGui/QWidget>

namespace U2 {

class MAlignment;
class CreatePhyTreeSettings;

class U2VIEW_EXPORT CreatePhyTreeWidget : public QWidget {
public:
    static const QString settingsPath;
    CreatePhyTreeWidget(QWidget* parent) : QWidget(parent) { }
    virtual void fillSettings(CreatePhyTreeSettings& settings) {Q_UNUSED(settings); } 
    virtual void storeSettings() {;}
    virtual void restoreDefault() {;}
    virtual bool checkSettings(QString& msg, const CreatePhyTreeSettings& settings) {return true;}
    virtual bool checkMemoryEstimation(QString& msg, const MAlignment& msa, const CreatePhyTreeSettings& settings) {return true;}
};
 
} //namespace

#endif
