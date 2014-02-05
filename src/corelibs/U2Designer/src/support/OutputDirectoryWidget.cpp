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

#include <QtCore/QEvent>

#include <QtGui/QFileDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>

#include <U2Core/U2SafePoints.h>

#include <U2Lang/WorkflowSettings.h>

#include "OutputDirectoryWidget.h"

namespace U2 {

const QString OutputDirectoryWidget::INFO = QObject::tr(
    "The Workflow Output Directory is a common directory that is used to store all output files in the Workflow Designer."
    " A separate subdirectory of the directory is created for each run of a workflow."
    "\n\nSet up the directory:"
    );

OutputDirectoryWidget::OutputDirectoryWidget(QWidget *parent, bool commitOnHide)
: QWidget(parent), commitOnHide(commitOnHide)
{
    QVBoxLayout *l = new QVBoxLayout(this);
    l->setContentsMargins(3, 3, 3, 3);
    label = new QLabel(INFO, this);
    label->setAlignment(Qt::AlignJustify | Qt::AlignVCenter);
    label->setWordWrap(true);
    label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    l->addWidget(label);
    QHBoxLayout *hl = new QHBoxLayout();
    hl->setContentsMargins(0, 0, 0, 0);
    pathEdit = new QLineEdit(this);
    browseButton = new QPushButton("...", this);
    browseButton->setFixedSize(25, 19);
    hl->addWidget(pathEdit);
    hl->addWidget(browseButton);

    l->addLayout(hl);

    connect(browseButton, SIGNAL(clicked()), SLOT(sl_browse()));
    pathEdit->setText(WorkflowSettings::getWorkflowOutputDirectory());
}

void OutputDirectoryWidget::sl_browse() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select a directory"), pathEdit->text());

    if(!dir.isEmpty()) {
        if (!dir.endsWith(QDir::separator())) {
            dir += QDir::separator();
        }
        pathEdit->setText(dir);
        WorkflowSettings::setWorkflowOutputDirectory(dir);
    }
    emit si_browsed();
}

void OutputDirectoryWidget::commit() {
    WorkflowSettings::setWorkflowOutputDirectory(pathEdit->text());
}

void OutputDirectoryWidget::hideEvent(QHideEvent *event) {
    if (commitOnHide) {
        commit();
    }
    QWidget::hideEvent(event);
}

} // U2
