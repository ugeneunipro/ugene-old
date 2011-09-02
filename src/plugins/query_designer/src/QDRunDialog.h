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

#ifndef _U2_QD_RUN_DIALOG_H_
#define _U2_QD_RUN_DIALOG_H_

#include <ui/ui_RunQueryDialog.h>
#include <ui/ui_QDDialog.h>

#include <U2Core/Task.h>
#include <U2Core/U2Region.h>
#include <U2Gui/RegionSelector.h>


namespace U2 {

class QDScheme;
class Document;
class QDScheduler;
class SaveDocumentTask;
class AnnotationTableObject;

class QDRunDialogTask : public Task {
    Q_OBJECT
public:
    QDRunDialogTask(QDScheme* scheme, const QString& inUri, const QString& outUri, bool addToProject);
protected:
    virtual QList<Task*> onSubTaskFinished(Task* subTask);
private slots:
    void sl_updateProgress();
private:
    QList<Task*> init();
    void setupQuery(Document* doc);
private:
    QDScheme* scheme;
    QString inUri;
    QString output;
    bool addToProject;
    Task* openProjTask;
    Task* loadTask;
    QDScheduler* scheduler;
    Document* inDoc;
    AnnotationTableObject* ato;
};

class QueryViewController;

class QDRunDialog : public QDialog, public Ui_RunQueryDlg {
    Q_OBJECT
public:
    QDRunDialog(QDScheme* _scheme, QWidget* parent, const QString& defaultIn = QString(), const QString& defaultOut = QString());
private slots:
    void sl_run();
    void sl_selectFile();
private:
    QDScheme* scheme;
};

class ADVSequenceObjectContext;
class CreateAnnotationWidgetController;

class QDDialog : public QDialog, public Ui_QDDialog {
    Q_OBJECT
public:
    QDDialog(ADVSequenceObjectContext* ctx);
private:
    void addAnnotationsWidget();
    void connectGUI();
private slots:
    void sl_selectScheme();
    void sl_okBtnClicked();
private:
    ADVSequenceObjectContext* ctx;
    CreateAnnotationWidgetController* cawc;
    QDScheme* scheme;
    QTextDocument* txtDoc;
    RegionSelector* rs;
};

}//namespace

#endif
