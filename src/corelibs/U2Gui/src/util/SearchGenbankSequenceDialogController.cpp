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

#include "ui/ui_SearchGenbankSequenceDialog.h"

#include "SearchGenbankSequenceDialogController.h"

namespace U2 {

SearchGenbankSequenceDialogController::SearchGenbankSequenceDialogController(QWidget *p) : QDialog(p)
{
    ui = new Ui_SearchGenbankSequenceDialog();
    ui->setupUi(this);

    queryBlockController = new QueryBuilderController(this);
    connect(ui->addQueryBlockButton, SIGNAL(clicked()), queryBlockController, SLOT(sl_addQueryBlockWidget()) );
}

SearchGenbankSequenceDialogController::~SearchGenbankSequenceDialogController()
{


}

void SearchGenbankSequenceDialogController::addQueryBlockWidget(QWidget *w)
{
    ui->queryBuilderBox->layout()->addWidget(w);
}

void SearchGenbankSequenceDialogController::removeQueryBlockWidget(QWidget *w)
{
    ui->queryBuilderBox->layout()->removeWidget(w);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

QueryBuilderController::QueryBuilderController(SearchGenbankSequenceDialogController *p) : parentController(p)
{


}


void QueryBuilderController::sl_addQueryBlockWidget()
{
    QWidget* widget = new QWidget();
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, widget);
    layout->setMargin(0);

    QComboBox* conditionBox = new QComboBox(widget);

    QComboBox* termBox = new QComboBox(widget);
    QLineEdit* queryEdit = new QLineEdit(widget);
    layout->addWidget(termBox);
    layout->addWidget(queryEdit);

    QToolButton* removeBlockButton = new QToolButton();
    removeBlockButton->setText("-");
    layout->addWidget(removeBlockButton);
    connect(removeBlockButton,  SIGNAL(clicked()), SLOT(sl_removeQueryBlockWidget()) );

    removeWidgetCallbacks.insert(removeBlockButton, widget );

    widget->setLayout(layout);


    parentController->addQueryBlockWidget(widget);

}

void QueryBuilderController::sl_removeQueryBlockWidget()
{
    QToolButton* callbackButton = qobject_cast<QToolButton*> ( sender() );
    assert(callbackButton);

    QWidget* queryBlockWidget = removeWidgetCallbacks.take(callbackButton);
    assert(queryBlockWidget);

    parentController->removeQueryBlockWidget(queryBlockWidget);

    delete queryBlockWidget;


}



} //namespace 
