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

#include <U2Core/U2SafePoints.h>
#include "ADVGraphModel.h"
#include "WindowStepSelectorWidget.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QColorDialog>
#include <QtGui/QMessageBox>

#include "GraphSettingsDialog.h"

#define BACKGROUND_COLOR "QPushButton { background-color : %1;}"

namespace U2 {

GraphSettingsDialog::GraphSettingsDialog( GSequenceGraphDrawer* d, const U2Region& range, QWidget* parent )
:QDialog(parent), colorMap(d->getColors())
{
	
	const GSequenceGraphWindowData& windowData = d->getWindowData();
	const GSequenceGraphMinMaxCutOffData& cutOffData = d->getCutOffData();
	wss = new WindowStepSelectorWidget(this, range, windowData.window, windowData.step);
	mms = new MinMaxSelectorWidget(this, cutOffData.min, cutOffData.max, cutOffData.enableCuttoff);
	QVBoxLayout* l = new QVBoxLayout();
	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	buttonsLayout->addStretch(10);
	
	QList<QHBoxLayout*> cLayouts;
	foreach(const QString& key, colorMap.keys()) {
		QLabel* defaultColorName = new QLabel(QString("%1:").arg(key));
		QPushButton* colorChangeButton = new QPushButton();
		colorChangeButton->setObjectName(key);
		colorChangeButton->setFixedSize(QSize(25,25));
		connect(colorChangeButton, SIGNAL(clicked()), SLOT(sl_onPickColorButtonClicked()));
		QColor color = colorMap.value(key);
		colorChangeButton->setStyleSheet(QString(BACKGROUND_COLOR).arg(color.name()));
		QHBoxLayout* cl = new QHBoxLayout();
		cl->addSpacerItem(new QSpacerItem(50,25, QSizePolicy::Expanding));
		cl->addWidget(defaultColorName);
		cl->addWidget(colorChangeButton);
		cl->addSpacerItem(new QSpacerItem(50,25, QSizePolicy::Expanding));
		cLayouts.append(cl);
		
	}		
	
	QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
	QPushButton* okButton = new QPushButton(tr("Ok"), this);
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);

	l->addWidget(wss);
	foreach(QLayout* cl, cLayouts) {
		l->addLayout(cl);
	}
	l->addWidget(mms);
	l->addLayout(buttonsLayout);

	setLayout(l);
	setWindowTitle(tr("Graph Settings"));
	setWindowIcon(QIcon(":core/images/graphs.png"));

	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	setMinimumWidth(200);

	connect(cancelButton, SIGNAL(clicked()), SLOT(sl_onCancelClicked()));
	connect(okButton, SIGNAL(clicked()), SLOT(sl_onOkClicked()));

	okButton->setDefault(true);
}

void GraphSettingsDialog::sl_onPickColorButtonClicked()
{
	QPushButton* colorButton = qobject_cast<QPushButton*> (sender());
	SAFE_POINT(colorButton, "Button for color is NULL", );

	QString colorName = colorButton->objectName();
	QColor initial = colorMap.value(colorName);

    QColorDialog CD(initial, this);

    if (CD.exec() == QDialog::Accepted){
        QColor newColor = CD.selectedColor();
        colorMap[colorName] = newColor;
        colorButton->setStyleSheet(QString(BACKGROUND_COLOR).arg(newColor.name()));
    }
}

void GraphSettingsDialog::sl_onCancelClicked()
{
	reject();
}

void GraphSettingsDialog::sl_onOkClicked()
{
	QString err = wss->validate();
	QString mmerr = mms->validate();
	if (err.isEmpty() && mmerr.isEmpty()) {
		accept();
		return;
	}
	QMessageBox::critical(this, windowTitle(), err.append(' ').append(mmerr));
}

} // namespace 