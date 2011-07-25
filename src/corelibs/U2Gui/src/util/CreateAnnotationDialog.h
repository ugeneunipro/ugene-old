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

#ifndef _U2_CREATE_ANNOTATION_DIALOG_H_
#define _U2_CREATE_ANNOTATION_DIALOG_H_

#include <U2Core/global.h>

#include <QtGui/QDialog>

namespace U2 {

class CreateAnnotationWidgetController;
class CreateAnnotationModel;

class U2GUI_EXPORT CreateAnnotationDialog : public QDialog {
Q_OBJECT
public:
    //saves the results to the provided model
	CreateAnnotationDialog(QWidget* p, CreateAnnotationModel& m);
    
private slots:
	void sl_onCreateClicked(bool);
	void sl_onCancelClicked(bool);

private:
    CreateAnnotationModel& model;
    CreateAnnotationWidgetController* annWidgetController;
    QPushButton* createButton;
	QPushButton* cancelButton;
};

} // namespace

#endif
