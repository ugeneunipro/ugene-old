/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_PAIRWISE_ALIGNMENT_GUI_EXTENSION_H_
#define _U2_PAIRWISE_ALIGNMENT_GUI_EXTENSION_H_

#include <U2Core/global.h>

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QVariantMap>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QPointer>

#include <QtGui/QWidget>


namespace U2 {

class PairwiseAlignmentTaskSettings;

// These classes are intended for extending pairwise alignment GUIs
// with options specific to the align algorithm

class U2VIEW_EXPORT PairwiseAlignmentMainWidget : public QWidget
{
    Q_OBJECT

public:
    PairwiseAlignmentMainWidget(QWidget* parent, QVariantMap* s);
    virtual ~PairwiseAlignmentMainWidget();

    virtual QVariantMap getPairwiseAlignmentCustomSettings(bool append);

public slots:
    void sl_externSettingsInvalide();

protected:
    virtual void fillInnerSettings();

protected:
    QVariantMap innerSettings;        //inner settings
    QVariantMap* externSettings;      //extern settings (from msa)
    bool externSettingsExists;                  //msa editor is alive, pointer externSettings is valid
};

class U2VIEW_EXPORT PairwiseAlignmentGUIExtensionFactory : public QObject {
    Q_OBJECT

public:
    virtual PairwiseAlignmentMainWidget* createMainWidget(QWidget* parent, QVariantMap* s) = 0;
    virtual bool hasMainWidget(const QWidget* parent);

protected slots:
    virtual void sl_widgetDestroyed(QObject * obj);

protected:
    QMap<const QWidget*, PairwiseAlignmentMainWidget*> mainWidgets;
};

}   //namespace

#endif  //_U2_PAIRWISE_ALIGNMENT_GUI_EXTENSION_H_
