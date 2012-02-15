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

#include "ExpertDiscoveryGraphs.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/TextUtils.h>

#include <U2Gui/GraphUtils.h>

namespace U2{

    ExpertDiscoveryScoreGraphAlgorithm::ExpertDiscoveryScoreGraphAlgorithm(ExpertDiscoveryData& data, int _edSeqNumber, SequenceType sType)
:edData(data)
,edSeqNumber(_edSeqNumber)
,edSeqType(sType)
{
    recData = edData.getRecognitionData(edSeqNumber, edSeqType);
}


ExpertDiscoveryScoreGraphAlgorithm::~ExpertDiscoveryScoreGraphAlgorithm()
{
}

/**
 * Calculates data for a score graph
 *
 * @param result Points of the graph
 * @param sequenceObject The sequence used to draw the graph
 * @param region The region of the sequence to use
 * @param windowData Current parameters of the graph (window, step, etc.)
 */
void ExpertDiscoveryScoreGraphAlgorithm::calculate(
    QVector<float>& result,
    U2SequenceObject* sequenceObject,
    const U2Region& region,
    const GSequenceGraphWindowData* windowData)
{
    assert(windowData !=NULL);

    
    QByteArray sequence = sequenceObject->getWholeSequenceData();

    int windowSize = windowData->window;
    int windowStep = windowData->step;

    // Variables
    int windowLeft = region.startPos;
    float windowThreshold = 0;

    // Getting the number of steps
    int stepsNumber = GSequenceGraphUtils::getNumSteps(region, windowData->window, windowData->step);

    // Allocating memory for the results
    result.reserve(stepsNumber);

    // Calculating the results
    for (int i = 0; i < stepsNumber; ++i)
    {
        // Calculating the threshold in the current window
        windowThreshold = 0;
        for (int j = windowLeft; j < windowLeft + windowSize - 1; ++j)
        {
            if(j < recData.size()){
                windowThreshold+=recData[j];
            }
        }
        windowThreshold /= (windowSize - 1);


        // Returning the point on the graph
        result.append(windowThreshold);

        // Enlarging the left position to the step
        windowLeft += windowStep;
    }
}


/**
 * Name of the graph (shown to a user)
 */
static QString nameByType() {
    return ExpertDiscoveryScoreGraphFactory::tr("Score graph");

}


/**
 * Constructor of the ExpertDiscovery score graph
 */
ExpertDiscoveryScoreGraphFactory::ExpertDiscoveryScoreGraphFactory(QObject* parent, ExpertDiscoveryData& data, int _edSeqNumber, SequenceType sType)
    : GSequenceGraphFactory(nameByType(), parent)
    ,edData(data)
    ,edSeqNumber(_edSeqNumber)
    ,edSeqType(sType)
{
}


/**
 * Verification
 */
bool ExpertDiscoveryScoreGraphFactory::isEnabled(U2SequenceObject* sequenceObject) const {
    DNAAlphabet* alphabet = sequenceObject->getAlphabet();
    return alphabet->getId() == BaseDNAAlphabetIds::NUCL_DNA_DEFAULT();
}


/**
 * Initializes graph data
 */
QList<GSequenceGraphData*> ExpertDiscoveryScoreGraphFactory::createGraphs(GSequenceGraphView* view)
{
    Q_UNUSED(view);
    QList<GSequenceGraphData*> res;
    assert(isEnabled(view->getSequenceObject()));
    assert(edSeqType!=UNKNOWN_SEQUENCE);
    GSequenceGraphData* data = new GSequenceGraphData(getGraphName());
    data->ga = new ExpertDiscoveryScoreGraphAlgorithm(edData, edSeqNumber, edSeqType);
    res.append(data);
    return res;
}


/**
 * Initializes the graph drawer
 */
GSequenceGraphDrawer* ExpertDiscoveryScoreGraphFactory::getDrawer(GSequenceGraphView* view)
{
    GSequenceGraphWindowData wd(DEFAULT_WINDOW_STEP, DEFAULT_WINDOW_SIZE);
    return new GSequenceGraphDrawer(view, wd);
}



QColor ExpertDiscoveryRecognitionErrorGraphWidget::ER1COLOR = QColor(255, 0, 0);
QColor ExpertDiscoveryRecognitionErrorGraphWidget::ER2COLOR = QColor(0, 0, 255);
QColor ExpertDiscoveryRecognitionErrorGraphWidget::BOUNDCOLOR = QColor(255,0,255);

ExpertDiscoveryRecognitionErrorGraphWidget::ExpertDiscoveryRecognitionErrorGraphWidget(QWidget* parent,
const std::vector<double>& _posScore, const std::vector<double>& _negScore, const CalculateErrorTaskInfo& _calcualteSettings)
:QWidget(parent), redraw(false), posScore(_posScore), negScore(_negScore), recBound(0), calcualteSettings(_calcualteSettings){

    textOffset = 15;
    w = width() - 2*textOffset;
    h = height() - 2*textOffset;
    connect(&errorsTask, SIGNAL(si_finished()), SLOT(sl_redraw()));
    sl_calculateErrors(calcualteSettings);
}
void ExpertDiscoveryRecognitionErrorGraphWidget::draw(double curRecBound){
    recBound = curRecBound;
    redraw = true;
    update();
}

void ExpertDiscoveryRecognitionErrorGraphWidget::sl_redraw(){
    draw(recBound);
}

void ExpertDiscoveryRecognitionErrorGraphWidget::sl_calculateErrors(const CalculateErrorTaskInfo& _calcualteSettings){
    calcualteSettings = _calcualteSettings;
    errorsTask.run(new ExpertDiscoveryCalculateErrors(calcualteSettings));
}

void ExpertDiscoveryRecognitionErrorGraphWidget::paintEvent(QPaintEvent * e){
    drawAll();
    QWidget::paintEvent(e);
}

void ExpertDiscoveryRecognitionErrorGraphWidget::drawAll(){
    if(pixmap.size() != size()){
        pixmap = QPixmap(size());
        w = width() - 2*textOffset;
        h = height() - 2*textOffset;
        redraw = true;
    }
    if(redraw){
        pixmap.fill(Qt::transparent);
        QPainter p(&pixmap);
        
        if(errorsTask.isFinished()){
            QPixmap graphPixmap = QPixmap(w, h);
            graphPixmap.fill(Qt::white);
            QPainter grP(&graphPixmap);
            drawGraph(grP);
            p.drawPixmap(textOffset, textOffset, graphPixmap);
            drawRuler(p);
            redraw = false;
        }
    }
    QPainter p(this);
    p.drawPixmap(0, 0, pixmap);
}

void ExpertDiscoveryRecognitionErrorGraphWidget::drawGraph(QPainter& p){

    double step = calcualteSettings.scoreStep;

    assert(step!=0);

    int regLen = calcualteSettings.scoreReg.length;
    int stepsNum = regLen/step;

    int offset = 2;
    int scoreWidthPoint = 0;
    const ErrorsInfo& errorsInfo = errorsTask.getResult();
    QPainterPath erFirstTypePath;
    QPainterPath erSecondTypePath;
    if (stepsNum  < w){
        emit si_showWarning(false);

        double pixelStep = double(w)/stepsNum;
        double ratioY = double(errorsInfo.maxErrorVal)/(h);
        
        int hPixels = 0;
        if(stepsNum!=0){
            hPixels = qint64(double(errorsInfo.errorFirstType[0])/ratioY +0.5);
            erFirstTypePath.moveTo(QPointF(0, h - hPixels));

            hPixels = qint64(double(errorsInfo.errorSecondType[0])/ratioY +0.5);
            erSecondTypePath.moveTo(QPointF(0, h - hPixels));
        }
        for(int i = 1; i < stepsNum; i++){
            hPixels = qint64(double(errorsInfo.errorFirstType[i])/ratioY +0.5);
            erFirstTypePath.lineTo(QPointF(i*pixelStep, h - hPixels));

            hPixels = qint64(double(errorsInfo.errorSecondType[i])/ratioY +0.5);
            erSecondTypePath.lineTo(QPointF(i*pixelStep, h - hPixels));
        }
        scoreWidthPoint = int(((recBound-calcualteSettings.scoreReg.startPos)/calcualteSettings.scoreReg.length)*w+0.5);
    }else{                                                   //average value per pixel used
        emit si_showWarning(true);
        int windowSize = int((double(stepsNum)/w) +0.5);
        double ratioY = double(errorsInfo.maxErrorVal)/(h);

        int hPixels = 0;
        if(stepsNum!=0){
            hPixels = qint64(double(errorsInfo.errorFirstType[0])/ratioY +0.5);
            erFirstTypePath.moveTo(QPointF(0, h - hPixels));

            hPixels = qint64(double(errorsInfo.errorSecondType[0])/ratioY +0.5);
            erSecondTypePath.moveTo(QPointF(0, h - hPixels));
        }
        for(int i = 0; i < stepsNum; i+=windowSize){
            double val = 0;
            for (int j = 0; (j < windowSize) && (i+j<stepsNum); j++){
                val+= errorsInfo.errorFirstType[i+j];
            }
            hPixels = qint64(val/(ratioY*windowSize) + 0.5);
            erFirstTypePath.lineTo(QPointF(i, h - hPixels));

            val = 0;
            for (int j = 0; (j < windowSize) && (i+j<stepsNum); j++){
                val+= errorsInfo.errorSecondType[i+j];
            }
            hPixels = qint64(val/(ratioY*windowSize) + 0.5);
            erSecondTypePath.lineTo(QPointF(i, h - hPixels));
            if((calcualteSettings.scoreReg.length/double(stepsNum)*i + calcualteSettings.scoreReg.startPos <= recBound)){
                scoreWidthPoint = i;
            }
        }
    }
    QPen linePen(ER1COLOR);
    linePen.setWidth(3);

    p.setPen(linePen);
    p.drawPath(erFirstTypePath);

    linePen.setColor(ER2COLOR);
    p.setPen(linePen);
    p.drawPath(erSecondTypePath);

    if(recBound >= calcualteSettings.scoreReg.startPos && recBound <= calcualteSettings.scoreReg.endPos()){
        linePen.setColor(BOUNDCOLOR);
        p.setPen(linePen);
        p.drawLine(QPoint(scoreWidthPoint, 0), QPoint(scoreWidthPoint, h ));
    }
}

void ExpertDiscoveryRecognitionErrorGraphWidget::drawRuler(QPainter& p){
    GraphUtils::RulerConfig rConf;

    rConf.notchSize = 2;
    rConf.textPosition = GraphUtils::RIGHT;
    QFont rulerFont;

    rulerFont.setFamily("Arial");
    rulerFont.setPointSize(8);

    int startX = calcualteSettings.scoreReg.startPos;
    int endX = calcualteSettings.scoreReg.length;

    GraphUtils::drawRuler(p, QPoint(textOffset, textOffset + h), w, startX, endX, rulerFont, rConf);
    rConf.textPosition = GraphUtils::LEFT;
    rConf.direction = GraphUtils::BTT;
    GraphUtils::drawRuler(p, QPoint(textOffset, textOffset), h, 0, 1, rulerFont, rConf);
}

}//namespace