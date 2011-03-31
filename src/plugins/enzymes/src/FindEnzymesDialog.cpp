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

#include "FindEnzymesDialog.h"
#include "FindEnzymesTask.h"
#include "EnzymesIO.h"
#include <limits>

#include <U2Core/GObjectReference.h>
#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/Log.h>
#include <U2Core/Timer.h>

#include <U2Gui/CreateAnnotationWidgetController.h>
#include <U2Misc/DialogUtils.h>
#include <U2Gui/GUIUtils.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Formats/GenbankLocationParser.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtGui/QVBoxLayout>
#include <QtGui/QTreeWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>
#include <QtGui/QHeaderView>

//TODO: group by TYPE, ORGANIZM
//TODO: check whole group (tristate mode)

namespace U2 {


static Logger log(ULOG_ENZYME_PLUGIN);

QList<SEnzymeData>   EnzymesSelectorWidget::loadedEnzymes;
QSet<QString>        EnzymesSelectorWidget::lastSelection;

EnzymesSelectorWidget::EnzymesSelectorWidget() {
    setupUi(this);
    ignoreItemChecks = false;

    splitter->setStretchFactor(0, 5);
    splitter->setStretchFactor(1, 1);

    tree->setSortingEnabled(true);
    tree->sortByColumn(0, Qt::AscendingOrder);
    tree->setUniformRowHeights(true);
    tree->setColumnWidth(0, 110); //id
    tree->setColumnWidth(1, 75); //accession
    tree->setColumnWidth(2, 50); //type

    totalEnzymes = 0;
    minLength = 1;

    connect(enzymesFileButton, SIGNAL(clicked()), SLOT(sl_selectFile()));
    connect(selectAllButton, SIGNAL(clicked()), SLOT(sl_selectAll()));
    connect(selectNoneButton, SIGNAL(clicked()), SLOT(sl_selectNone()));
    connect(selectByLengthButton, SIGNAL(clicked()), SLOT(sl_selectByLength()));
    connect(invertSelectionButton, SIGNAL(clicked()), SLOT(sl_inverseSelection()));
    connect(saveSelectionButton, SIGNAL(clicked()), SLOT(sl_saveSelectionToFile()));
    connect(enzymeInfo, SIGNAL(clicked()), SLOT(sl_openDBPage()));

    if (loadedEnzymes.isEmpty()) {
        QString lastUsedFile = AppContext::getSettings()->getValue(DATA_FILE_KEY).toString();
        loadFile(lastUsedFile);
    } else {
        setEnzymesList(loadedEnzymes);
    }
}

EnzymesSelectorWidget::~EnzymesSelectorWidget() {
    saveSettings();
}

void EnzymesSelectorWidget::setupSettings() {
    QString dir = DialogUtils::getLastOpenFileDir(DATA_DIR_KEY);
    if (dir.isEmpty() || !QDir(dir).exists()) {
        dir = QDir::searchPaths( PATH_PREFIX_DATA ).first() + "/enzymes/";
        DialogUtils::setLastOpenFileDir(dir, DATA_DIR_KEY);
    }
    QString lastEnzFile = AppContext::getSettings()->getValue(DATA_FILE_KEY).toString();
    if (lastEnzFile.isEmpty() || !QFile::exists(lastEnzFile)) {
        lastEnzFile = dir + DEFAULT_ENZYMES_FILE;
        AppContext::getSettings()->setValue(DATA_FILE_KEY, lastEnzFile);
    }
    initSelection();
}

QList<SEnzymeData> EnzymesSelectorWidget::getSelectedEnzymes() {
    QList<SEnzymeData> selectedEnzymes;
    lastSelection.clear();
    for(int i=0, n = tree->topLevelItemCount(); i<n; i++){
        EnzymeGroupTreeItem* gi = static_cast<EnzymeGroupTreeItem*>(tree->topLevelItem(i));
        for (int j=0, m = gi->childCount(); j < m; j++) {
            EnzymeTreeItem* item = static_cast<EnzymeTreeItem*>(gi->child(j));
            if (item->checkState(0) == Qt::Checked) {
                selectedEnzymes.append(item->enzyme);
                lastSelection.insert(item->enzyme->id);
            }
        }
    }
    return selectedEnzymes;
}

QList<SEnzymeData> EnzymesSelectorWidget::getLoadedEnzymes() {
    if (loadedEnzymes.isEmpty()) {
        TaskStateInfo ti;
        QString lastUsedFile = AppContext::getSettings()->getValue(DATA_FILE_KEY).toString();
        loadedEnzymes = EnzymesIO::readEnzymes(lastUsedFile, ti);
    }
    return loadedEnzymes;
}

void EnzymesSelectorWidget::loadFile(const QString& url) {
    TaskStateInfo ti;
    QList<SEnzymeData> enzymes;

    if (!QFileInfo(url).exists()) {
        ti.setError(  tr("File not exists: %1").arg(url) );
    } else {
        GTIMER(c1,t1,"FindEnzymesDialog::loadFile [EnzymesIO::readEnzymes]");
        enzymes = EnzymesIO::readEnzymes(url, ti);
    }
    if (ti.hasErrors()) {
        if (isVisible()) {
            QMessageBox::critical(NULL, tr("Error"), ti.getError());
        } else  {
            log.error(ti.getError());
        }
        return;
    }
    if (!enzymes.isEmpty()) {
        if (AppContext::getSettings()->getValue(DATA_FILE_KEY).toString() != url) {
            lastSelection.clear();
        }
        AppContext::getSettings()->setValue(DATA_FILE_KEY, url);
    }

    setEnzymesList(enzymes);
}

void EnzymesSelectorWidget::saveFile(const QString& url) {
    TaskStateInfo ti;
    QString source = AppContext::getSettings()->getValue(DATA_FILE_KEY).toString();

    GTIMER(c1,t1,"FindEnzymesDialog::saveFile [EnzymesIO::writeEnzymes]");

    QSet<QString> enzymes;

    for(int i=0, n = tree->topLevelItemCount(); i<n; i++){
        EnzymeGroupTreeItem* gi = static_cast<EnzymeGroupTreeItem*>(tree->topLevelItem(i));
        for (int j=0, m = gi->childCount(); j < m; j++) {
            EnzymeTreeItem* item = static_cast<EnzymeTreeItem*>(gi->child(j));
            if (item->checkState(0) == Qt::Checked) {
                enzymes.insert(item->enzyme->id);
            }
        }
    }

    EnzymesIO::writeEnzymes(url, source, enzymes, ti);

    if (ti.hasErrors()) {
        if (isVisible()) {
            QMessageBox::critical(NULL, tr("Error"), ti.getError());
        } else  {
            log.error(ti.getError());
        }
        return;
    }
    if (QMessageBox::question(this, tr("New enzymes database has been saved."),
        tr("Do you want to work with new database?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            loadFile(url);
    }
}

void EnzymesSelectorWidget::setEnzymesList(const QList<SEnzymeData>& enzymes) {
    tree->setSortingEnabled(false);
    tree->disconnect(this);
    tree->clear();
    totalEnzymes = 0;

    GTIMER(c2,t2,"FindEnzymesDialog::loadFile [refill data tree]");
    foreach (const SEnzymeData& enz, enzymes) {
        EnzymeTreeItem* item = new EnzymeTreeItem(enz);
        if (lastSelection.contains(enz->id)) {
            item->setCheckState(0, Qt::Checked);
        }
        totalEnzymes++;
        EnzymeGroupTreeItem* gi = findGroupItem(enz->id.isEmpty() ? QString(" ") : enz->id.left(1), true);
        gi->addChild(item);

    }
    for(int i=0, n = tree->topLevelItemCount(); i<n; i++) {
        EnzymeGroupTreeItem* gi = static_cast<EnzymeGroupTreeItem*>(tree->topLevelItem(i));
        gi->updateVisual();
    }
    if (tree->topLevelItemCount() > 0 && tree->topLevelItem(0)->childCount() < 10) {
        tree->topLevelItem(0)->setExpanded(true);
    }
    t2.stop();

    GTIMER(c3,t3,"FindEnzymesDialog::loadFile [sort tree]");
    tree->setSortingEnabled(true);
    t3.stop();

    connect(tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(sl_itemChanged(QTreeWidgetItem*, int)));


    //     GTIMER(c4,t4,"FindEnzymesDialog::loadFile [resize tree]");
    //     tree->header()->resizeSections(QHeaderView::ResizeToContents);
    //     t4.stop();

    updateStatus();
    loadedEnzymes = enzymes;
}

EnzymeGroupTreeItem* EnzymesSelectorWidget::findGroupItem(const QString& s, bool create) {
    for(int i=0, n = tree->topLevelItemCount(); i<n; i++) {
        EnzymeGroupTreeItem* gi = static_cast<EnzymeGroupTreeItem*>(tree->topLevelItem(i));
        if (gi->s == s) {
            return gi;
        }
    }
    if (create) {
        EnzymeGroupTreeItem* gi = new EnzymeGroupTreeItem(s);
        tree->addTopLevelItem(gi);
        return gi;
    }
    return NULL;
}

void EnzymesSelectorWidget::updateStatus() {
    int nChecked = 0;
    QStringList checkedNamesList;
    for(int i=0, n = tree->topLevelItemCount(); i<n; i++){
        EnzymeGroupTreeItem* gi = static_cast<EnzymeGroupTreeItem*>(tree->topLevelItem(i));
        nChecked+= gi->checkedEnzymes.size();
        foreach(const EnzymeTreeItem* ci, gi->checkedEnzymes) {
            checkedNamesList.append(ci->enzyme->id);
        }
    }
    checkedNamesList.sort();
    checkedEnzymesEdit->setPlainText(checkedNamesList.join(","));

    emit si_selectionModified(totalEnzymes, nChecked);
}

void EnzymesSelectorWidget::sl_selectFile() {
    LastOpenDirHelper dir(DATA_DIR_KEY);
    dir.url = QFileDialog::getOpenFileName(this, tr("Select enzyme database file"), dir.dir, EnzymesIO::getFileDialogFilter());
    if (!dir.url.isEmpty()) {
        loadFile(dir.url);
    }
}

void EnzymesSelectorWidget::sl_selectAll() {
    ignoreItemChecks =  true;
    for(int i=0, n = tree->topLevelItemCount(); i<n; i++){
        EnzymeGroupTreeItem* gi = static_cast<EnzymeGroupTreeItem*>(tree->topLevelItem(i));
        for (int j=0, m = gi->childCount(); j < m; j++) {
            EnzymeTreeItem* item = static_cast<EnzymeTreeItem*>(gi->child(j));
            item->setCheckState(0, Qt::Checked);
        }
        gi->updateVisual();
    }
    ignoreItemChecks = false;
    updateStatus();
}

void EnzymesSelectorWidget::sl_selectNone() {
    ignoreItemChecks = true;
    for(int i=0, n = tree->topLevelItemCount(); i<n; i++){
        EnzymeGroupTreeItem* gi = static_cast<EnzymeGroupTreeItem*>(tree->topLevelItem(i));
        for (int j=0, m = gi->childCount(); j < m; j++) {
            EnzymeTreeItem* item = static_cast<EnzymeTreeItem*>(gi->child(j));
            item->setCheckState(0, Qt::Unchecked);
        }
        gi->updateVisual();
    }
    ignoreItemChecks = false;
    updateStatus();
}

void EnzymesSelectorWidget::sl_selectByLength() {
    bool ok;
    int len = QInputDialog::getInt(this, tr("Minimum length"), tr("Enter minimum length of recognition sites"), minLength, 1, 20, 1, &ok);
    if (ok) {
        minLength = len;
        ignoreItemChecks = true;
        for(int i = 0, n = tree->topLevelItemCount(); i < n; i++){
            EnzymeGroupTreeItem* gi = static_cast<EnzymeGroupTreeItem*>(tree->topLevelItem(i));
            for (int j=0, m = gi->childCount(); j < m; j++) {
                EnzymeTreeItem* item = static_cast<EnzymeTreeItem*>(gi->child(j));
                if (item->enzyme->seq.length() < len) {
                    item->setCheckState(0, Qt::Unchecked);
                } else {
                    item->setCheckState(0, Qt::Checked);
                }
            }
            gi->updateVisual();
        }
        ignoreItemChecks = false;
    }
    updateStatus();
}

void EnzymesSelectorWidget::sl_inverseSelection() {
    ignoreItemChecks = true;
    for(int i=0, n = tree->topLevelItemCount(); i<n; i++){
        EnzymeGroupTreeItem* gi = static_cast<EnzymeGroupTreeItem*>(tree->topLevelItem(i));
        for (int j=0, m = gi->childCount(); j < m; j++) {
            EnzymeTreeItem* item = static_cast<EnzymeTreeItem*>(gi->child(j));
            Qt::CheckState oldState = item->checkState(0);
            item->setCheckState(0, oldState == Qt::Checked ? Qt::Unchecked : Qt::Checked);
        }
        gi->updateVisual();
    }
    ignoreItemChecks = false;
    updateStatus();
}

void EnzymesSelectorWidget::sl_saveSelectionToFile() {
    LastOpenDirHelper dir(DATA_DIR_KEY);
    dir.url = QFileDialog::getSaveFileName(this, tr("Select enzyme database file"), dir.dir, EnzymesIO::getFileDialogFilter());
    if (!dir.url.isEmpty()) {
        saveFile(dir.url);
    }
    updateStatus();
}

void EnzymesSelectorWidget::sl_openDBPage() {
    QTreeWidgetItem* ci = tree->currentItem();
    EnzymeTreeItem* item = ci == NULL || ci->parent() == 0 ? NULL : static_cast<EnzymeTreeItem*>(tree->currentItem());
    if (item == NULL) {
        QMessageBox::critical(this, tr("Error!"), tr("No enzyme selected!"));
        return;
    }
    QString suffix = item->enzyme->accession;
    if (suffix.isEmpty()){
        suffix = item->enzyme->id;
    } else if (suffix.startsWith("RB")) {
        suffix = suffix.mid(2);

    }
    GUIUtils::runWebBrowser("http://rebase.neb.com/cgi-bin/reb_get.pl?enzname="+suffix);
}

void EnzymesSelectorWidget::sl_itemChanged(QTreeWidgetItem* item, int col) {
    if (item->parent() == NULL  || col != 0 || ignoreItemChecks) {
        return;
    }
    EnzymeTreeItem* ei = static_cast<EnzymeTreeItem*>(item);
    EnzymeGroupTreeItem* gi = static_cast<EnzymeGroupTreeItem*>(ei->parent());
    gi->updateVisual();
    updateStatus();
}

int EnzymesSelectorWidget::getNumSelected()
{
    int nChecked = 0;
    for(int i=0, n = tree->topLevelItemCount(); i<n; i++){
        EnzymeGroupTreeItem* gi = static_cast<EnzymeGroupTreeItem*>(tree->topLevelItem(i));
        nChecked+= gi->checkedEnzymes.size();
    }
    return nChecked;
}

void EnzymesSelectorWidget::saveSettings()
{
    QStringList sl(lastSelection.toList());
    AppContext::getSettings()->setValue(LAST_SELECTION, sl.join(SEP));
}

void EnzymesSelectorWidget::initSelection()
{
    QString selStr = AppContext::getSettings()->getValue(LAST_SELECTION).toString();
    if (selStr.isEmpty()) {
        selStr = COMMON_ENZYMES;
    }
    lastSelection = selStr.split(SEP).toSet();
}

FindEnzymesDialog::FindEnzymesDialog(ADVSequenceObjectContext* sctx)
: QDialog(sctx->getAnnotatedDNAView()->getWidget()), seqCtx(sctx) {
    setupUi(this);

    circularButton->setChecked(seqCtx->getSequenceObject()->isCircular());

    maxHitSB->setMaximum(INT_MAX);
    minHitSB->setMaximum(INT_MAX);

    maxHitSB->setMinimum(ANY_VALUE);
    minHitSB->setMinimum(ANY_VALUE);

    sbRangeStart->setMaximum( seqCtx->getSequenceLen() );
    sbRangeEnd->setMaximum( seqCtx->getSequenceLen() );

    initSettings();

    QVBoxLayout* vl = new QVBoxLayout();
    enzSel = new EnzymesSelectorWidget();
    vl->setMargin(0);
    vl->addWidget(enzSel);
    enzymesSelectorWidget->setLayout(vl);
    enzymesSelectorWidget->setMinimumSize(enzSel->size());

    connect(enzSel, SIGNAL(si_selectionModified(int,int)), SLOT(sl_onSelectionModified(int,int)));
    connect(fillRangeButton, SIGNAL(clicked()), SLOT(sl_onFillRangeButtonClicked()));
    sl_onSelectionModified( enzSel->getTotalNumber(),enzSel->getNumSelected());
    
}

#define MAX_ENZYMES_TO_FIND 100*1000

void FindEnzymesDialog::sl_onSelectionModified(int total, int nChecked) {
    statusLabel->setText(tr("Total number of enzymes: %1, selected %2").arg(total).arg(nChecked));
}

void FindEnzymesDialog::accept() {
    QList<SEnzymeData> selectedEnzymes = enzSel->getSelectedEnzymes();
    
    if (selectedEnzymes.isEmpty()) {
        QMessageBox::critical(this, tr("Error!"), tr("No enzymes selected!"));
        return;
    }
    
    int maxHitVal = maxHitSB->value(), minHitVal = minHitSB->value();
    if(maxHitVal == ANY_VALUE){
        maxHitVal = INT_MAX;
    }
    if (minHitVal == ANY_VALUE){
        minHitVal = 1;
    }

    if (minHitVal > maxHitVal) {
        QMessageBox::critical(this, tr("Error!"), tr("Minimum hit value must be lesser or equal then maximum!"));
        return;
    }
    
    saveSettings();
    
    
    AppContext::getAutoAnnotationsSupport()->updateAnnotationsByGroup(ANNOTATION_GROUP_ENZYME);
   

    QDialog::accept();
}


void FindEnzymesDialog::initSettings()
{
    EnzymesSelectorWidget::initSelection();
    bool useHitCountControl = AppContext::getSettings()->getValue(ENABLE_HIT_COUNT, false).toBool();
    int minHitValue = AppContext::getSettings()->getValue(MIN_HIT_VALUE, 1).toInt();
    int maxHitValue = AppContext::getSettings()->getValue(MAX_HIT_VALUE, 2).toInt();
    
    QString exludedRegionStr = AppContext::getSettings()->getValue(NON_CUT_REGION, "").toString();
    bool excludeRegionOn = false;
    if (!exludedRegionStr.isEmpty()) {
        U2Location location;
        Genbank::LocationParser::parseLocation(qPrintable(exludedRegionStr),exludedRegionStr.length(), location);
        if (!location->isEmpty()) {
            excludeRegionOn = true;
            const U2Region& range = location->regions.first();
            sbRangeStart->setValue(range.startPos + 1);
            sbRangeEnd->setValue(range.endPos());
        }
    }
    
    excludeRegionBox->setChecked(excludeRegionOn);

    filterGroupBox->setChecked(useHitCountControl);
    if (useHitCountControl) {
        minHitSB->setValue(minHitValue);
        maxHitSB->setValue(maxHitValue);
    } else {
        minHitSB->setValue(1);
        maxHitSB->setValue(2);
    }
}

void FindEnzymesDialog::saveSettings()
{
    AppContext::getSettings()->setValue(ENABLE_HIT_COUNT, filterGroupBox->isChecked());
    if (filterGroupBox->isChecked()) {
        AppContext::getSettings()->setValue(MIN_HIT_VALUE, minHitSB->value());
        AppContext::getSettings()->setValue(MAX_HIT_VALUE, maxHitSB->value());
    } else {
        AppContext::getSettings()->setValue(MIN_HIT_VALUE, 1);
        AppContext::getSettings()->setValue(MAX_HIT_VALUE, INT_MAX);
    }

    QVector<U2Region> range;
    if (excludeRegionBox->isChecked()) {
        U2Region r;
        r.startPos = qMin(sbRangeStart->value(), sbRangeEnd->value()) - 1;
        r.length = qMax(sbRangeStart->value(), sbRangeEnd->value()) - r.startPos;
        if (r.length != 0) {
            range.append(r);
        }
    }
    AppContext::getSettings()->setValue(NON_CUT_REGION, QVariant::fromValue(range) );
    
    enzSel->saveSettings();

}

void FindEnzymesDialog::initDefaultSettings()
{
    AppContext::getSettings()->setValue(NON_CUT_REGION, "" );
}

void FindEnzymesDialog::sl_onFillRangeButtonClicked()
{
    DNASequenceSelection* selection = seqCtx->getSequenceSelection();
    if (!selection->isEmpty()) {
        const U2Region& range = selection->getSelectedRegions().first();
        sbRangeStart->setValue(range.startPos + 1);
        sbRangeEnd->setValue(range.endPos());
    }
}


//////////////////////////////////////////////////////////////////////////
// Tree item
EnzymeTreeItem::EnzymeTreeItem(const SEnzymeData& ed) 
: enzyme(ed)
{
    setText(0, enzyme->id);  
    setCheckState(0, Qt::Unchecked);
    setText(1, enzyme->accession);
    setText(2, enzyme->type);
    setText(3, enzyme->seq);
    setData(3, Qt::ToolTipRole, enzyme->seq);
    setText(4, enzyme->organizm);//todo: show cut sites
    setData(4, Qt::ToolTipRole, enzyme->organizm);
}

bool EnzymeTreeItem::operator<(const QTreeWidgetItem & other) const {
    int col = treeWidget()->sortColumn();
    const EnzymeTreeItem& ei = (const EnzymeTreeItem&)other;
    if (col == 0) {
        bool eq = enzyme->id == ei.enzyme->id;
        if (!eq) {
            return enzyme->id < ei.enzyme->id;
        }
        return this < &ei;
    } 
    return text(col) < ei.text(col);
}

EnzymeGroupTreeItem::EnzymeGroupTreeItem(const QString& _s) : s(_s){
    updateVisual();
}

void EnzymeGroupTreeItem::updateVisual() {
    int numChilds = childCount();
    checkedEnzymes.clear();
    for (int i=0; i < numChilds; i++) {
        EnzymeTreeItem* item = static_cast<EnzymeTreeItem*>(child(i));
        if (item->checkState(0) == Qt::Checked) {
            checkedEnzymes.insert(item);
        }
    }
    QString text0 = s + " (" + QString::number(checkedEnzymes.size()) + ", " + QString::number(numChilds) + ")";
    setText(0, text0);
    
    if (numChilds > 0) {
        QString text4 = (static_cast<EnzymeTreeItem*>(child(0)))->enzyme->id;
        if (childCount() > 1) {
            text4+=" .. "+(static_cast<EnzymeTreeItem*>(child(numChilds-1)))->enzyme->id;
        }
        setText(4, text4);
    }
}

bool EnzymeGroupTreeItem::operator<(const QTreeWidgetItem & other) const {
    if (other.parent() != NULL) {
        return true;
    }
    int col = treeWidget()->sortColumn();
    return text(col) <other.text(col);
}

} //namespace
