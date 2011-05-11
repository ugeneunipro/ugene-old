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

#ifndef _U2_DNA_SEQUENCE_GENERATOR_DIALOG_H_
#define _U2_DNA_SEQUENCE_GENERATOR_DIALOG_H_

#include "ui_DNASequenceGeneratorDialog.h"
#include "ui_BaseContentDialog.h"


namespace U2 {

class SaveDocumentGroupController;

class DNASequenceGeneratorDialog : public QDialog, public Ui_DNASequenceGeneratorDialog {
    Q_OBJECT
public:
    DNASequenceGeneratorDialog(QWidget* p=NULL);
private slots:
    void sl_browseReference();
    void sl_configureContent();
    void sl_generate();
    void sl_refButtonToggled(bool checked);
private:
    SaveDocumentGroupController* saveGroupContoller;
    static QMap<char, qreal> content;
};

class BaseContentDialog : public QDialog, public Ui_BaseContentDialog {
    Q_OBJECT
public:
    BaseContentDialog(QMap<char, qreal>& percentMap_, QWidget* p=NULL);
private slots:
    void sl_save();
    void sl_baseClicked();
    void sl_gcSkewClicked();
private:
    QMap<char, qreal>& percentMap;
    float gcSkew;
};

} //namespace

#endif
