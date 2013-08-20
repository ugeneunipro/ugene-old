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

#include "SpideySupport.h"
#include "SpideySupportTask.h"
#include "ExternalToolSupportSettingsController.h"
#include "ExternalToolSupportSettings.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Gui/AppSettingsGUI.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Algorithm/SplicedAlignmentTaskRegistry.h>
#include <U2Gui/MainWindow.h>
#include <QtGui/QMainWindow>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/ADVConstants.h>
#include <U2View/ADVUtils.h>
#include <U2View/ADVSequenceObjectContext.h>

#include <U2Gui/ProjectTreeController.h>
#include <U2Gui/ProjectTreeItemSelectorDialog.h>
#include <U2Gui/GUIUtils.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/CreateAnnotationDialog.h>
#include <U2Gui/CreateAnnotationWidgetController.h>


namespace U2 {


SpideySupport::SpideySupport(const QString& name, const QString& path) : ExternalTool(name, path)
{
    if (AppContext::getMainWindow()) {
        viewCtx = new SpideySupportContext(this);
        icon = QIcon(":external_tool_support/images/cmdline.png");
        grayIcon = QIcon(":external_tool_support/images/cmdline_gray.png");
        warnIcon = QIcon(":external_tool_support/images/cmdline_warn.png");
    }
#ifdef Q_OS_WIN
    executableFileName="Spidey.exe";
#else
    #if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    executableFileName="spidey";
    #endif
#endif
    validMessage="SPIDEY   arguments:";
    description=tr("<i>Spidey</i> is mRNA-to-DNA alignment program. \
                   <br>Binaries can be downloaded from http://www.ncbi.nlm.nih.gov/spidey/spideyexec.html");
    description+=tr("<br><br> Wheelan SJ, Church DM, Ostell JM.");
    description+=tr("<br>Spidey: a tool for mRNA-to-genomic alignments");
    description+=tr("<br>Genome Res. 2001 Nov;11(11):1952-7.");
    versionRegExp=QRegExp("VersionDate: (\\d+\\/\\d+\\/\\d+)");
    toolKitName="spidey";

    connect(this, SIGNAL(si_toolValidationStatusChanged(bool)), SLOT(sl_validationStatusChanged(bool))); 
    

}

void SpideySupport::sl_validationStatusChanged( bool isValid )
{

    SplicedAlignmentTaskRegistry* registry = AppContext::getSplicedAlignmentTaskRegistry();
    if (isValid) 
    {
        if (!registry->hadRegistered(ET_SPIDEY)) {
            registry->registerTaskFactory(new SpideyAlignmentTaskFactory, ET_SPIDEY);
        }
    } else {
        registry->unregisterTaskFactory(ET_SPIDEY);
    }
}


////////////////////////////////////////
//SpideySupportContext

SpideySupportContext::SpideySupportContext(QObject* p) : GObjectViewWindowContext(p,  AnnotatedDNAViewFactory::ID) {

}

void SpideySupportContext::initViewContext(GObjectView* view) {
    AnnotatedDNAView* dnaView = qobject_cast<AnnotatedDNAView*>(view);
    assert(dnaView != NULL);
    if (dnaView->getSequenceInFocus() == NULL) {
            return;
    }

    // add spidey action
    QString spideyTitle = tr("Align sequence to mRNA");
    ADVGlobalAction* alignAction = new ADVGlobalAction(dnaView, QIcon(), spideyTitle, 1000*2000,
                                                       ADVGlobalActionFlags(ADVGlobalActionFlag_SingleSequenceOnly));
    alignAction->setObjectName("Align sequence to mRNA");
    addViewAction(alignAction);

    connect(alignAction, SIGNAL(triggered()), SLOT(sl_align_with_Spidey()));
}

void SpideySupportContext::buildMenu(GObjectView* view, QMenu* m) {
        QList<GObjectViewAction *> actions = getViewActions(view);
        QMenu* alignMenu = GUIUtils::findSubMenu(m, ADV_MENU_ALIGN);
        SAFE_POINT(alignMenu != NULL, "alignMenu", );
        foreach(GObjectViewAction* a, actions) {
                a->addToMenuWithOrder(alignMenu);
        }
}

void SpideySupportContext::sl_align_with_Spidey() {


    QWidget* parent = QApplication::activeWindow();

    //Check that Spidey and tempory directory path defined
    if (AppContext::getExternalToolRegistry()->getByName(ET_SPIDEY)->getPath().isEmpty()){
        QMessageBox msgBox(parent);
        msgBox.setWindowTitle(ET_SPIDEY);
        msgBox.setText(tr("Path for %1 tool not selected.").arg(ET_SPIDEY));
        msgBox.setInformativeText(tr("Do you want to select it now?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes ) {
            AppContext::getAppSettingsGUI()->showSettingsDialog(ExternalToolSupportSettingsPageId);
            return;
        }
    }

    ProjectTreeControllerModeSettings settings;
    settings.allowMultipleSelection = false;
    settings.objectTypesToShow.append(GObjectTypes::SEQUENCE);
    std::auto_ptr<U2SequenceObjectConstraints> seqConstraints(new U2SequenceObjectConstraints());
    seqConstraints->alphabetType = DNAAlphabet_NUCL;
    settings.objectConstraints.append(seqConstraints.get());

    QList<GObject*> objects = ProjectTreeItemSelectorDialog::selectObjects(settings,parent);
    if (objects.size() != 1) {
        return;
    }

    U2SequenceObject* rnaObj = qobject_cast<U2SequenceObject*>(objects.first());

    ADVGlobalAction* action = qobject_cast<ADVGlobalAction*>( sender() );
    assert(action != NULL);
    AnnotatedDNAView* dnaView = qobject_cast<AnnotatedDNAView*>(action->getObjectView());
    U2SequenceObject* dnaObj = dnaView->getSequenceInFocus()->getSequenceObject();

    if (rnaObj && dnaObj) {
        CreateAnnotationModel m;
        m.sequenceObjectRef = dnaObj;
        m.hideLocation = true;
        m.hideAnnotationName = true;
        m.groupName = "exon";
        CreateAnnotationDialog dlg(parent,m);
        dlg.setWindowTitle("Save result to annotation");
        if (dlg.exec() != QDialog::Accepted) {
            return;
        }

        SplicedAlignmentTaskConfig cfg(rnaObj,dnaObj);
        SpideySupportTask* spideyTask = new SpideySupportTask(cfg,m.getAnnotationObject());
        AppContext::getTaskScheduler()->registerTopLevelTask(spideyTask);

    }


}



}//namespace
