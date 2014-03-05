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

#ifndef _U2_TEST_VIEW_REPORTER_H_
#define _U2_TEST_VIEW_REPORTER_H_

#include "TestViewController.h"

#include <U2Gui/MainWindow.h>
#include <ui/ui_Reporter.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QTreeWidgetItem>
#else
#include <QtWidgets/QTreeWidgetItem>
#endif


namespace U2 {

    class TVTSItem;
    class TVTestItem;
    

    class TestViewReporter : public  MWMDIWindow, Ui::Reporter {
     Q_OBJECT

 protected:
    virtual bool onCloseEvent();

 public:

    TestViewReporter(TestViewController* parent,QTreeWidget* tree=NULL,int runTime=-1);
    QString getReportText() {return curReportText;}
    bool saveAs(const QString url,const QString data);

    virtual void setupMDIToolbar(QToolBar* tb);
    virtual void setupViewMenu(QMenu* n);
 
 private:
    const QString prepareHTMLText(QTreeWidget* tree=NULL,int runTime=0);
     
    const QString getHTMLNoTests();
    const QString getHTMLHead();
    const QString getHTMLStyle();
    const QString getHTMLFirstPart(const char *info1=NULL, const char *info2=NULL);
    const QString getHTMLStaticInfo(int data);
    const QString getHTMLStaticInfo(QString* data);
    const QString getHTMLStaticInfo(char *info1=NULL);
    const QString getHTMLRuntime(int data);
    const QString getHTMLStatusBar(int data, bool norun=false);
    const QString getHTMLEndOfStaticBar();
    const QString getHTMLLastPart(QTreeWidget* tree);
    const QString getHTMLErrorTables(QTreeWidget* tree);
    const QString getHTMLSuiteName(TVTSItem* Suite);
    const QString getHTMLErrorList(QList<TVTestItem*> failedTests,int* index=NULL);
    const QString getHTMLTestsTexts(QList<TVTestItem*> failedTests,int* index);

    bool setColorInTestText(QString* inputData=NULL);

    QList<TVTestItem*> getFailedTests(TVTSItem* Root);
    
   
    QString curReportText;
    TestViewController* curParent;
    
    QAction* save;
    private slots:
        void sl_save();
};

}
#endif
