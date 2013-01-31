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

#ifndef _U2_DISTANCE_MATRIX_MSA_PROFILE_DIALOG_H_
#define _U2_DISTANCE_MATRIX_MSA_PROFILE_DIALOG_H_

#include "ui/ui_DistanceMatrixMSAProfileDialog.h"

#include <U2Core/global.h>
#include <U2Core/Task.h>
#include <U2Core/MAlignment.h>

#include <QtCore/QHash>
#include <QtCore/QSet>

namespace U2 {

class MSAEditor;

class DistanceMatrixMSAProfileDialog : public QDialog, public Ui_DistanceMatrixMSAProfileDialog {
    Q_OBJECT

public:
    DistanceMatrixMSAProfileDialog(QWidget* p, MSAEditor* ctx);

    virtual void accept();

private slots:
    
    void sl_selectFile();
    void sl_formatChanged(bool);

private:
    MSAEditor* ctx;
};

enum DistanceMatrixMSAProfileOutputFormat {
    DistanceMatrixMSAProfileOutputFormat_Show,
    DistanceMatrixMSAProfileOutputFormat_CSV,
    DistanceMatrixMSAProfileOutputFormat_HTML
};

class DistanceMatrixMSAProfileTaskSettings {
public:
    DistanceMatrixMSAProfileTaskSettings(){ 
        outFormat = DistanceMatrixMSAProfileOutputFormat_Show; 
        usePercents = false;
    }

    QString                         algoName;    // selected algorithm
    QString                         profileName; // usually object name
    QString                         profileURL;  // document url
    MAlignment                      ma;
    bool                            usePercents; //report percents but not counts
    bool                            excludeGaps; //exclude gaps when calculate distance
    DistanceMatrixMSAProfileOutputFormat   outFormat;   
    QString                         outURL;    
};

class DistanceMatrixMSAProfileTask : public Task {
    Q_OBJECT
public:
    DistanceMatrixMSAProfileTask(const DistanceMatrixMSAProfileTaskSettings& s);

    virtual void prepare();
    
    QList<Task*> onSubTaskFinished(Task* subTask);
    //void run();    
    ReportResult report();

private:
    
    DistanceMatrixMSAProfileTaskSettings   s;        
    QString                         resultText;
};

}//namespace

#endif
