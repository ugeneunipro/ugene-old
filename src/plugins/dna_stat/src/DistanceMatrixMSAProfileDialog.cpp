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

#include "DistanceMatrixMSAProfileDialog.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/TextUtils.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/MAlignmentObject.h>

#include <U2Gui/LastUsedDirHelper.h>

#include <U2View/MSAEditor.h>
#include <U2View/WebWindow.h>

#include <U2Algorithm/MSADistanceAlgorithm.h>
#include <U2Algorithm/MSADistanceAlgorithmRegistry.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include <QDateTime>

namespace U2 {

DistanceMatrixMSAProfileDialog::DistanceMatrixMSAProfileDialog(QWidget* p, MSAEditor* _c) : QDialog(p), ctx(_c) {
    setupUi(this);

    QStringList algo = AppContext::getMSADistanceAlgorithmRegistry()->getAlgorithmIds();
    algoCombo->addItems(algo);

    MAlignmentObject* msaObj = ctx->getMSAObject();
    if (msaObj != NULL) {
        QVector<U2Region> unitedRows;
        MAlignment ma = msaObj->getMAlignment();
        ma.sortRowsBySimilarity(unitedRows);
        if(unitedRows.size() < 2)
            groupStatisticsCheck->setEnabled(false);
    }

    connect(fileButton, SIGNAL(clicked()), SLOT(sl_selectFile()));
    connect(htmlRB, SIGNAL(toggled(bool)), SLOT(sl_formatChanged(bool)));
    connect(csvRB, SIGNAL(toggled(bool)), SLOT(sl_formatChanged(bool)));
}

void DistanceMatrixMSAProfileDialog::sl_selectFile() {
    LastUsedDirHelper h("plugin_dna_stat");
    QString filter;
    if (csvRB->isChecked()) {
        filter = tr("CSV files") + " (*.csv)";
    } else {
        filter = tr("HTML files") + " (*.html)";
    }
    h.url = QFileDialog::getSaveFileName(this, tr("Select file to save report to.."), h.dir, filter);
    if (h.url.isEmpty()) {
        return;
    }
    fileEdit->setText(h.url);
}

void DistanceMatrixMSAProfileDialog::sl_formatChanged(bool) {
    QString t = fileEdit->text();
    if (t.isEmpty()) {
        return;
    }
    QString ext = ".html";
    if (csvRB->isChecked()) {
        ext = ".csv";
    }
    if (t.endsWith(ext)) {
        return;
    }
    QFileInfo fi(t);
    QString dir = fi.absoluteDir().absolutePath();
    if (!(dir.endsWith('/') || dir.endsWith('\\'))) {
        dir.append('/');
    }
    fileEdit->setText(dir + fi.baseName() + ext);
}

void DistanceMatrixMSAProfileDialog::accept() {
    DistanceMatrixMSAProfileTaskSettings s;
    MAlignmentObject* msaObj = ctx->getMSAObject();
    if (msaObj == NULL) {
        return;
    }
    s.profileName = msaObj->getGObjectName();
    s.profileURL = msaObj->getDocument()->getURLString();
    s.usePercents = percentsRB->isChecked();
    s.algoName = algoCombo->currentText();
    s.ma = msaObj->getMAlignment();
    s.excludeGaps = checkBox->isChecked();
    s.showGroupStatistic = groupStatisticsCheck->isChecked();
    s.ctx = ctx;

    if (saveBox->isChecked()) {
        s.outURL = fileEdit->text();
        if (s.outURL.isEmpty()) {
            QMessageBox::critical(this, tr("Error"), tr("File URL is empty"));
            return;
        }
        s.outFormat = csvRB->isChecked() ? DistanceMatrixMSAProfileOutputFormat_CSV : DistanceMatrixMSAProfileOutputFormat_HTML;
    }
    AppContext::getTaskScheduler()->registerTopLevelTask(new DistanceMatrixMSAProfileTask(s));
    QDialog::accept();
}


//////////////////////////////////////////////////////////////////////////
// task
DistanceMatrixMSAProfileTask::DistanceMatrixMSAProfileTask(const DistanceMatrixMSAProfileTaskSettings& _s) 
: Task(tr("Generate distance matrix"), TaskFlag_NoRun), s(_s)
{
    setVerboseLogMode(true);
}

void DistanceMatrixMSAProfileTask::prepare() {
    MSADistanceAlgorithmFactory* factory = AppContext::getMSADistanceAlgorithmRegistry()->getAlgorithmFactory(s.algoName);
    if(s.excludeGaps){
        factory->setFlag(DistanceAlgorithmFlag_ExcludeGaps);
    }else{
        factory->resetFlag(DistanceAlgorithmFlag_ExcludeGaps);
    }
    MSADistanceAlgorithm* algo = factory->createAlgorithm(s.ma);
    if (algo == NULL) {
        return;
    }
    addSubTask(algo);
}

QList<Task*> DistanceMatrixMSAProfileTask::onSubTaskFinished(Task* subTask){ 
    MSADistanceAlgorithm* algo = qobject_cast<MSADistanceAlgorithm*>(subTask);
    QList<Task*> res;
    if (algo != NULL) {
        if (s.outFormat != DistanceMatrixMSAProfileOutputFormat_Show && s.outURL.isEmpty()) {
            setError(tr("No output file name specified"));
            return res;
        }
        if (s.outFormat == DistanceMatrixMSAProfileOutputFormat_Show || s.outFormat == DistanceMatrixMSAProfileOutputFormat_HTML) {
            int maxVal = s.usePercents ? 100 : s.ma.getLength();
            QString colors[] = {"#ff5555", "#ff9c00", "#60ff00", "#a1d1e5", "#dddddd"};

            //setup style
            resultText= "<STYLE TYPE=\"text/css\"><!-- \n";
            resultText+="table.tbl   {\n border-width: 1px;\n border-style: solid;\n border-spacing: 0;\n border-collapse: collapse;\n}\n";
            resultText+="table.tbl td{\n max-width: 400px;\n min-width: 20px;\n text-align: center;\n border-width: 1px;\n ";
            resultText+="border-style: solid;\n margin:0px;\n padding: 0px;\n}\n";
            resultText+="--></STYLE>\n";

            //header
            resultText+= "<h2>" + tr("Multiple Sequence Alignment Distance Matrix") + "</h2><br>\n";

            resultText+="<table>\n";
            resultText+= "<tr><td><b>"+tr("Alignment file:") + "</b></td><td>" + s.profileURL + "@" + s.profileName+ "</td></tr>\n";
            resultText+= "<tr><td><b>"+tr("Table content:") + "</b></td><td>" +(s.usePercents ? (algo->getName()+" in percent") : algo->getName()) + "</td></tr>\n";
            resultText+= "</table>\n";
            resultText+="<br><br>\n";

            bool isSimilarity = algo->isSimilarityMeasure();
            int minLen = s.ma.getLength();

            createDistanceTable(algo, resultText, s.ma.getRows());

            resultText+="<br><br>\n";

            if(true == s.showGroupStatistic) {
                resultText+= "<tr><td><b>"+tr("Group statistics of multiple alignment") + "</td></tr>\n";
                resultText+="<table>\n";
                QVector<U2Region> unitedRows;
                s.ma.sortRowsBySimilarity(unitedRows);
                QList<MAlignmentRow> rows;
                int i = 1;
                srand(QDateTime::currentDateTime().toTime_t());
                foreach(const U2Region &reg, unitedRows) {
                    MAlignmentRow row = s.ma.getRow(reg.startPos + qrand() % reg.length);
                    row.setName(QString("Group %1: ").arg(i) + "(" + row.getName() + ")");
                    rows.append(s.ma.getRow(reg.startPos + qrand() % reg.length));

                    resultText+="<tr><td><b>" + QString("Group %1: ").arg(i) + "</b></td><td>";
                    for(int x = reg.startPos; x < reg.endPos(); x++)
                        resultText+= s.ma.getRow(x).getName() + ", ";
                    resultText += "\n";
                    i++;
                    }
                resultText+= "</table>\n";
                resultText+="<br><br>\n";
                createDistanceTable(algo, resultText, rows);
                        }


            //legend:
            resultText+="<br><br>\n";
            resultText+= "<table><tr><td><b>" + tr("Legend:")+"&nbsp;&nbsp;</b>\n";
            if(isSimilarity){
                resultText+="<td bgcolor="+colors[4]+">10%</td>\n";
                resultText+="<td bgcolor="+colors[3]+">25%</td>\n";
                resultText+="<td bgcolor="+colors[2]+">50%</td>\n";
                resultText+="<td bgcolor="+colors[1]+">70%</td>\n";
                resultText+="<td bgcolor="+colors[0]+">90%</td>\n";
            }else{
                resultText+="<td bgcolor="+colors[0]+">10%</td>\n";
                resultText+="<td bgcolor="+colors[1]+">25%</td>\n";
                resultText+="<td bgcolor="+colors[2]+">50%</td>\n";
                resultText+="<td bgcolor="+colors[3]+">70%</td>\n";
                resultText+="<td bgcolor="+colors[4]+">90%</td>\n";
            }
            resultText+="</tr></table><br>\n";
        } else {
            resultText+= " ";
            for (int i=0; i < s.ma.getNumRows(); i++) {
                QString name = s.ma.getRow(i).getName();
                TextUtils::wrapForCSV(name);
                resultText+= "," + name;
            }
            resultText+="\n";

            for (int i=0; i < s.ma.getNumRows(); i++) {
                QString name = s.ma.getRow(i).getName();
                TextUtils::wrapForCSV(name);
                resultText+=name;
                for (int j=0; j < s.ma.getNumRows(); j++) {
                    int val = qRound(algo->getSimilarity(i, j) * (s.usePercents ? (100.0 / s.ma.getLength()) : 1.0));                    
                    resultText+= "," + QString::number(val) + (s.usePercents ? "%" : "");
                }
                resultText+="\n";
            }
        }

        if (s.outFormat != DistanceMatrixMSAProfileOutputFormat_Show) {
            QFile f(s.outURL);
            bool ok = f.open(QIODevice::Truncate | QIODevice::WriteOnly);
            if (!ok) {
                setError(tr("Can't open file for write: %1").arg(s.outURL));
                return res;
            }
            f.write(resultText.toLocal8Bit());
        }
    }
    return res;
}

void DistanceMatrixMSAProfileTask::createDistanceTable(MSADistanceAlgorithm* algo, QString &result, const QList<MAlignmentRow> &rows)
{
    int maxVal = s.usePercents ? 100 : s.ma.getLength();
    QString colors[] = {"#ff5555", "#ff9c00", "#60ff00", "#a1d1e5", "#dddddd"};
    bool isSimilarity = algo->isSimilarityMeasure();
    int minLen = s.ma.getLength();

    if(rows.size() < 2) {
        resultText+= "<tr><td><b>"+tr("There is not enough groups to create distance matrix!") + "</td></tr>\n";
        return;
    }

    resultText+="<table class=tbl>\n";
    resultText+="<tr><td></td>";
    for (int i=0; i < rows.size(); i++) {
        QString name = rows.at(i).getName();
        resultText+="<td> " + name + "</td>";
    }
    resultText+="</tr>\n";

    //out char freqs
    for (int i=0; i < rows.size(); i++) {
        QString name = rows.at(i).getName();
        resultText+="<tr>";
        resultText+="<td> " + name + "</td>";
        for (int j=0; j < rows.size(); j++) {
            if(s.usePercents && s.excludeGaps){
                int len1 = rows.at(i).getUngappedLength();
                int len2 = rows.at(j).getUngappedLength();
                minLen = qMin(len1, len2);
            }
            int val = qRound(algo->getSimilarity(i, j) * (s.usePercents ? (100.0 / minLen) : 1.0));

            QString colorStr = "";
            if (i != j) {
                int hotness = qRound(100 * double(val) / maxVal);
                if ((hotness  >= 90 && isSimilarity) || (hotness <= 10 && !isSimilarity))  {
                    colorStr = " bgcolor=" + colors[0];
                } else if ((hotness  > 70 && isSimilarity) || (hotness <= 25 && !isSimilarity)) {
                    colorStr = " bgcolor=" + colors[1];
                } else if ((hotness  > 50 && isSimilarity) || (hotness <= 50 && !isSimilarity)) {
                    colorStr = " bgcolor=" + colors[2];
                } else if ((hotness  > 25 && isSimilarity) || (hotness <= 70 && !isSimilarity)) {
                    colorStr = " bgcolor=" + colors[3];
                } else if ((hotness  > 10 && isSimilarity) || (hotness < 90 && !isSimilarity)) {
                    colorStr = " bgcolor=" + colors[4];
                }
            }         
            resultText+="<td"+colorStr+">" + QString::number(val) + (s.usePercents ? "%" : "") + "</td>";
        }
        resultText+="</tr>\n";
    }
    resultText+="</table>\n";
}


Task::ReportResult DistanceMatrixMSAProfileTask::report() {
    if (s.outFormat != DistanceMatrixMSAProfileOutputFormat_Show || hasError() || isCanceled()) {
        return Task::ReportResult_Finished;
    }
    assert(!resultText.isEmpty());
    QString title = s.profileName.isEmpty() ? tr("Distance matrix") : tr("Distance matrix for %1").arg(s.profileName);
    WebWindow* w = new WebWindow(title, resultText);
    w->setWindowIcon(QIcon(":core/images/chart_bar.png"));
    AppContext::getMainWindow()->getMDIManager()->addMDIWindow(w);
    return Task::ReportResult_Finished;
}

}//namespace

