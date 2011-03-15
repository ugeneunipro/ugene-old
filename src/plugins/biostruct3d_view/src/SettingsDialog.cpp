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


#include "SettingsDialog.h"
#include <U2Core/Log.h>

namespace U2 {

static Logger log("test");

BioStruct3DSettingsDialog::BioStruct3DSettingsDialog()
{
    setupUi(this);

    glWidget = NULL;
    initColorSchemes();
}

void BioStruct3DSettingsDialog::setWidget(BioStruct3DGLWidget *glWidget)
{
    this->glWidget = glWidget;

    state = glWidget->getState();
}

void BioStruct3DSettingsDialog::initColorSchemes()
{
    glassesColorSchemes.insert(0, GlassesColorScheme(QString("Custom"), QColor(0, 0, 0), QColor(0, 0, 0)));

    glassesColorSchemes.append(GlassesColorScheme(QString("Red - Blue"), QColor(255, 0, 0), QColor(0, 0, 255)));
    glassesColorSchemes.append(GlassesColorScheme(QString("Cyan - Red"), QColor(0, 255, 255), QColor(255, 0, 0)));
    glassesColorSchemes.append(GlassesColorScheme(QString("Red - Dark Cyan"), QColor(255, 0, 0), QColor(0, 139, 139)));
    glassesColorSchemes.append(GlassesColorScheme(QString("Red - Green"), QColor(255, 0, 0), QColor(0, 150, 0)));
    glassesColorSchemes.append(GlassesColorScheme(QString("Magneta - Green"), QColor(255, 0, 150), QColor(0, 150, 0)));

    foreach (const GlassesColorScheme &scheme, glassesColorSchemes)
        BioStruct3DSettingsDialog::glassesColorSchemeComboBox->addItem(scheme.name);
}


void BioStruct3DSettingsDialog::sl_setBackgroundColor()
{
    backgroundColor=QColorDialog::getColor(backgroundColor,this);

    state[glWidget->BACKGROUND_COLOR_NAME] = QVariant::fromValue(backgroundColor);
    glWidget->setState(state);
}

void BioStruct3DSettingsDialog::sl_setSelectionColor()
{
    selectionColor=QColorDialog::getColor(selectionColor,this);

    state[glWidget->SELECTION_COLOR_NAME] = QVariant::fromValue(selectionColor);
    glWidget->setState(state);
}

void BioStruct3DSettingsDialog::sl_setRenderDetailLevel()
{
    renderDetailLevel=renderDetailLevelSlider->sliderPosition();

    state[glWidget->RENDER_DETAIL_LEVEL_NAME] = QVariant::fromValue(renderDetailLevel);
    glWidget->setState(state);
}

void BioStruct3DSettingsDialog::sl_setShadingLevel()
{
    shadingLevel = shadingSlider->sliderPosition();
    state[glWidget->SHADING_LEVEL_NAME] = QVariant::fromValue(shadingLevel);
    glWidget->setState(state);
}

void BioStruct3DSettingsDialog::sl_setAnaglyph()
{
    anaglyph = anaglyphViewGroupBox->isChecked();

    state[glWidget->ANAGLYPH_NAME] = QVariant::fromValue(anaglyph);
    glWidget->setState(state);
}

void BioStruct3DSettingsDialog::sl_setEyesShift()
{
    eyesShift = eyesShiftSlider->sliderPosition();

    state[glWidget->EYES_SHIFT_NAME] = QVariant::fromValue(eyesShift);
    glWidget->setState(state);
}

void BioStruct3DSettingsDialog::sl_setGlassesColorScheme()
{
    int itemNum = glassesColorSchemeComboBox->currentIndex();

    setGlassesColorScheme(itemNum);
}

void BioStruct3DSettingsDialog::sl_setLeftEyeColor()
{
    QColor changed(QColorDialog::getColor(leftEyeColor,this));

    if (changed.red()!=0 || changed.green()!=0 || changed.blue()!=0)
        setGlassesColorScheme(changed, rightEyeColor);

    state[glWidget->LEFT_EYE_COLOR_NAME] = QVariant::fromValue(changed);
    glWidget->setState(state);
}

void BioStruct3DSettingsDialog::sl_setRightEyeColor()
{
    QColor changed(QColorDialog::getColor(rightEyeColor,this));

    if (changed.red()!=0 || changed.green()!=0 || changed.blue()!=0)
        setGlassesColorScheme(leftEyeColor, changed);
}

void BioStruct3DSettingsDialog::sl_swapColors()
{
    QColor right(rightEyeColor);
    QColor left(leftEyeColor);

    setGlassesColorScheme(right, left);
}

QColor BioStruct3DSettingsDialog::getBackgroundColor()const
{
    return backgroundColor;
}

QColor BioStruct3DSettingsDialog::getSelectionColor()const
{
    return selectionColor;
}

int BioStruct3DSettingsDialog::getRenderDetailLevel()const
{
    return renderDetailLevel;
}

bool BioStruct3DSettingsDialog::getAnaglyph()const
{
    return anaglyph;
}

int BioStruct3DSettingsDialog::getEyesShift()const
{
    return eyesShift;
}

QColor BioStruct3DSettingsDialog::getLeftEyeColor()const
{
    return leftEyeColor;
}

QColor BioStruct3DSettingsDialog::getRightEyeColor()const
{
    return rightEyeColor;
}

void BioStruct3DSettingsDialog::setGlassesColorScheme(QColor &leftEyeColor, QColor &rightEyeColor)
{
    setLeftEyeColor(leftEyeColor);
    setRightEyeColor(rightEyeColor);

    int currentNumber=0;
    foreach(const GlassesColorScheme& scheme, glassesColorSchemes)
    {
        if (scheme.leftEyeColor==leftEyeColor && scheme.rightEyeColor==rightEyeColor)
        {
            glassesColorSchemeComboBox->setCurrentIndex(currentNumber);
            return;
        }
        currentNumber++;
    }

    glassesColorSchemeComboBox->setCurrentIndex(0);
}

void BioStruct3DSettingsDialog::setGlassesColorScheme(int num)
{
    if (num<=0 || num>glassesColorSchemes.size())
        return;

    GlassesColorScheme need(glassesColorSchemes.at(num));

    setLeftEyeColor(need.leftEyeColor);
    setRightEyeColor(need.rightEyeColor);
}

void BioStruct3DSettingsDialog::setEyesShift(int eyesShift)
{
    this->eyesShift=eyesShift;
    this->eyesShiftSlider->setSliderPosition(eyesShift);
}

void BioStruct3DSettingsDialog::setBackgroundColor(QColor color)
{
    this->backgroundColor=color;
}

void BioStruct3DSettingsDialog::setSelectionColor(QColor color)
{
    this->selectionColor=color;
}

void BioStruct3DSettingsDialog::setRenderDetailLevel(int renderDetailLevel)
{
    this->renderDetailLevel = renderDetailLevel;

    renderDetailLevelSlider->setSliderPosition(renderDetailLevel);
}

void BioStruct3DSettingsDialog::setAnaglyph(bool anaglyph)
{
    this->anaglyph = anaglyph;

    anaglyphViewGroupBox->setChecked(anaglyph);
}

void BioStruct3DSettingsDialog::setAnaglyphAvailability(bool anaglyphAvailability) {

    anaglyphViewGroupBox->setEnabled(anaglyphAvailability);

    if (!anaglyphAvailability) {    
        QString anaglyphTitile = anaglyphViewGroupBox->title();
        anaglyphViewGroupBox->setTitle(anaglyphTitile + QString(" ") + QString(tr("(not supported by your videocard)")));
    }
}

static const QString COLOR_STYLE("QPushButton { background-color : %1;}");// color : %2;

void BioStruct3DSettingsDialog::setLeftEyeColor(QColor leftEyecolor)
{
    leftEyeColor = leftEyecolor;
    leftEyeColorChangeButton->setStyleSheet(COLOR_STYLE.arg(leftEyeColor.name()));

    state[glWidget->LEFT_EYE_COLOR_NAME] = QVariant::fromValue(leftEyeColor);
    glWidget->setState(state);
}

void BioStruct3DSettingsDialog::setRightEyeColor(QColor rightEyecolor)
{
    rightEyeColor = rightEyecolor;
    rightEyeColorChangeButton->setStyleSheet(COLOR_STYLE.arg(rightEyeColor.name()));

    state[glWidget->RIGHT_EYE_COLOR_NAME] = QVariant::fromValue(rightEyeColor);
    glWidget->setState(state);
}

int BioStruct3DSettingsDialog::getShadingLevel() const {
    return shadingLevel;
}

void BioStruct3DSettingsDialog::setShadingLevel(int shading) {
    shadingLevel = shading;
    shadingSlider->setSliderPosition(shading);
}

} // namespace
