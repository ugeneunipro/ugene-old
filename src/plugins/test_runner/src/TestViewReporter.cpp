#include "TestViewController.h"
#include "TestRunnerPlugin.h"
#include <U2Test/TestRunnerTask.h>
#include "TestViewReporter.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Test/GTest.h>
#include <U2Test/GTestFrameworkComponents.h>
#include <U2Core/IOAdapter.h>
#include <QtGui/QFileDialog>
#include <QtGui/QToolBar>

#include <QtGui/QMenu>
#include <memory>
//todo: remember splitter geom



namespace U2 {

#define SETTINGS_ROOT QString("test_runner/view/")

TestViewReporter::TestViewReporter(TestViewController* parent,QTreeWidget* tree,int runTime) : MWMDIWindow("Report")
{
    curParent=parent;
    setupUi(this);

    save = new QAction(tr("save"), this);
    save->setObjectName("action_save");
    connect(save, SIGNAL(triggered()), SLOT(sl_save()));

    curReportText=prepareHTMLText(tree,runTime);
    this->reportText->setHtml(curReportText);
    //saveAs("c:/1.html", curReportText);//test line

}
//-------------------------------------------------------------------------
const QString TestViewReporter::prepareHTMLText(QTreeWidget* tree,int runTime){
    QString rezult;
    QString tempRezult;
    if(tree==NULL){rezult="treeRoot is Empty";}
    else{
        int tpassed = 0;
        int tfailed = 0;
        int tnone = 0;
        int texcluded = 0;
        for (int i=0, n = tree->topLevelItemCount(); i<n; i++) {
            TVItem* item = static_cast<TVItem*>(tree->topLevelItem(i));     
            assert(item->isSuite());
            TVTSItem* tItem  = static_cast<TVTSItem*>(item);

            int passed=0,failed=0,none=0;
            int excluded = 0;
            int procent=100;
            bool norun=false;

            tItem->getTestsState(&passed,&failed,&none, &excluded);
            tpassed=tpassed+passed;
            tfailed=tfailed+failed;
            tnone=tnone+none;
            texcluded+=excluded; //TODO include to report

            if(failed!=0 && passed!=0){
                procent=(passed*100)/(failed+passed);
            }
            if(passed==0 && failed!=0){
                procent=0;
            }
            if(passed==0 && failed==0){
                norun=true;
            }
            QString temp;
            temp=tItem->ts->getName();
            tempRezult+="<tr>\n";
            tempRezult+=getHTMLStaticInfo(&temp);
            tempRezult+=getHTMLStaticInfo(passed+failed+none);
            tempRezult+=getHTMLStaticInfo(passed);
            tempRezult+=getHTMLStaticInfo(failed);
            tempRezult+=getHTMLStaticInfo(excluded);
            tempRezult+=getHTMLStatusBar(procent,norun);
            tempRezult+="</tr>\n";
        }
        int tprocent=100;
        if(tfailed && tpassed){
            tprocent=(tpassed*100)/(tfailed+tpassed);
        }
        if(!tpassed && tfailed){
            tprocent=0;
        }
        if(!tpassed && !tfailed){
            rezult=getHTMLNoTests();
            return rezult;
        }
        rezult+=getHTMLHead();
        rezult+=getHTMLFirstPart("_","_");//any information        

        rezult+=getHTMLRuntime(runTime);

        QString totalName;
        totalName="Total:";
        rezult+=getHTMLStaticInfo(&totalName);
        rezult+=getHTMLStaticInfo(tpassed+tfailed+tnone);
        rezult+=getHTMLStaticInfo(tpassed);
        rezult+=getHTMLStaticInfo(tfailed);
        rezult+=getHTMLStaticInfo(texcluded);
        rezult+=getHTMLStatusBar(tprocent);

        rezult+=tempRezult;

        rezult+=getHTMLEndOfStaticBar();
        rezult+=getHTMLLastPart(tree);
    }

    return rezult;
}

const QString TestViewReporter::getHTMLNoTests(){
    QString rezult;
    rezult=("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 //EN\" \"http://www.w3.org/TR/html401/loose.dtd\">\n<html>\n<head>\n<META http-equiv=\"Content-Type\" content=\"text/html; charset=US-ASCII\">\n<TITLE>UGENE Test Report</TITLE>\n</head>\n<body>\nNo Failed Tests</body>\n</html>");
    return rezult;
}

const QString TestViewReporter::getHTMLHead(){
    QString rezult;
    rezult=("<html lang=\"ru\">\n<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 //EN\" \"http://www.w3.org/TR/html401/loose.dtd\">\n<head>\n<META http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n");
    rezult+=getHTMLStyle();
    rezult+=("\n<TITLE>UGENE Test Report</TITLE>\n</head>\n<body>\n");
    rezult+=("<script language=\"JavaScript\">\n<!--\nfunction Show_Stuff(Click_Menu)\n{\nif (Click_Menu.style.display == \"none\")\n{\nClick_Menu.style.display = \"\";\n}\nelse\n{\nClick_Menu.style.display = \"none\";\n}\n}\n\n");
    rezult+=("function Show_Stuff_Ever(Click_Menu)\n{\nClick_Menu.style.display = \"\";\n}\n-->\n</script>");
    return rezult;
}

const QString TestViewReporter::getHTMLStyle(){
    QString rezult;
    rezult=("<style type=\"text/css\">\nbody {\nfont-family: verdana, arial, sans-serif;\nfont-size: medium; \ncolor: black\n}\ntable.title {\nborder: 1px solid #c7cbcc;\nbackground-color: #e1f6f7;\n}\n\ntable.info {\nbackground-color: #c7cbcc;\n}\nth {\nbackground-color: white;}tr.suiteName {\nbackground-color: #e1f6f7;\n}\n\ntd.title {\n font-size: large;\nfont-weight: bold;\n}\ntd {\nwhite-space: nowrap;\n}\n\ntable.bar {\n background-color: #f25913;\n}\ntable.goodbar {\n background-color: #60de51;\n}\ntd.norun {\n background-color: #0000FF;\n}\ntd.bar {\nbackground-color: #60de51;\n}\n</style>");
    return rezult;
}

const QString TestViewReporter::getHTMLFirstPart(const char *info1, const char *info2){
    QString rezult;
    rezult=("\n\n<table width=\"100%\" class=\"title\">\n<tr>\n<td class=\"title\">UGENE Test Report</td><td align=\"right\"><em>\n");
    if(info1)rezult+= info1;
    rezult+=("</em></td>\n</tr>\n<tr>\n<td>&nbsp;&nbsp;&nbsp;generated by <A HREF=\"http://ugene.unipro.ru\">UGENE</A></td><td align=\"right\"><em>\n");
    if(info2)rezult+= info2;
    rezult+=("</em></td>\n</tr>\n<tr>\n<td>&nbsp;&nbsp;&nbsp;\n");
    QDate d = QDate::currentDate ();
    QTime t = QTime::currentTime();
    rezult+=d.toString();
    rezult+=" ";
    rezult+=t.toString();
    return rezult;
}

const QString TestViewReporter::getHTMLRuntime(int data){
    QString rezult;
    rezult+=("</td>\n</tr>\n<table width=\"100%\" cellpadding=\"2\" cellspacing=\"1\" class=\"info\">\n\n<tr>\n<th>");

    rezult+="Total Runtime: ";
    rezult+=QString::number(data);
    rezult+=" s";

    rezult+=("</th>\n<th>Number of tests</th>\n<th>Pass</th>\n<th>Fail</th>\n<th>Excluded</th>\n<th colspan=\"2\">Success Rate</th>\n</tr>\n\n<tr>\n");
    return rezult;
}

const QString TestViewReporter::getHTMLStaticInfo(int data){
    QString rezult;
    rezult+="<th>";
    rezult+=QString::number(data);
    rezult+="</th>\n";
    return rezult;
}

const QString TestViewReporter::getHTMLStaticInfo(QString* data){
    QString rezult;
    rezult+="<th>";
    rezult+=data;
    rezult+="</th>\n";
    return rezult;  
}
const QString TestViewReporter::getHTMLStaticInfo(char* info){
    QString rezult;
    rezult+="<th>";
    rezult+=info;
    rezult+="</th>\n";
    return rezult;
}

const QString TestViewReporter::getHTMLStatusBar(int data, bool norun){
    QString rezult;
    rezult+="<th>";
    rezult+=QString::number(data);
    rezult+="%</th>\n";
    rezult+=("\n<th>\n<table cellpadding=\"0\" cellspacing=\"0\" width=\"100\" class=\"bar\">\n<tr>\n<td>\n<table cellpadding=\"0\" cellspacing=\"0\" align=\"left\" width=\"");
    rezult+=QString::number(data);
    if(norun){
        rezult+=("%\" class=\"goodbar\">\n<tr>\n<td height=\"12\" class=\"norun\"></td>\n</tr>\n</table>\n</td>\n</tr>\n</table>\n</th>\n");
    }    
    else{
    rezult+=("%\" class=\"goodbar\">\n<tr>\n<td height=\"12\" class=\"bar\"></td>\n</tr>\n</table>\n</td>\n</tr>\n</table>\n</th>\n");
    }
    //rezult+="<th><table cellpadding=\"0\" cellspacing=\"0\" width=\"100\" class=\"bar\"><tr><td><table align=\"left\" cellpadding=\"0\" cellspacing=\"0\" width=\"55%\" class=\"goodbar\"><tr><td height=\"12\" ></td></tr></table></td></tr></table></th>"; 
    return rezult;
}

const QString TestViewReporter::getHTMLEndOfStaticBar(){
    QString rezult;
    rezult+=("</table>\n</table>\n\n\n<tr>\n<td></td>\n<td></td>\n</tr>\n");
    return rezult;
}
const QString TestViewReporter::getHTMLLastPart(QTreeWidget* tree){
    QString rezult;
    rezult+=("<table width=\"100%\">\n<tr>\n<td>\n");
    rezult+=getHTMLErrorTables(tree);
    rezult+=("</td>\n</tr>\n\n</body>\n</html>\n");
    return rezult;
}

const QString TestViewReporter::getHTMLErrorTables(QTreeWidget* tree){
    QString rezult;
    bool testsText=true;
    //create all tables
    int testIndex=1;
    int testIndex2=1;
    for (int i=0, n = tree->topLevelItemCount(); i<n; i++) {
        TVItem* item = static_cast<TVItem*>(tree->topLevelItem(i));     
        assert(item->isSuite());
        TVTSItem* tItem  = static_cast<TVTSItem*>(item);
        QList<TVTestItem*> failedTests = getFailedTests(tItem);
        if(!failedTests.isEmpty()){
            rezult+=getHTMLSuiteName(tItem);
            if(testsText==true){rezult+=getHTMLErrorList(failedTests,&testIndex);}
            else {rezult+=getHTMLErrorList(failedTests);}
        }
    }
    //create all tests texts
    if(testsText==true){
        rezult+="</table>";
        rezult+="<tr><td><br><br><br><br><br><hr></td></tr>";
        for (int i=0, n = tree->topLevelItemCount(); i<n; i++) {
            TVItem* item = static_cast<TVItem*>(tree->topLevelItem(i));     
            assert(item->isSuite());
            TVTSItem* tItem  = static_cast<TVTSItem*>(item);
            QList<TVTestItem*> failedTests = getFailedTests(tItem);
            if(!failedTests.isEmpty()){
                rezult+="<table>";
                rezult+=getHTMLSuiteName(tItem);
                rezult+="</table>";
                rezult+="<tr><td>";
                rezult+=getHTMLTestsTexts(failedTests,&testIndex2);
                rezult+="</td></tr>";
            }
        }
    }
    assert(testIndex==testIndex2);
    return rezult;
}

const QString TestViewReporter::getHTMLSuiteName(TVTSItem* Suite){
    QString rezult;   
    rezult+=("<tr class=\"suiteName\">\n<td colspan=\"2\">");
    rezult+=("Failed test in Suite: ");
    rezult+=Suite->ts->getName();
    rezult+=("</td>\n</tr>\n");
    return rezult;
}

const QString TestViewReporter::getHTMLErrorList(QList<TVTestItem*> failedTests,int* index){
    QString rezult; 
    rezult+=("<td>\n<table width=\"100%\" cellpadding=\"2\" cellspacing=\"1\" class=\"info\">\n");
    foreach(TVTestItem* curItem,failedTests){
        rezult+=("<tr>\n<th width=\"50%\" align=\"left\">");
        rezult+=curItem->testState->getTestRef()->getShortName();
        rezult+=("</th><th width=\"50%\" align=\"left\">");
        if(index!=NULL){
            rezult+="<a href=\"#Test_";
            rezult+=QString::number(*index);
            rezult+="\" ";

            rezult+="onClick=\"javascript:Show_Stuff_Ever(display";
            rezult+=QString::number(*index);
            rezult+=")\" ";

            rezult+=">";
            *index=*index+1;
        }
        rezult+=curItem->testState->getErrorMessage();
        if(index!=NULL){
            rezult+="</a>";
        }
        rezult+=("</th>\n</tr>\n");
    }
    rezult+="</table></td>\n";
    return rezult;
}
const QString TestViewReporter::getHTMLTestsTexts(QList<TVTestItem*> failedTests,int* index){
    QString rezult;

    foreach(TVTestItem* curItem,failedTests){
        //----------------------
        rezult+="<DIV><a href=\"javascript:Show_Stuff(display";
        rezult+=QString::number(*index);
        rezult+=")\" ";

        rezult+="name=\"Test_";
        rezult+=QString::number(*index);
        rezult+="\"";
        rezult+=">";
        rezult+=curItem->testState->getTestRef()->getShortName();
        rezult+="</a></DIV>\n";
        //----------------------
        rezult+="<div ID=\"display";
        rezult+=QString::number(*index);
        rezult+="\" style=\"display: none\">\n";
        //----------------------
        *index=*index+1;

        rezult+="<table width=\"100%\" class=\"info\">\n";

        QString tempOne=curItem->getRichDesc();
        setColorInTestText(&tempOne);

        tempOne.remove("<b>"+curItem->testState->getTestRef()->getShortName()+"</b><br>");
        tempOne.remove("<hr>");
        rezult+="<tr><td>";
        rezult+=tempOne;
        rezult+="<hr>";
        rezult+="</td></tr>";
        rezult+="</table>\n";
        //----------------------
        rezult+="</div>\n";

    }

    return rezult;
}

bool TestViewReporter::setColorInTestText(QString* inputData){
    if(inputData==NULL)return false;

    QString mainCommentColor=" <font color=\"#339966\" mainColorPoint>\\1</font mainColorPointEnd> ";
    QString commentColor=" <font color=\"#33FF66\" commentColorPoint>\\1</font commentColorPointEnd> ";
    QString normalTagColor="<font color=\"#3366FF\">\\1</font>";
    QString valueColor=" <font color=\"#FF3333\">\\1</font>=<font color=\"#0000CC\">\\2</font> ";
    QString deleteColor="\\1\\2";

    inputData->replace( QRegExp("(\\s[^\\s]*)=(\"[^\"]*\")"),valueColor);//first find all values

    QRegExp rx("(&lt;!--.*--&gt;)");
    rx.setMinimal(true);
    inputData->replace(rx,commentColor);//2- find all comments

    rx.setPattern("(&lt;!.*&gt;)"); 
    inputData->replace(rx,mainCommentColor);//2- find all main comments

    rx.setPattern("(&lt;.*\\s)");
    inputData->replace(rx,normalTagColor);//3- find all tag names
    rx.setPattern("(&lt;[^\\s]*&gt;)");
    inputData->replace(rx,normalTagColor);//3- find all tag names
    rx.setPattern("(&gt;)");
    inputData->replace(rx,normalTagColor);//3- find all tag names

    rx.setPattern("(commentColorPoint.*)</font>(.*commentColorPointEnd)");
    while(rx.indexIn( *inputData )!=-1)
        inputData->replace (rx,deleteColor);//4- find information to delete

    rx.setPattern("(commentColorPoint.*)<font color=\".*\">(.*commentColorPointEnd)");
    while(rx.indexIn( *inputData )!=-1)
        inputData->replace (rx,deleteColor);//4- find information to delete

    rx.setPattern("(commentColorPoint.*)</font mainColorPointEnd>(.*commentColorPointEnd)");
    while(rx.indexIn( *inputData )!=-1)
        inputData->replace (rx,deleteColor);//4- find information to delete

    rx.setPattern("(commentColorPoint.*)<font color=\"#339966\" mainColorPoint>(.*commentColorPointEnd)");
    while(rx.indexIn( *inputData )!=-1)
        inputData->replace (rx,deleteColor);//4- find information to delete

    rx.setPattern("(mainColorPoint.*)</font>(.*mainColorPointEnd)");
    while(rx.indexIn( *inputData )!=-1)
        inputData->replace (rx,deleteColor);//4- find information to delete

    rx.setPattern("(mainColorPoint.*)<font color=\".*\">(.*mainColorPointEnd)");
    while(rx.indexIn( *inputData )!=-1)
        inputData->replace (rx,deleteColor);//4- find information to delete

    inputData->remove("commentColorPointEnd");
    inputData->remove("commentColorPoint");
    inputData->remove("mainColorPointEnd");
    inputData->remove("mainColorPoint");

    return true;
}
//-------------------------------------------------------------------------
QList<TVTestItem*> TestViewReporter::getFailedTests(TVTSItem* Root){    
    QList<TVTestItem*> rezult;
    for(int i = 0; i < Root->childCount(); i++) {
        TVItem* item = static_cast<TVItem*>(Root->child(i));
        if(item->isTest()){ 
            TVTestItem* tItem  = static_cast<TVTestItem*>(item);
            if(tItem->testState->isFailed())rezult.append(tItem);
        }

        else{
            assert(item->isSuite());
            TVTSItem* tItem  = static_cast<TVTSItem*>(item);
            rezult+=getFailedTests(tItem);      
        }
    } 
    return rezult;
}
bool TestViewReporter::saveAs(const QString url,const QString data){
    if(url.isEmpty())return false;
    if(data.isEmpty())return false;
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
    std::auto_ptr<IOAdapter> io(iof->createIOAdapter());
    if (!io->open(url, IOAdapterMode_Write)) {
        return false;
        //ti.setError(  IOAdapter::tr("error_opening_url_for_write '%1'").arg(url) );
    }
    io->writeBlock(data.toUtf8());
    return true;

}

void TestViewReporter::setupMDIToolbar(QToolBar* tb) {
    tb->addAction(save);
}

void TestViewReporter::setupViewMenu(QMenu* m) {
    m->addAction(save);
}

void TestViewReporter::sl_save(){
    QString dir = AppContext::getSettings()->getValue(SETTINGS_ROOT + "lastDir", QString()).toString();
    QString file = QFileDialog::getSaveFileName(this, tr("select_save_path"), dir,tr("*.html"));
    if (file.isEmpty()) {
        return;
    }
    else{
        saveAs(file,curReportText);
    }
}

bool TestViewReporter::onCloseEvent() {
    curParent->reporterFormClosedOutside();
    return true;
}

}
