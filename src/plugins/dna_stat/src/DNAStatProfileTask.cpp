/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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
#include <U2Core/GObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/AppContext.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/WebWindow.h>

#include <U2View/CharOccurTask.h>
#include <U2View/DinuclOccurTask.h>
#include <U2View/DNAStatisticsTask.h>

#include "DNAStatProfileTask.h"

namespace U2 {

DNAStatProfileTask::DNAStatProfileTask(ADVSequenceObjectContext* context)
    :Task(tr("Generate sequence statistics profile"), TaskFlags_NR_FOSE_COSC),
      ctx(context),
      charTask(NULL),
      dinucTask(NULL),
      statTask(NULL)
{
    SAFE_POINT_EXT(ctx != NULL, setError(tr("Sequence context is NULL")), );

    charTask = new CharOccurTask(ctx->getAlphabet(), ctx->getSequenceRef(), U2Region(0, ctx->getSequenceLength()));
    dinucTask = new DinuclOccurTask(ctx->getAlphabet(), ctx->getSequenceRef(), U2Region(0, ctx->getSequenceLength()));
    statTask = new DNAStatisticsTask(ctx->getAlphabet(), ctx->getSequenceRef(), U2Region(0, ctx->getSequenceLength()));

    addSubTask(charTask);
    addSubTask(dinucTask);
    addSubTask(statTask);

    seqLen = ctx->getSequenceLength();
}

QList <Task*> DNAStatProfileTask::onSubTaskFinished(Task *subTask) {
    QList <Task*> res;

    if (subTask == charTask) {
        charResult = formCharResultString();
    }
    if (subTask == dinucTask) {
        dinucResult = formDinucResultString();
    }
    if (subTask == statTask) {
        statResult = formStatResultString();
    }

    return res;
}

QString DNAStatProfileTask::getResult() const {
    QString resultText;

    //setup style
    resultText= "<STYLE TYPE=\"text/css\"><!-- \n";
    resultText+="table.tbl   {\n border-width: 1px;\n border-style: solid;\n border-spacing: 0;\n border-collapse: collapse;\n}\n";
    resultText+="table.tbl td{\n max-width: 200px;\n min-width: 20px;\n text-align: center;\n border-width: 1px;\n ";
    resultText+="border-style: solid;\n margin:0px;\n padding: 0px;\n}\n";
    resultText+="--></STYLE>\n";

    resultText += statResult;
    resultText += charResult;
    resultText += dinucResult;

    return resultText;
}

QString DNAStatProfileTask::formCharResultString() const {
    QList<CharOccurResult> result = charTask->getResult();

    QString resultText;
    resultText+="<table class=tbl>";
    resultText+="<tr><td></td><td>" +
        DNAStatProfileTask::tr("Symbol counts") + "</td><td>" +
        DNAStatProfileTask::tr("Symbol percents %") + "</td></tr>";

    foreach (CharOccurResult r, result) {
        resultText += QString("<tr><td><b>%1</b></td><td>%2</td><td>%3</td></tr>")
                .arg( r.getChar() )
                .arg(QString::number( r.getNumberOfOccur() ))
                .arg(QString::number( r.getPercentage(), 'g', 4));
    }
    resultText+= "</table>\n";
    return resultText;
}

QString DNAStatProfileTask::formDinucResultString() const {
    CHECK(seqLen != 0, QString());
    QMap <QByteArray, qint64> result = dinucTask->getResult();

    QString resultText;
    if (!result.isEmpty()){
        resultText+="<br>";
        resultText+="<table class=tbl>";
        resultText+="<tr><td></td><td>" +
            DNAStatProfileTask::tr("Dinucleotide counts") + "</td><td>" +
            DNAStatProfileTask::tr("Dinucleotide percents %") + "</td></tr>";
        QMap<QByteArray, qint64>::const_iterator it(result.begin());
        for(;it != result.end(); it++){
            const QByteArray diNucl = it.key();
            const qint64 cnt = it.value();
            double percentage = cnt/(double)seqLen * 100;
            resultText+=QString("<tr><td><b>%1</b></td><td>%2</td><td>%3</td></tr>").arg(QString(diNucl)).arg(QString::number(cnt)).arg(QString::number(percentage, 'g', 4));
        }
        resultText+= "</table>\n";
    }

    return resultText;
}

QString DNAStatProfileTask::formStatResultString() const {
    QString seqName = ctx->getSequenceGObject()->getGObjectName();
    QString url = ctx->getSequenceGObject()->getDocument()->getURL().getURLString();
    U2SequenceObject* dnaObj = ctx->getSequenceObject();

    QString resultText;
    resultText+="<h2>" + DNAStatProfileTask::tr("Sequence Statistics") + "</h2><br>\n";
    resultText+="<table>\n";
    resultText+="<tr><td><b>" + DNAStatProfileTask::tr("Sequence file:") + "</b></td><td>" + url + "@" + seqName + "</td></tr><tr><td><b>" +
        DNAStatProfileTask::tr("Sequence length:") + "</b></td><td>" + QString::number(seqLen) + "</td></tr>\n";
    resultText += "<tr><td><b>Molecule Type:</b></td><td>" + ctx->getSequenceObject()->getAlphabet()->getName() + "</td></tr>\n";

    const DNAAlphabet* al = dnaObj->getAlphabet();
    DNAStatistics result = statTask->getResult();
    if (al->isNucleic()) {
        resultText += "<tr><td><b>" + DNAStatProfileTask::tr("GC content:") + "</b></td><td>"
                + QString("%1 %").arg(result.gcContent, 0, 'f', 2) + "</td></tr>\n";

        resultText += "<tr><td><b>" + DNAStatProfileTask::tr("Molar Weight:") + "</b></td><td>"
                + QString("%1 Da").arg(result.molarWeight, 0, 'f', 2) + "</td></tr>\n";

        resultText += "<tr><td><b>" + DNAStatProfileTask::tr("Molar ext. coef.:") + "</b></td><td>"
                + QString("%1 I/mol (at 260 nm)").arg(result.molarExtCoef) + "</td></tr>\n";

        resultText += "<tr><td><b>" + DNAStatProfileTask::tr("Melting Tm:") + "</b></td><td>"
                + QString("%1 C (at salt CC 50 mM, primer CC 50 mM, pH 7.0)").arg(result.meltingTm, 0, 'f', 2) + "</td></tr>\n";

    } else if (al->isAmino()) {
        resultText += "<tr><td><b>" + DNAStatProfileTask::tr("Molecular Weight:") + "</b></td><td>"
                + QString("%1").arg(result.molecularWeight, 0, 'f', 2) + "</td></tr>\n";

        resultText += "<tr><td><b>" + DNAStatProfileTask::tr("Isoelectric Point (pI):") + "</b></td><td>"
                + QString("%1").arg(result.isoelectricPoint, 0, 'f', 2) + "</td></tr>\n";
    }

    resultText+="</table>\n";
    resultText+="<br><br>\n";

    return resultText;
}

} //namespace

