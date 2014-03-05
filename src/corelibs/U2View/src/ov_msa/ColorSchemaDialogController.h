#ifndef COLOR_SCHEMA_DIALOG_CONTROLLER_H_
#define COLOR_SCHEMA_DIALOG_CONTROLLER_H_

#include "ui/ui_ColorSchemaDialog.h"

#include <QtCore/QMap>

#include <QtGui/QPixmap>
#include <QtGui/QMouseEvent>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif

namespace U2{

class ColorSchemaDialogController : public QDialog, public Ui_ColorSchemaDialog{
    Q_OBJECT
public:
    ColorSchemaDialogController(QMap<char, QColor>& colors);
    ~ColorSchemaDialogController();
    int adjustAlphabetColors(); 
protected:
    void mouseReleaseEvent(QMouseEvent *);
    void paintEvent(QPaintEvent*);
private slots:
    void sl_onClear();
    void sl_onRestore();
private:
    QPixmap* alphabetColorsView;
    QMap<char, QColor>& newColors;
    QMap<char, QColor> storedColors;
    QMap<char, QRect> charsPlacement;

};

}

#endif
