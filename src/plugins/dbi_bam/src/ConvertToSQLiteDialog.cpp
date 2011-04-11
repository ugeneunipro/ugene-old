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

#include <U2Core/Task.h>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QTextEdit>
#include <U2Misc/DialogUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Gui/ObjectViewModel.h>
#include "BAMDbiPlugin.h"
#include "BaiReader.h"
#include "ConvertToSQLiteDialog.h"
#include "LoadBamInfoTask.h"

namespace U2 {
namespace BAM {

ConvertToSQLiteDialog::ConvertToSQLiteDialog(const GUrl& _sourceUrl, bool hasProject, BAMInfo& _bamInfo) : QDialog(QApplication::activeWindow()), sourceUrl(_sourceUrl), bamInfo(_bamInfo) {
    ui.setupUi(this);
    
    if(!hasProject) {
        ui.addToProjectBox->setChecked(false);
        ui.addToProjectBox->setVisible(false);
    }

    connect(ui.bamInfoButton, SIGNAL(clicked()), SLOT(sl_bamInfoButtonClicked()));

    ui.tableWidget->setColumnCount(3);
    ui.tableWidget->setRowCount(bamInfo.getHeader().getReferences().count());
    QStringList header; header << BAMDbiPlugin::tr("Contig name") << BAMDbiPlugin::tr("Length") << BAMDbiPlugin::tr("URI");
    ui.tableWidget->setHorizontalHeaderLabels(header);
    ui.tableWidget->horizontalHeader()->setStretchLastSection(true);    
    {
        int i = 0;
        foreach(const Header::Reference& ref, bamInfo.getHeader().getReferences()) {
            QTableWidgetItem* checkbox = new QTableWidgetItem();
            checkbox->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);            
            checkbox->setText(ref.getName());
            ui.tableWidget->setItem(i, 0, checkbox);
            QTableWidgetItem* item = new QTableWidgetItem(QString::number(ref.getLength()));
            item->setFlags(Qt::ItemIsEnabled);
            ui.tableWidget->setItem(i, 1, item);
            ui.tableWidget->setCellWidget(i, 2, new QLabel("<a href=\"" + ref.getUri().getURLString() + "\">" + ref.getUri().getURLString() + "</a>"));
            checkbox->setCheckState(Qt::Checked);
            i++;
        }
    }
    ui.destinationUrlEdit->setText(sourceUrl.dirPath() + "/" + sourceUrl.baseFileName() + ".ugenedb");
    ui.sourceUrlView->setText(sourceUrl.getURLString());
    ui.okButton->setFocus();
    connect(ui.tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), SLOT(sl_contigCheckChanged(QTableWidgetItem*)));
}

void ConvertToSQLiteDialog::sl_bamInfoButtonClicked() {
    const Header& header = bamInfo.getHeader();
    QDialog dialog(this);
    QList<QPair<QString, QString> > list;
    QString sort;
    switch(header.getSortingOrder()) {
        case Header::Unsorted: sort = BAMDbiPlugin::tr("Unsorted"); break;
        case Header::Unknown: sort = BAMDbiPlugin::tr("Unknown"); break;
        case Header::Coordinate: sort = BAMDbiPlugin::tr("Coordinate"); break;
        case Header::QueryName: sort = BAMDbiPlugin::tr("Query name"); break;
    }
    list << QPair<QString, QString>(BAMDbiPlugin::tr("Format Version"), header.getFormatVersion().text) 
        << QPair<QString, QString>(BAMDbiPlugin::tr("Sorting order"), sort);

    foreach(const Header::Program& prog, header.getPrograms()) {
        list << QPair<QString, QString>(BAMDbiPlugin::tr("Program"), "Name: " + prog.getName() + ", Version: " + prog.getVersion() + ", Command: " + prog.getCommandLine());
    }

    QTableWidget* table = new QTableWidget();
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels(QStringList() << BAMDbiPlugin::tr("Property name") << BAMDbiPlugin::tr("Value"));
    table->horizontalHeader()->setStretchLastSection(true);
    table->setRowCount(list.count());
    dialog.setLayout(new QVBoxLayout());
    dialog.layout()->addWidget(table);

    {
        for(int i=0; i<list.count(); i++) {
            const QPair<QString, QString>& pair = list.at(i);
            QTableWidgetItem* item = new QTableWidgetItem(pair.first);
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            table->setItem(i, 0, item);
            item = new QTableWidgetItem(pair.second);
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            table->setItem(i, 1, item);
        }
    }
    
    dialog.exec();
}

void ConvertToSQLiteDialog::sl_contigCheckChanged(QTableWidgetItem * item) {
    bamInfo.getSelected()[item->row()] = (item->checkState() == Qt::Checked);
}

const GUrl &ConvertToSQLiteDialog::getDestinationUrl()const {
    return destinationUrl;
}

bool ConvertToSQLiteDialog::addToProject() const {
    return ui.addToProjectBox->isChecked();
}

void ConvertToSQLiteDialog::accept() {
    destinationUrl = GUrl(ui.destinationUrlEdit->text());
    if(destinationUrl.isEmpty()) {
        ui.destinationUrlEdit->setFocus(Qt::OtherFocusReason);
        QMessageBox::critical(this, windowTitle(), BAMDbiPlugin::tr("Destinaiton URL is not specified"));
    } else if(!destinationUrl.isLocalFile()) {
        ui.destinationUrlEdit->setFocus(Qt::OtherFocusReason);
        QMessageBox::critical(this, windowTitle(), BAMDbiPlugin::tr("Destinaiton URL must point to a local file"));
    } else {
        bool selected = false;
        foreach(const bool& i, bamInfo.getSelected()) {
            if(i) {
                selected = true;
                break;
            }
        }
        if(!selected) {
            QMessageBox::critical(this, windowTitle(), BAMDbiPlugin::tr("At least one contig must be selected."));
            return;
        }
        Project * prj = AppContext::getProject();
        if(prj != NULL) {
            Document * destDoc = prj->findDocumentByURL(destinationUrl);
            if(destDoc != NULL && destDoc->isLoaded() && !GObjectViewUtils::findViewsWithAnyOfObjects(destDoc->getObjects()).isEmpty()) {
                QMessageBox::critical(this, windowTitle(), BAMDbiPlugin::tr("There is opened view with destination file.\n"
                                                                            "Close it or choose different file"));
                ui.destinationUrlEdit->setFocus(Qt::OtherFocusReason);
                return;
            }
        }
        if(QFile::exists(destinationUrl.getURLString())) {
            int result = QMessageBox::question(this, windowTitle(), 
                                               BAMDbiPlugin::tr("Destination file already exists.\n"
                                                                "To overwrite the file, press 'Replace'.\n"
                                                                "To append data to existing file press 'Append'."), 
                                               BAMDbiPlugin::tr("Replace"), 
                                               BAMDbiPlugin::tr("Append"), 
                                               BAMDbiPlugin::tr("Cancel"), 2);
            switch(result) {
                case 0: 
                    {
                        bool ok = QFile::remove(destinationUrl.getURLString());
                        if(!ok) {
                            coreLog.error(BAMDbiPlugin::tr("Destination file '%1' cannot be removed").arg(destinationUrl.getURLString()));
                        }    
                    }
                case 1:
                    QDialog::accept();
                    break;
            }
        } else {
            QDialog::accept();
        }
        
    }
}

static const QString DIR_HELPER_DOMAIN("ConvertToSQLiteDialog");
void U2::BAM::ConvertToSQLiteDialog::on_destinationUrlButton_clicked() {
    QString dir = sourceUrl.dirPath() + "/" + sourceUrl.baseFileName();
    QString returnedValue = QFileDialog::getSaveFileName(this, BAMDbiPlugin::tr("Destination SQLite File"), dir, BAMDbiPlugin::tr("SQLite Files (*.ugenedb);;All Files (*)"), NULL, QFileDialog::DontConfirmOverwrite);
    if(!returnedValue.isEmpty()) {
        ui.destinationUrlEdit->setText(returnedValue);
    }
}

} // namespace BAM
} // namespace U2
