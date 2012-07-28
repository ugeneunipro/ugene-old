#include "ColorSchemaDialogController.h"

#include <QtGui/QPainter>
#include <QtGui/QColorDialog>

namespace U2{

ColorSchemaDialogController::ColorSchemaDialogController(QMap<char, QColor>& colors):QDialog(), newColors(colors){}

int ColorSchemaDialogController::adjustAlphabetColors(){    
    setupUi(this);
    alphabetColorsView = new QPixmap(alphabetColorsFrame->size());

    update();

    return exec();
}

ColorSchemaDialogController::~ColorSchemaDialogController(){
    delete alphabetColorsView;
}

void ColorSchemaDialogController::paintEvent(QPaintEvent*){
    const int columns = 6;    

    const int rect_width = static_cast<double> (alphabetColorsFrame->size().width()) / columns ;
    if(rect_width == 0){return;}

    const int rows = (newColors.size() / columns) + ((newColors.size() % columns) ?  1 : 0);
    const int rect_height = static_cast<double> (alphabetColorsFrame->size().height() ) / rows;    
    if(rect_height == 0){return;}
    
    delete alphabetColorsView; 
    alphabetColorsView = new QPixmap(alphabetColorsFrame->size());

    QPainter painter(alphabetColorsView);
    QFont font;
    font.setFamily("Verdana");
    font.setPointSize(qMin(rect_width, rect_height) / 2);
    painter.setFont(font);

    QMapIterator<char, QColor> it(newColors);

    int i = 0;
    int j = 0;
    for(int i = 0; i < rows; ++i){
        painter.drawLine(0, i * rect_height, alphabetColorsView->size().width(), i * rect_height);
        for(int j = 0; j < columns; ++j){
            if(!it.hasNext()){break;}

            it.next();
            QRect nextRect(j * rect_width, i * rect_height + 1, rect_width, rect_height - 1);
            painter.fillRect(nextRect, it.value());
            painter.drawLine(j * rect_width, i * rect_height, j * rect_width, (i + 1) * rect_height);
            painter.drawLine((j + 1) * rect_width, i * rect_height, (j + 1) * rect_width, (i + 1) * rect_height);
            painter.drawText(nextRect, Qt::AlignCenter, QString(it.key()));

            charsPlacement[it.key()] = nextRect;
        }   
        if(!it.hasNext()){break;}
    }

    QPainter dialogPainter(this);
    dialogPainter.drawPixmap(alphabetColorsFrame->geometry().x(), alphabetColorsFrame->geometry().y(), *alphabetColorsView);
}

void ColorSchemaDialogController::mouseReleaseEvent(QMouseEvent * event){
    QMapIterator<char, QRect> it(charsPlacement);

    while(it.hasNext()){
        it.next();
        if(it.value().contains(event->pos().x() - alphabetColorsFrame->geometry().x(), event->pos().y() - alphabetColorsFrame->geometry().y())){
            QColorDialog d;
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