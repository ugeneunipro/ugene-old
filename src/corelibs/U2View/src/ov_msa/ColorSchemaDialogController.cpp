#include "ColorSchemaDialogController.h"

#include <QtGui/QPainter>
#include <QtGui/QColorDialog>

namespace U2{

ColorSchemaDialogController::ColorSchemaDialogController(QMap<char, QColor>& colors):QDialog(), newColors(colors), storedColors(colors){}

int ColorSchemaDialogController::adjustAlphabetColors(){    
    setupUi(this);
    alphabetColorsView = new QPixmap(alphabetColorsFrame->size());
    connect(clearButton, SIGNAL(clicked()), SLOT(sl_onClear()));
    connect(restoreButton, SIGNAL(clicked()), SLOT(sl_onRestore()));

    update();

    return exec();
}

ColorSchemaDialogController::~ColorSchemaDialogController(){
    delete alphabetColorsView;
}

void ColorSchemaDialogController::paintEvent(QPaintEvent*){
    QPainter dialogPainter(this);
    const int columns = 6;    

    const int rect_width = static_cast<double> (alphabetColorsFrame->size().width()) / columns ;
    if(rect_width == 0){return;}
    int rect_width_rest = alphabetColorsFrame->size().width() % columns;

    const int rows = (newColors.size() / columns) + ((newColors.size() % columns) ?  1 : 0);
    const int rect_height = static_cast<double> (alphabetColorsFrame->size().height() ) / rows;    
    if(rect_height == 0){return;}
    int rect_height_rest = alphabetColorsFrame->size().height() % rows;
    
    delete alphabetColorsView; 
    alphabetColorsView = new QPixmap(alphabetColorsFrame->size());

    QPainter painter(alphabetColorsView);
    QFont font;
    font.setFamily("Verdana");
    font.setPointSize(qMin(rect_width, rect_height) / 2);
    painter.setFont(font);

    QMapIterator<char, QColor> it(newColors);

    int hLineY = 0;
    for(int i = 0; i < rows; ++i){
        int rh = rect_height;
        rect_width_rest = alphabetColorsFrame->size().width() % columns;
        if(rect_height_rest > 0){
            rh++;
            rect_height_rest--;
        }
        painter.drawLine(0, hLineY, alphabetColorsView->size().width(), hLineY);
        int vLineX = 0;
        for(int j = 0; j < columns; ++j){
            if(!it.hasNext()){break;}

            it.next();
            int rw = rect_width;
            if(rect_width_rest > 0){
                rw++;
                rect_width_rest--;
            }
            QRect nextRect(vLineX, hLineY + 1, rw, rh - 1);
            painter.fillRect(nextRect, it.value());
            painter.drawText(nextRect, Qt::AlignCenter, QString(it.key()));
            painter.drawLine(vLineX, hLineY, vLineX, hLineY + rh);
            painter.drawLine(vLineX + rw, hLineY, vLineX + rw, hLineY + rh);
            vLineX += rw;
            charsPlacement[it.key()] = nextRect;
        }
        hLineY += rh;
        if(!it.hasNext()) {
            painter.fillRect(vLineX + 1, hLineY - rh + 1, alphabetColorsView->size().width() - vLineX - 1, rh - 1, dialogPainter.background());
            break;
        }
    }
    painter.drawLine(0, alphabetColorsView->size().height(), alphabetColorsView->size().width(), alphabetColorsView->size().height());
    painter.drawLine(alphabetColorsView->size().width(), 0, alphabetColorsView->size().width(), alphabetColorsView->size().height());


    dialogPainter.drawPixmap(alphabetColorsFrame->geometry().x(), alphabetColorsFrame->geometry().y(), *alphabetColorsView);
}

void ColorSchemaDialogController::sl_onClear(){
    storedColors = newColors;

    QMapIterator<char, QColor> it(newColors);
    while(it.hasNext()){
        it.next();
        newColors[it.key()] = QColor(Qt::white);
    }

    update();
}

void ColorSchemaDialogController::sl_onRestore(){
    newColors = storedColors;
    update();
}

void ColorSchemaDialogController::mouseReleaseEvent(QMouseEvent * event){
    QMapIterator<char, QRect> it(charsPlacement);

    while(it.hasNext()){
        it.next();
        if(it.value().contains(event->pos().x() - alphabetColorsFrame->geometry().x(), event->pos().y() - alphabetColorsFrame->geometry().y())){
            QColorDialog d;
#ifdef Q_OS_MAC
            // A workaround because of UGENE-2263
            // Another way should be found.
            // I suppose, that it is bug in the Qt libraries (Qt-4.8.5 for mac)
            d.setOption(QColorDialog::DontUseNativeDialog);
#endif
            int res = d.exec();
            if(res == QDialog::Accepted){
                newColors[it.key()] = d.selectedColor();                
            }
            break;
        }
    }    

    update();
}

} // namespace
