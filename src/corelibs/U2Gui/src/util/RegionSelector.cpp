/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "RegionSelector.h"
#include <U2Core/L10n.h>

#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QToolButton>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QGroupBox>
#include <QtGui/QIntValidator>
#include <QtGui/QMessageBox>
#include <QtGui/QPalette>
#include <QtGui/QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <math.h>

namespace U2 {
////////////////////////////////////////
// RangeSelectorWidget
RegionSelector::RegionSelector(QWidget* p, qint64 _len, bool _isVertical, DNASequenceSelection* _selection)
    : QWidget(p), maxLen(_len), startEdit(NULL), endEdit(NULL), isVertical(_isVertical)
{
    needAddSelectionButton=true;
    selection = NULL;
    if (_selection != NULL && !_selection->isEmpty()) {
        selection = _selection;
        region.startPos = selection->getSelectedRegions().first().startPos;
        region.length = selection->getSelectedRegions().first().endPos() - region.startPos;
        init();
    } else {
        region.startPos = 0;
        region.length = _len;
        init();
    }
}

void RegionSelector::init() {
    int w = qMax(((int)log10((double)region.endPos()))*10, 70);

    comboBox = new QComboBox(this);
    comboBox->addItem(tr("Whole sequence"));
    if (selection != NULL && !selection->isEmpty()){
        comboBox->addItem(tr("Selected"));
        comboBox->setCurrentIndex(1);
    }
    comboBox->addItem(tr("Custom"));
    connect(comboBox,SIGNAL(currentIndexChanged(int)),SLOT(sl_onComboBoxIndexChanged(int)));

    startEdit = new RegionLineEdit(this,tr("Set minimum"), 1);
    startEdit->setValidator(new QIntValidator(1, maxLen, startEdit));
    startEdit->setMinimumWidth(w);

    startEdit->setAlignment(Qt::AlignRight);
    startEdit->setText(QString::number(region.startPos + 1));
    connect(startEdit, SIGNAL(editingFinished()), SLOT(sl_onRegionChanged()));
    connect(startEdit, SIGNAL(textEdited(QString)), SLOT(sl_onValueEdited()));

    endEdit = new RegionLineEdit(this,tr("Set maximum"), maxLen);
    endEdit->setValidator(new QIntValidator(1, maxLen, startEdit));
    endEdit->setMinimumWidth(w);

    endEdit->setAlignment(Qt::AlignRight);
    endEdit->setText(QString::number(region.endPos()));

    connect(endEdit, SIGNAL(editingFinished()), SLOT(sl_onRegionChanged()));
    connect(endEdit, SIGNAL(textEdited(QString)), SLOT(sl_onValueEdited()));

    if(isVertical){
        QGroupBox* gb=new QGroupBox(this);
        gb->setTitle(tr("Region"));
        QGridLayout* l = new QGridLayout(gb);
        gb->setLayout(l);
        l->addWidget(comboBox,0,0,1,3);
        l->addWidget(startEdit, 1, 0);
        l->addWidget(new QLabel(tr("-"), gb), 1, 1);
        l->addWidget(endEdit, 1, 2);
        l->setRowStretch(2,1);
        QVBoxLayout* rootLayout = new QVBoxLayout(this);
        rootLayout->setMargin(0);
        setLayout(rootLayout);
        rootLayout->addWidget(gb);
    }else{
        QHBoxLayout* l = new QHBoxLayout(this);
        l->setMargin(0);

        setLayout(l);

        QLabel* rangeLabel = new QLabel(tr("Region"), this);
        rangeLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);


        l->addWidget(rangeLabel);
        l->addWidget(comboBox);
        l->addWidget(startEdit);
        l->addWidget(new QLabel(tr("-"), this));
        l->addWidget(endEdit);

    }

    startEdit->setObjectName("start_edit_line");
    endEdit->setObjectName("end_edit_line");
    this->setObjectName("range_selector");
}

RegionSelector::~RegionSelector(){
}

void RegionSelector::sl_onComboBoxIndexChanged(int index) {
    if(index == 0) {//whole sequence
        startEdit->setText("1");
        endEdit->setText(QString::number(maxLen));
    }else if(index == 1 && selection != NULL && !selection->isEmpty()){//selection region
        qint64 start = selection->getSelectedRegions().first().startPos + 1;
        qint64 end = selection->getSelectedRegions().first().endPos();
        startEdit->setText(QString::number(start));
        endEdit->setText(QString::number(end));
    }
    sl_onValueEdited();
    sl_onRegionChanged();
}

void RegionSelector::sl_onValueEdited(){
    if(startEdit->text().isEmpty() || endEdit->text().isEmpty()){
        if(startEdit->text().isEmpty()){
            QPalette p = startEdit->palette();
            p.setColor(QPalette::Base, QColor(255,200,200));//pink color
            startEdit->setPalette(p);
        }
        if(endEdit->text().isEmpty()){
            QPalette p = endEdit->palette();
            p.setColor(QPalette::Base, QColor(255,200,200));//pink color
            endEdit->setPalette(p);
        }
        return;
    }
    region=getRegion();
    if(region.startPos == 0 && region.length == maxLen){
        comboBox->setCurrentIndex(0);
    }else if (selection != NULL && !selection->isEmpty()){//selection region
        qint64 start = selection->getSelectedRegions().first().startPos;
        qint64 end = selection->getSelectedRegions().first().endPos();
        if(start == region.startPos && end == region.endPos()){
            comboBox->setCurrentIndex(1);
        }else{
            comboBox->setCurrentIndex(2);
        }
    } else {
        comboBox->setCurrentIndex(1);
    }
    if(region.isEmpty()){
        QPalette p = startEdit->palette();
        p.setColor(QPalette::Base, QColor(255,200,200));//pink color
        startEdit->setPalette(p);
        p = endEdit->palette();
        p.setColor(QPalette::Base, QColor(255,200,200));//pink color
        endEdit->setPalette(p);
    }else{
        QPalette p = startEdit->palette();
        p.setColor(QPalette::Base, QColor(255,255,255));//white color
        startEdit->setPalette(p);
        p = endEdit->palette();
        p.setColor(QPalette::Base, QColor(255,255,255));//white color
        endEdit->setPalette(p);
    }
}

void RegionSelector::sl_onRegionChanged() {
    bool ok = false;
    int v1 = startEdit->text().toInt(&ok);
    if (!ok || v1 < 1 || v1 > maxLen) {
        return;
    }
    int v2 = endEdit->text().toInt(&ok);
    if (!ok || v2 < v1 || v2 > maxLen) {
        return;
    }

    U2Region r(v1 - 1, v2 - (v1 - 1));
    emit si_regionChanged(r);
}

U2Region RegionSelector::getRegion(bool *_ok) const{
    bool ok = false;
    qint64 v1 = startEdit->text().toLongLong(&ok) - 1;
    if (!ok || v1 < 0){
        if (_ok != NULL) *_ok = false;
        return U2Region();
    }
    int v2 = endEdit->text().toLongLong(&ok);
    if (!ok || v2 <= 0 || v2 > maxLen){
        if (_ok != NULL) *_ok = false;
        return U2Region();
    }
    if (v1 > v2 ) { // start > end
        if (_ok != NULL) *_ok = false;
        return U2Region();
    }
    if (_ok != NULL) *_ok = true;
    return U2Region(v1, v2 - v1);
}

void RegionSelector::setRegion(const U2Region& value){
    if (value.startPos < 0 || value.length > maxLen){
        return;
    }
    if (value == region) {
        return;
    }
    startEdit->setText(QString::number(value.startPos+1));
    endEdit->setText(QString::number(value.endPos()));
    emit si_regionChanged(value);

}

void RegionSelector::reset(){
    if (selection != NULL){
        comboBox->setCurrentIndex(1);
    } else {
        comboBox->setCurrentIndex(0);
    }
}

void RegionSelector::showErrorMessage(){
    QMessageBox msgBox;//todo more informative message
    msgBox.setWindowTitle(L10N::errorTitle());
    msgBox.setText(tr("Invalid sequence region!"));
    msgBox.setStandardButtons(QMessageBox::Ok);
    //set focus to error field
    bool ok = false;
    qint64 v1 = startEdit->text().toLongLong(&ok) - 1;
    if (!ok || v1 < 0){
        msgBox.setInformativeText(tr("Invalid Start position of region"));
        msgBox.exec();
        startEdit->setFocus();
        return;
    }
    int v2 = endEdit->text().toLongLong(&ok);
    if(!ok || v2 <= 0 || v2 > maxLen){
        msgBox.setInformativeText(tr("Invalid End position of region"));
        msgBox.exec();
        endEdit->setFocus();
        return;
    }
    if ( v1 > v2 ){ // start > end
        msgBox.setInformativeText(tr("Start position is greater than End position"));
        msgBox.exec();
        startEdit->setFocus();
        return;
    }
    msgBox.exec();//all other errors
}

void RegionSelector::setWholeRegionSelected()
{
    comboBox->setCurrentIndex(0);
}

///////////////////////////////////////
//RegionLineEdit
//only for empty field highlight
void RegionLineEdit::focusOutEvent ( QFocusEvent * event) {
    bool ok = false;
    int value = text().toInt(&ok);
    Q_UNUSED(value);
    if (!ok) {
        QPalette p = palette();
        p.setColor(QPalette::Base, QColor(255,200,200));//pink color
        setPalette(p);
    }
    QLineEdit::focusOutEvent(event);
}
void RegionLineEdit::contextMenuEvent(QContextMenuEvent *event){
        QMenu *menu = createStandardContextMenu();
        QAction* setDefaultValue=new QAction(actionName,this);
        connect(setDefaultValue,SIGNAL(triggered()),this,SLOT(sl_onSetMinMaxValue()));
        menu->insertSeparator(menu->actions().first());
        menu->insertAction(menu->actions().first(),setDefaultValue);
        menu->exec(event->globalPos());
        delete menu;
}
void RegionLineEdit::sl_onSetMinMaxValue(){
    setText(QString::number(defaultValue));
    emit textEdited(QString::number(defaultValue));
}
} //namespace
