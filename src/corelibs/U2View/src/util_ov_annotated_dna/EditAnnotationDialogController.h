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

#ifndef _U2_EDIT_ANNOTATION_DIALOG_CONTROLLER_H_
#define _U2_EDIT_ANNOTATION_DIALOG_CONTROLLER_H_

#include "ui/ui_EditAnnotationDialog.h"

#include <U2Core/U2Region.h>
#include <U2Core/AnnotationTableObject.h>

namespace U2{

class U2VIEW_EXPORT EditAnnotationDialogController : public QDialog, public Ui_EditAnnotationDialog {
    Q_OBJECT
public:
    EditAnnotationDialogController(Annotation* a, U2Region _seqRange, QWidget* p = NULL);

    virtual void accept ();

    QString getName() const {return currentName;}
    U2Location getLocation() const {return location;}
    
private slots:
    void sl_onTextChanged(const QString&);
    void sl_setPredefinedAnnotationName();
    void sl_complementLocation();

private:
    QMenu* createAnnotationNamesMenu(QWidget* p, QObject* receiver);

    U2Region            seqRange;
    U2Location          location;
    QString             currentName;
};

}//ns

#endif //_U2_EDIT_ANNOTATION_DIALOG_CONTROLLER_H_
