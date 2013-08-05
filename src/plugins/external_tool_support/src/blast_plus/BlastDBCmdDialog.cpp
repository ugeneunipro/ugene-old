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

#include "BlastDBCmdDialog.h"

#include <U2Gui/LastUsedDirHelper.h>

#include <QtGui/QFileDialog>

namespace U2 {

BlastDBCmdDialog::BlastDBCmdDialog(BlastDBCmdSupportTaskSettings &_settings, QWidget *_parent) :
        QDialog(_parent), settings(_settings)
{
    setupUi(this);

    connect(inputDbToolButton, SIGNAL(clicked()), SLOT(sl_onSelectInputDbButtonClick()) );
    connect(browseOutputButton, SIGNAL(clicked()), SLOT(sl_onSelectOutputFileButtonClick()) );
    connect(queryIdEdit, SIGNAL(textChanged( const QString& )), SLOT(sl_onQueryLineEditTextChanged()));
    connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
    connect(fetchButton, SIGNAL(clicked()), SLOT(sl_BlastDBCmd()));

    update();
}


void BlastDBCmdDialog::sl_BlastDBCmd(){
    
    settings.query = queryIdEdit->text();
    settings.databasePath = inputDbLineEdit->text();
    settings.outputPath = outputPathLineEdit->text();
    settings.isNuclDatabase = nucleotideTypeRadioButton->isChecked();

    accept();
}

void BlastDBCmdDialog::sl_onSelectInputDbButtonClick()
{
    LastUsedDirHelper lod("Database Directory");

    QString name;
    lod.url = name = QFileDialog::getOpenFileName(NULL, tr("Select a database file"), lod.dir);
    if (!name.isEmpty()) {
        QFileInfo fileInfo(name);
        QString dbName = fileInfo.filePath().replace(QRegExp(".(phr|pin|psq|nhr|nin|nsq)", Qt::CaseInsensitive), QString());
        inputDbLineEdit->setText(dbName);
        //File::join()
        //inputDbLineEdit->setText(fileInfo.dir().path() );
    }
    update();
}

void BlastDBCmdDialog::sl_onSelectOutputFileButtonClick()
{
    LastUsedDirHelper lod;
    lod.url = QFileDialog::getSaveFileName(this, tr("Set a result FASTA file name"), lod.dir);
    if (!lod.url.isEmpty()) {
        GUrl result = lod.url;
        if (result.lastFileSuffix().isEmpty()) {
            result = QString( "%1.fa" ).arg( result.getURLString() );
        }
        outputPathLineEdit->setText(result.getURLString());
    }
    update();
}

void BlastDBCmdDialog::update()
{
    bool dbPathIsSet = !inputDbLineEdit->text().isEmpty();
    bool outputPathIsSet = !outputPathLineEdit->text().isEmpty();
    bool queryIsSet = !queryIdEdit->text().isEmpty();

    fetchButton->setEnabled(dbPathIsSet && outputPathIsSet && queryIsSet);


}

void BlastDBCmdDialog::sl_onQueryLineEditTextChanged()
{
    update();
}

}//namespace
