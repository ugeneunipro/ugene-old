#ifndef _U2_SETTINGSDIALOG_H_
#define _U2_SETTINGSDIALOG_H_

#include "BioStruct3DGLWidget.h"
#include <QtGui/QColorDialog>
#include "ui_SettingsDialog.h"

namespace U2 {


class GlassesColorScheme
{
public:
    QColor leftEyeColor;
    QColor rightEyeColor;

    QString name;

    GlassesColorScheme()
    {
        name="";
        leftEyeColor=QColor(0, 0, 0);
        rightEyeColor = leftEyeColor;
    }

    GlassesColorScheme(QString name, QColor leftEyeColor, QColor rightEyeColor)
    {
        this->name = name;
        this->leftEyeColor = leftEyeColor;
        this->rightEyeColor = rightEyeColor;
    }
};


class BioStruct3DSettingsDialog : public QDialog, private Ui::SettingsDialog
{
    Q_OBJECT

public:
    BioStruct3DSettingsDialog();

private slots:
    void sl_setBackgroundColor();
    void sl_setSelectionColor();
    void sl_setShadingLevel();
    void sl_setRenderDetailLevel();
    void sl_setAnaglyph();

    void sl_setEyesShift();

    void sl_setGlassesColorScheme();
    void sl_setLeftEyeColor();
    void sl_setRightEyeColor();
    void sl_swapColors();


public:
    QColor getBackgroundColor()const;
    QColor getSelectionColor()const;

    QColor getLeftEyeColor()const;
    QColor getRightEyeColor()const;

    int getRenderDetailLevel()const;
    int getShadingLevel()const;
    bool getAnaglyph()const;
    int getEyesShift()const;

    void setBackgroundColor(QColor color);
    void setSelectionColor(QColor color);

    void setLeftEyeColor(QColor leftEyecolor);
    void setRightEyeColor(QColor rightEyecolor);

    void setGlassesColorScheme(QColor &leftEyeColor, QColor &rightEyeColor);
    void setGlassesColorScheme(int num);

    void setRenderDetailLevel(int renderDetailLevel);
    void setShadingLevel(int shading);

    void setAnaglyph(bool anaglyph);
    void setAnaglyphAvailability(bool anaglyph);
    void setEyesShift(int eyesShift);

    void setWidget(BioStruct3DGLWidget *glWidget);

private:
    QVariantMap state;

    BioStruct3DGLWidget *glWidget;
    QList<GlassesColorScheme> glassesColorSchemes;

    void initColorSchemes();

    QColor backgroundColor;
    QColor selectionColor;

    QColor leftEyeColor;
    QColor rightEyeColor;

    int renderDetailLevel;
    int shadingLevel;
    bool anaglyph;
    int eyesShift;
};

} // namespace

#endif // _U2_SETTINGSDIALOG_H_
