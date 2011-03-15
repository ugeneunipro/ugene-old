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

#ifndef _U2_DNASTAT_MSA_PROFILE_DIALOG_H_
#define _U2_DNASTAT_MSA_PROFILE_DIALOG_H_

#include "ui/ui_DNAStatMSAProfileDialog.h"

#include <U2Core/global.h>
#include <U2Core/Task.h>
#include <U2Core/MAlignment.h>

#include <QtCore/QHash>
#include <QtCore/QSet>

namespace U2 {

class MSAEditor;

class DNAStatMSAProfileDialog : public QDialog, public Ui_DNAStatMSAProfileDialog {
    Q_OBJECT

public:
    DNAStatMSAProfileDialog(QWidget* p, MSAEditor* ctx);

    virtual void accept();

private slots:
    
    void sl_selectFile();
    void sl_formatChanged(bool);

private:
    MSAEditor* ctx;
};

enum DNAStatMSAProfileOutputFormat {
    DNAStatMSAProfileOutputFormat_Show,
    DNAStatMSAProfileOutputFormat_CSV,
    DNAStatMSAProfileOutputFormat_HTML
};

class DNAStatMSAProfileTaskSettings {
public:
    DNAStatMSAProfileTaskSettings(){ 
        outFormat = DNAStatMSAProfileOutputFormat_Show; 
        usePercents = false;
        reportGaps = false;
        stripUnused = false;
        countGapsInConsensusNumbering = true;
    }

    QString                         profileName; // usually object name
    QString                         profileURL;  // document url
    MAlignment                      ma;
    bool                            usePercents; //report percents but not counts
    DNAStatMSAProfileOutputFormat   outFormat;   
    QString                         outURL;
    bool                            reportGaps;  // report GAPS statistics
    bool                            stripUnused; // do not include into report chars unused in alignment
    bool                            countGapsInConsensusNumbering; //affects html output only
};

class DNAStatMSAProfileTask : public Task {
    Q_OBJECT
public:
    DNAStatMSAProfileTask(const DNAStatMSAProfileTaskSettings& s);

    void run();
    
    ReportResult report();

private:
    void computeStats();
    
    DNAStatMSAProfileTaskSettings   s;
    
    // fields to keep statistics
    struct ColumnStat {
        char                consChar;
        QVector<int>        charFreqs;
    };
    

    QStringList                     verticalColumnNames;
    QVector<ColumnStat>             columns;
    QVector<char>                   consenusChars;
    QHash<char, int>                char2index;  //char 2 index in 'ColumnStat'
    QSet<char>                      unusedChars;
    QString                         resultText;
};

}//namespace

#endif
