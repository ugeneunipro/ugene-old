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

#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

#include "DashboardsManagerDialog.h"

static const QString REMOVE_DASHBOARDS_MESSAGE_BOX_TITLE = QObject::tr( "Removing Dashboards" );
static const QString REMOVE_MULTIPLE_DASHBOARDS_MESSAGE_BOX_TEXT
    = QObject::tr( "The following dashboards are about to be deleted:" );
static const QString REMOVE_SINGLE_DASHBOARD_MESSAGE_BOX_TEXT
    = QObject::tr( "The following dashboard is about to be deleted:" );
static const QString DASHBOARD_NAME_LIST_START = "<ul style=\"margin-top:5px;margin-bottom:0px\"><li>";
static const QString DASHBOARD_NAMES_DELIMITER = "</li><li>";
static const QString DASHBOARD_NAME_LIST_END = "</li></ul>";
static const QString SKIP_EXTENSION = "...";
static const int DASHBOARD_NAME_DISPLAYING_SYMBOLS_COUNT = 30;
static const int DASHBOARD_MAX_DISPLAING_NAME_COUNT = 5;

namespace U2 {

DashboardsManagerDialog::DashboardsManagerDialog(ScanDashboardsDirTask *_task, QWidget *parent)
: QDialog(parent), task(_task)
{
    setupUi(this);
    setupList();

    connect(checkButton, SIGNAL(clicked()), SLOT(sl_check()));
    connect(uncheckButton, SIGNAL(clicked()), SLOT(sl_uncheck()));
    connect(allButton, SIGNAL(clicked()), SLOT(sl_selectAll()));
    connect(removeButton, SIGNAL(clicked()), SLOT(sl_remove()));
}

void DashboardsManagerDialog::setupList() {
    QStringList header;
    header << tr("Name") << tr("Directory");
    listWidget->setHeaderLabels(header);
    listWidget->header()->setMovable(false);

    const int defaultNameColumnWidth = 250;
    listWidget->header()->resizeSection(0, defaultNameColumnWidth);

    foreach (const DashboardInfo &info, task->getResult()) {
        QStringList data;
        data << info.name << info.dirName;
        QTreeWidgetItem *item = new QTreeWidgetItem(listWidget, data);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        Qt::CheckState state = info.opened ? Qt::Checked : Qt::Unchecked;
        item->setCheckState(0, state);
        item->setData(0, Qt::UserRole, qVariantFromValue<DashboardInfo>(info));
        listWidget->addTopLevelItem(item);
    }
    listWidget->sortByColumn(1, Qt::AscendingOrder);
}

QList<QTreeWidgetItem*> DashboardsManagerDialog::allItems() {
    return listWidget->findItems("*", Qt::MatchWildcard);
}

QList<DashboardInfo> DashboardsManagerDialog::selectedDashboards() {
    QList<DashboardInfo> result;
    foreach (QTreeWidgetItem *item, allItems()) {
        if (Qt::Checked == item->checkState(0)) {
            result << item->data(0, Qt::UserRole).value<DashboardInfo>();
        }
    }
    return result;
}

QList<DashboardInfo> DashboardsManagerDialog::removedDashboards() {
    return removed;
}

void DashboardsManagerDialog::sl_check() {
    foreach (QTreeWidgetItem *item, listWidget->selectedItems()) {
        item->setCheckState(0, Qt::Checked);
    }
}

void DashboardsManagerDialog::sl_uncheck() {
    foreach (QTreeWidgetItem *item, listWidget->selectedItems()) {
        item->setCheckState(0, Qt::Unchecked);
    }
}

void DashboardsManagerDialog::sl_selectAll() {
    foreach (QTreeWidgetItem *item, allItems()) {
        item->setSelected(true);
    }
}

void DashboardsManagerDialog::sl_remove() {
    if ( !confirmDashboardsRemoving( ) ) {
        return;
    }

    foreach (QTreeWidgetItem *item, listWidget->selectedItems()) {
        removed << item->data(0, Qt::UserRole).value<DashboardInfo>();
        delete item;
    }
}

bool DashboardsManagerDialog::confirmDashboardsRemoving( ) const {
    QList<QTreeWidgetItem *> selectedItems = listWidget->selectedItems( );
    if ( selectedItems.isEmpty( ) ) {
        return false;
    }
    // build name list of selected dashboards
    QString warningMessageText = ( 1 == selectedItems.count( ) )
        ? REMOVE_SINGLE_DASHBOARD_MESSAGE_BOX_TEXT
        : REMOVE_MULTIPLE_DASHBOARDS_MESSAGE_BOX_TEXT;
    warningMessageText += DASHBOARD_NAME_LIST_START;

    QString fullDashboardNamesList;

    const bool tooManyDashboardsSelected
        = ( DASHBOARD_MAX_DISPLAING_NAME_COUNT < selectedItems.count( ) );
    int dashboardCounter = 0;
    foreach ( QTreeWidgetItem *item, selectedItems ) {
        QString dashboardName = item->data( 0, Qt::DisplayRole ).value<QString>( );
        if ( tooManyDashboardsSelected ) {
            fullDashboardNamesList += " - " + dashboardName + "\n";
        }

        if ( DASHBOARD_MAX_DISPLAING_NAME_COUNT >= ++dashboardCounter ) {
            // cut long names
            if ( DASHBOARD_NAME_DISPLAYING_SYMBOLS_COUNT < dashboardName.size( ) ) {
                dashboardName = dashboardName.left( DASHBOARD_NAME_DISPLAYING_SYMBOLS_COUNT );
                dashboardName += SKIP_EXTENSION;
            }
            warningMessageText += dashboardName;
            warningMessageText += DASHBOARD_NAMES_DELIMITER;
        }
    }
    // remove last delimiter
    warningMessageText = warningMessageText.left(
        warningMessageText.length( ) - DASHBOARD_NAMES_DELIMITER.size( ) );
    warningMessageText += DASHBOARD_NAME_LIST_END;
    if ( tooManyDashboardsSelected ) {
        warningMessageText += "<pre style=\"margin-top:0px;\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
            + SKIP_EXTENSION + "</pre>";
    }

    QMessageBox questionBox;
    questionBox.setIcon( QMessageBox::Question );
    questionBox.setWindowTitle( REMOVE_DASHBOARDS_MESSAGE_BOX_TITLE );
    questionBox.setText( warningMessageText );
    if ( tooManyDashboardsSelected ) {
        questionBox.setDetailedText( fullDashboardNamesList );
    }
    QPushButton *confirmButton = questionBox.addButton( tr( "Confirm" ), QMessageBox::ApplyRole );
    const QPushButton *cancelButton = questionBox.addButton( tr( "Cancel" ),
        QMessageBox::RejectRole );
    questionBox.setDefaultButton( confirmButton );
    questionBox.exec( );

    return ( dynamic_cast<const QAbstractButton *>( cancelButton )
        != questionBox.clickedButton( ) );
}

} // U2
