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

#ifndef _U2_CREATE_RULER_DIALOG_CONTROLLER_H_
#define _U2_CREATE_RULER_DIALOG_CONTROLLER_H_


#include <ui/ui_CreateRulerDialog.h>

#include <U2Core/U2Region.h>

#include <QtCore/QSet>
#include <QtGui/QDialog>

namespace U2 {

class U2VIEW_EXPORT CreateRulerDialogController : public QDialog, public Ui_CreateRulerDialog {
    Q_OBJECT
public:
    CreateRulerDialogController(const QSet<QString>& namesToFilter, const U2Region& range, int offset = -1, QWidget* p=NULL);
    
    virtual void accept ();

private slots:
    void sl_colorButtonClicked();

private:
    void updateColorSample();

public:
    QString         name;
    int             offset;
    QColor          color;
    QSet<QString>   filter;

};


}// namespace

#endif
