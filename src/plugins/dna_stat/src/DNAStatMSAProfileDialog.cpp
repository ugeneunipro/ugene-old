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

#include "DNAStatMSAProfileDialog.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/MAlignmentObject.h>

#include <U2Gui/LastUsedDirHelper.h>

#include <U2View/MSAEditor.h>
#include <U2View/WebWindow.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

namespace U2 {

DNAStatMSAProfileDialog::DNAStatMSAProfileDialog(QWidget* p, MSAEditor* _c) : QDialog(p), ctx(_c) {
    setupUi(this);
    connect(fileButton, SIGNAL(clicked()), SLOT(sl_selectFile()));
    connect(htmlRB, SIGNAL(toggled(bool)), SLOT(sl_formatChanged(bool)));
    connect(csvRB, SIGNAL(toggled(bool)), SLOT(sl_formatChanged(bool)));
}

void DNAStatMSAProfileDialog::sl_selectFile() {
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

void DNAStatMSAProfileDialog::sl_formatChanged(bool) {
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

void DNAStatMSAProfileDialog::accept() {
    DNAStatMSAProfileTaskSettings s;
    MAlignmentObject* msaObj = ctx->getMSAObject();
    if (msaObj == NULL) {
        return;
    }
    s.profileName = msaObj->getGObjectName();
    s.profileURL = msaObj->getDocument()->getURLString();
    s.usePercents = percentsRB->isChecked();
    s.ma = msaObj->getMAlignment();
    s.reportGaps = gapCB->isChecked();
    s.stripUnused = !unusedCB->isChecked();
    s.countGapsInConsensusNumbering = !skipGapPositionsCB->isChecked();
    if (saveBox->isChecked()) {
        s.outURL = fileEdit->text();
        if (s.outURL.isEmpty()) {
            QMessageBox::critical(this, tr("Error"), tr("File URL is empty"));
            return;
        }
        s.outFormat = csvRB->isChecked() ? DNAStatMSAProfileOutputFormat_CSV : DNAStatMSAProfileOutputFormat_HTML;
    }
    AppContext::getTaskScheduler()->registerTopLevelTask(new DNAStatMSAProfileTask(s));
    QDialog::accept();
}


//////////////////////////////////////////////////////////////////////////
// task
DNAStatMSAProfileTask::DNAStatMSAProfileTask(const DNAStatMSAProfileTaskSettings& _s) 
: Task(tr("Generate alignment profile"), TaskFlag_None), s(_s)
{
    setVerboseLogMode(true);
}

void DNAStatMSAProfileTask::run() {
    computeStats();
    if (hasError()) {
        return;
    }
    
    if (s.outFormat != DNAStatMSAProfileOutputFormat_Show && s.outURL.isEmpty()) {
        setError(tr("No output file name specified"));
        return;
    }
    if (s.outFormat == DNAStatMSAProfileOutputFormat_Show || s.outFormat == DNAStatMSAProfileOutputFormat_HTML) {
        int maxVal = s.usePercents ? 100 : s.ma.getNumRows();
        QString colors[] = {"#ff5555", "#ff9c00", "#60ff00", "#a1d1e5", "#dddddd"};
        
        //setup style
        resultText= "<STYLE TYPE=\"text/css\"><!-- \n";
        resultText+="table.tbl   {\n border-width: 1px;\n border-style: solid;\n border-spacing: 0;\n border-collapse: collapse;\n}\n";
        resultText+="table.tbl td{\n max-width: 200px;\n min-width: 20px;\n text-align: center;\n border-width: 1px;\n ";
        resultText+="border-style: solid;\n margin:0px;\n padding: 0px;\n}\n";
        resultText+="--></STYLE>\n";
        
        //header
        resultText+= "<h2>" + tr("Multiple Sequence Alignment Grid Profile") + "</h2><br>\n";
        
        resultText+="<table>\n";
        resultText+= "<tr><td><b>"+tr("Alignment file:") + "</b></td><td>" + s.profileURL + "@" + s.profileName+ "</td></tr>\n";
        resultText+= "<tr><td><b>"+tr("Table content:") + "</b></td><td>" +(s.usePercents ? tr("symbol percents") : tr("symbol counts")) + "</td></tr>\n";
        resultText+= "</table>\n";
        resultText+="<br><br>\n";

        resultText+="<table class=tbl>";

        //consensus numbers line
        resultText+="<tr><td></td>";
        int pos = 1;
        for (int i=0; i< columns.size(); i++) {
            ColumnStat& cs = columns[i];
            QString posStr;
            bool nums = s.countGapsInConsensusNumbering || cs.consChar!=MAlignment_GapChar;
            posStr = nums? QString::number(pos++) : QString("&nbsp;");
//            while(posStr.length() < maxLenLen) {posStr = (nums ? "0" : "&nbsp;") + posStr;}
            resultText+="<td width=20>" + posStr + "</td>";
        }
        resultText+="</tr>\n";

        //consensus chars line

        resultText+="<tr><td> Consensus </td>";
        for (int i=0; i< columns.size(); i++) {
            ColumnStat& cs = columns[i];
            resultText+="<td><b>" + QString(cs.consChar) + "</b></td>";
        }
        resultText+="</tr>\n";
        //out char freqs
        QByteArray aChars = s.ma.getAlphabet()->getAlphabetChars();
        for (int i=0; i < aChars.size(); i++) {
            char c = aChars[i];
            if (c == MAlignment_GapChar && !s.reportGaps) {
                continue;
            }
            if (s.stripUnused && unusedChars.contains(c)) {
                continue;
            }
            int idx = char2index[c];
            resultText+="<tr>";
            resultText+="<td> " + QString(c) + "</td>";
            for (int j=0; j < columns.size(); j++) {
                ColumnStat& cs = columns[j];
                int val = cs.charFreqs[idx];
                QString colorStr;
                int hotness = qRound(100 * double(val) / maxVal);
                if (hotness  >= 90)  {
                    colorStr = " bgcolor=" + colors[0];
                } else if (hotness >= 70) {
                    colorStr = " bgcolor=" + colors[1];
                } else if (hotness > 50) {
                    colorStr = " bgcolor=" + colors[2];
                } else if (hotness > 25) {
                    colorStr = " bgcolor=" + colors[3];
                } else if (hotness > 10) {
                    colorStr = " bgcolor=" + colors[4];
                }
                resultText+="<td"+colorStr+">" + QString::number(cs.charFreqs[idx]) + "</td>";
            }
            resultText+="</tr>\n";
        }
        resultText+="</table>\n";
        
        //legend:
        resultText+="<br><br>\n";
        resultText+= "<table><tr><td><b>" + tr("Legend:")+"&nbsp;&nbsp;</b>\n";
        resultText+="<td bgcolor="+colors[4]+">10%</td>\n";
        resultText+="<td bgcolor="+colors[3]+">25%</td>\n";
        resultText+="<td bgcolor="+colors[2]+">50%</td>\n";
        resultText+="<td bgcolor="+colors[1]+">70%</td>\n";
        resultText+="<td bgcolor="+colors[0]+">90%</td>\n";
        resultText+="</tr></table><br>\n";
    } else {
        //out char freqs
        QByteArray aChars = s.ma.getAlphabet()->getAlphabetChars();
        for (int i=0; i < aChars.size(); i++) {
            char c = aChars[i];
            if (c == MAlignment_GapChar && !s.reportGaps) {
                continue;
            }
            if (s.stripUnused && unusedChars.contains(c)) {
                continue;
            }
            int idx = char2index[c];
            resultText+=QString(c);
            for (int j=0; j < columns.size(); j++) {
                ColumnStat& cs = columns[j];
                resultText+="," + QString::number(cs.charFreqs[idx]);
            }
            resultText+="\n";
        }
    }
    
    if (s.outFormat != DNAStatMSAProfileOutputFormat_Show) {
        QFile f(s.outURL);
        bool ok = f.open(QIODevice::Truncate | QIODevice::WriteOnly);
        if (!ok) {
            setError(tr("Can't open file for write: %1").arg(s.outURL));
            return;
        }
        f.write(resultText.toLocal8Bit());
    }
}

Task::ReportResult DNAStatMSAProfileTask::report() {
    if (s.outFormat != DNAStatMSAProfileOutputFormat_Show || hasError() || isCanceled()) {
        return Task::ReportResult_Finished;
    }
    assert(!resultText.isEmpty());
    QString title = s.profileName.isEmpty() ? tr("Alignment profile") : tr("Alignment profile for %1").arg(s.profileName);
    WebWindow* w = new WebWindow(title, resultText);
    w->setWindowIcon(QIcon(":core/images/chart_bar.png"));
    AppContext::getMainWindow()->getMDIManager()->addMDIWindow(w);
    return Task::ReportResult_Finished;
}

void DNAStatMSAProfileTask::computeStats() {
    //fill names
    QByteArray aChars = s.ma.getAlphabet()->getAlphabetChars();
    for (int i = 0; i < aChars.size(); i++) {
        char c = aChars[i];
        verticalColumnNames.append(QChar(c));
        char2index[uchar(c)] = i;
        unusedChars.insert(c);
    }

    //fill values
    columns.resize(s.ma.getLength());
    consenusChars.resize(s.ma.getLength());
    for (int pos = 0; pos < s.ma.getLength(); pos++) {
        int topCharCount = 0;
        ColumnStat& cs = columns[pos];
        cs.charFreqs.resize(aChars.size());
        cs.consChar = MAlignment_GapChar;
        for (int i = 0; i< s.ma.getNumRows(); i++) {
            const MAlignmentRow& row = s.ma.getRow(i);
            char c = row.chatAt(pos);
            unusedChars.remove(c);
            int idx = char2index.value(c);
            int v = ++cs.charFreqs[idx];
            if (v > topCharCount) {
                topCharCount = v;
                cs.consChar = c;
            } else if (v == topCharCount) {
                cs.consChar = MAlignment_GapChar;
            }
        }
    }

    if (s.usePercents) {
        int charsInColumn = s.ma.getNumRows();
        for (int pos = 0; pos < s.ma.getLength(); pos++) {
            ColumnStat& cs = columns[pos];
            for (int i=0; i < aChars.size(); i++) {
                char c = aChars[i];
                int idx = char2index.value(c);
                cs.charFreqs[idx] = qRound(100.0 * cs.charFreqs[idx] / charsInColumn);
            }
        }
    }
}


}//namespace

