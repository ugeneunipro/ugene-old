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

#include <QtCore/QTextStream>

#include <QtGui/QClipboard>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QDialog>
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QMessageBox>
#endif

#include <U2Algorithm/CreateSubalignmentTask.h>

#include <U2Core/AddSequencesToAlignmentTask.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/MAlignment.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/MsaDbiUtils.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/Settings.h>
#include <U2Core/Task.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Gui/OptionsPanel.h>
#include <U2Gui/OPWidgetFactory.h>

#include <U2Formats/DocumentFormatUtils.h>

#include <U2Gui/AppSettingsGUI.h>
#include <U2Gui/DialogUtils.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/PositionSelector.h>
#include <U2Gui/ProjectTreeController.h>
#include <U2Gui/ProjectTreeItemSelectorDialog.h>

#include "ColorSchemaSettingsController.h"
#include "CreateSubalignmentDialogController.h"
#include "Highlighting/MSAHighlightingTabFactory.h"
#include "MSAColorScheme.h"
#include "MSAEditor.h"
#include "MSAEditorNameList.h"
#include "MSAEditorSequenceArea.h"


namespace U2 {

/* TRANSLATOR U2::MSAEditor */

#define SETTINGS_ROOT QString("msaeditor/")
#define SETTINGS_COLOR_NUCL     "color_nucl"
#define SETTINGS_COLOR_AMINO    "color_amino"
#define SETTINGS_HIGHGHLIGHT_NUCL      "highghlight_nucl"
#define SETTINGS_HIGHGHLIGHT_AMINO     "highghligh_amino"

MSAEditorSequenceArea::MSAEditorSequenceArea(MSAEditorUI* _ui, GScrollBar* hb, GScrollBar* vb)
    : editor(_ui->editor), ui(_ui), shBar(hb), svBar(vb), prevPressedButton(Qt::NoButton),
    changeTracker(editor->getMSAObject()->getEntityRef())
{
    setObjectName("msa_editor_sequence_area");
    setFocusPolicy(Qt::WheelFocus);

    cachedView = new QPixmap();

    completeRedraw = true;

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setMinimumSize(100, 100);
    startPos = 0;
    startSeq = 0;
    highlightSelection = false;
    selecting = false;
    shifting = false;
    shiftingWasPerformed = false;

    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

    delSelectionAction = new QAction(tr("Remove selection"), this);
    delSelectionAction->setObjectName("Remove selection");
    delSelectionAction->setShortcutContext(Qt::WidgetShortcut);
    connect(delSelectionAction, SIGNAL(triggered()), SLOT(sl_delCurrentSelection()));

    connect(ui->getCopySelectionAction(), SIGNAL(triggered()), SLOT(sl_copyCurrentSelection()));
    addAction(ui->getCopySelectionAction());

    delColAction = new QAction(QIcon(":core/images/msaed_remove_columns_with_gaps.png"), tr("Remove columns of gaps..."), this);
    delColAction->setObjectName("remove_columns_of_gaps");
    delColAction->setShortcut(QKeySequence(Qt::SHIFT| Qt::Key_Delete));
    delColAction->setShortcutContext(Qt::WidgetShortcut);
    connect(delColAction, SIGNAL(triggered()), SLOT(sl_delCol()));

    insSymAction = new QAction(tr("Fill selection with gaps"), this);
    insSymAction->setObjectName("fill_selection_with_gaps");
    insSymAction->setShortcutContext(Qt::WidgetShortcut);
    connect(insSymAction, SIGNAL(triggered()), SLOT(sl_fillCurrentSelectionWithGaps()));
    addAction(insSymAction);


    createSubaligniment = new QAction(tr("Save subalignment"), this);
    createSubaligniment->setObjectName("Save subalignment");
    createSubaligniment->setShortcutContext(Qt::WidgetShortcut);
    connect(createSubaligniment, SIGNAL(triggered()), SLOT(sl_createSubaligniment()));

    saveSequence = new QAction(tr("Save sequence"), this);
    saveSequence->setObjectName("Save sequence");
    saveSequence->setShortcutContext(Qt::WidgetShortcut);
    connect(saveSequence, SIGNAL(triggered()), SLOT(sl_saveSequence()));

    gotoAction = new QAction(QIcon(":core/images/goto.png"), tr("Go to position..."), this);
    gotoAction->setObjectName("action_go_to_position");
    gotoAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
    gotoAction->setShortcutContext(Qt::WindowShortcut);
    gotoAction->setToolTip(QString("%1 (%2)").arg(gotoAction->text()).arg(gotoAction->shortcut().toString()));
    connect(gotoAction, SIGNAL(triggered()), SLOT(sl_goto()));

    removeAllGapsAction = new QAction(QIcon(":core/images/msaed_remove_all_gaps.png"), tr("Remove all gaps"), this);
    removeAllGapsAction->setObjectName("Remove all gaps");
    connect(removeAllGapsAction, SIGNAL(triggered()), SLOT(sl_removeAllGaps()));

    addSeqFromFileAction = new QAction(tr("Sequence from file..."), this);
    addSeqFromFileAction->setObjectName("Sequence from file");
    connect(addSeqFromFileAction, SIGNAL(triggered()), SLOT(sl_addSeqFromFile()));

    addSeqFromProjectAction = new QAction(tr("Sequence from current project..."), this);
    addSeqFromProjectAction->setObjectName("Sequence from current project");
    connect(addSeqFromProjectAction, SIGNAL(triggered()), SLOT(sl_addSeqFromProject()));

    sortByNameAction = new QAction(tr("Sort sequences by name"), this);
    sortByNameAction->setObjectName("action_sort_by_name");
    connect(sortByNameAction, SIGNAL(triggered()), SLOT(sl_sortByName()));

    collapseModeSwitchAction = new QAction(QIcon(":core/images/collapse.png"), tr("Switch on/off collapsing"), this);
    collapseModeSwitchAction->setObjectName("Enable collapsing");
    collapseModeSwitchAction->setCheckable(true);
    connect(collapseModeSwitchAction, SIGNAL(toggled(bool)), SLOT(sl_setCollapsingMode(bool)));

    collapseModeUpdateAction = new QAction(QIcon(":core/images/collapse_update.png"), tr("Update collapsed groups"), this);
    collapseModeUpdateAction->setObjectName("Update collapsed groups");
    collapseModeUpdateAction->setEnabled(false);
    connect(collapseModeUpdateAction, SIGNAL(triggered()), SLOT(sl_updateCollapsingMode()));

    reverseComplementAction = new QAction(tr("Replace selected rows with reverse-complement"), this);
    reverseComplementAction->setObjectName("replace_selected_rows_with_reverse-complement");
    connect(reverseComplementAction, SIGNAL(triggered()), SLOT(sl_reverseComplementCurrentSelection()));

    reverseAction = new QAction(tr("Replace selected rows with reverse"), this);
    reverseAction->setObjectName("replace_selected_rows_with_reverse");
    connect(reverseAction, SIGNAL(triggered()), SLOT(sl_reverseCurrentSelection()));

    complementAction = new QAction(tr("Replace selected rows with complement"), this);
    complementAction->setObjectName("replace_selected_rows_with_complement");
    connect(complementAction, SIGNAL(triggered()), SLOT(sl_complementCurrentSelection()));

    connect(editor->getMSAObject(), SIGNAL(si_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)),
        SLOT(sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)));
    connect(editor->getMSAObject(), SIGNAL(si_lockedStateChanged()), SLOT(sl_lockedStateChanged()));

    connect(editor, SIGNAL(si_buildStaticMenu(GObjectView*, QMenu*)), SLOT(sl_buildStaticMenu(GObjectView*, QMenu*)));
    connect(editor, SIGNAL(si_buildStaticToolbar(GObjectView*, QToolBar*)), SLOT(sl_buildStaticToolbar(GObjectView*, QToolBar*)));
    connect(editor, SIGNAL(si_buildPopupMenu(GObjectView* , QMenu*)), SLOT(sl_buildContextMenu(GObjectView*, QMenu*)));
    connect(editor, SIGNAL(si_zoomOperationPerformed(bool)), SLOT(sl_zoomOperationPerformed(bool)));
    connect(editor, SIGNAL(si_fontChanged(QFont)), SLOT(sl_fontChanged(QFont)));
    connect(ui->getCollapseModel(), SIGNAL(toggled()), SLOT(sl_modelChanged()));
    connect(editor, SIGNAL(si_referenceSeqChanged(qint64)), SLOT(sl_referenceSeqChanged(qint64)));

    QAction* undoAction = ui->getUndoAction();
    QAction* redoAction = ui->getRedoAction();
    addAction(undoAction);
    addAction(redoAction);
    connect(undoAction, SIGNAL(triggered()), SLOT(sl_resetCollapsibleModel()));
    connect(redoAction, SIGNAL(triggered()), SLOT(sl_resetCollapsibleModel()));

    prepareColorSchemeMenuActions();
    prepareHighlightingMenuActions();
    updateActions();
}

MSAEditorSequenceArea::~MSAEditorSequenceArea() {
    delete cachedView;
    deleteOldCustomSchemes();
}

void MSAEditorSequenceArea::prepareColorSchemeMenuActions() {
    Settings* s = AppContext::getSettings();
    if (!s || !editor){
        return;
    }
    MAlignmentObject* maObj = editor->getMSAObject();
    if (!maObj){
        return;
    }

    const DNAAlphabet* al = maObj->getAlphabet();
    if (!al){
        return;
    }
    DNAAlphabetType atype = al->getType();
    MSAColorSchemeRegistry* csr = AppContext::getMSAColorSchemeRegistry();
    connect(csr, SIGNAL(si_customSettingsChanged()), SLOT(sl_customColorSettingsChanged()));

    QString csid = atype == DNAAlphabet_AMINO ?
            s->getValue(SETTINGS_ROOT + SETTINGS_COLOR_AMINO, MSAColorScheme::UGENE_AMINO).toString()
          : s->getValue(SETTINGS_ROOT + SETTINGS_COLOR_NUCL, MSAColorScheme::UGENE_NUCL).toString();

    MSAColorSchemeFactory* csf = csr->getMSAColorSchemeFactoryById(csid);
    if (csf == NULL) {
        csf = csr->getMSAColorSchemeFactoryById(atype == DNAAlphabet_AMINO ? MSAColorScheme::UGENE_AMINO : MSAColorScheme::UGENE_NUCL);
    }
    assert(csf!=NULL);
    colorScheme = csf->create(this, maObj);

    QList<MSAColorSchemeFactory*> factories= csr->getMSAColorSchemes(atype);
    foreach(MSAColorSchemeFactory* f, factories) {
        QAction* action = new QAction(f->getName(), this);
        action->setObjectName(f->getName());
        action->setCheckable(true);
        action->setChecked(f == csf);
        action->setData(f->getId());
        connect(action, SIGNAL(triggered()), SLOT(sl_changeColorScheme()));
        colorSchemeMenuActions.append(action);
    }
    initCustomSchemeActions(csf->getId(), atype);
}

void MSAEditorSequenceArea::prepareHighlightingMenuActions() {
    Settings* s = AppContext::getSettings();
    MAlignmentObject* maObj = editor->getMSAObject();
    if (maObj == NULL)
        return;

    DNAAlphabetType atype = maObj->getAlphabet()->getType();
    MSAHighlightingSchemeRegistry* hsr = AppContext::getMSAHighlightingSchemeRegistry();

    QString hsid = atype == DNAAlphabet_AMINO ?
        s->getValue(SETTINGS_ROOT + SETTINGS_HIGHGHLIGHT_AMINO, MSAHighlightingScheme::EMPTY_AMINO).toString()
        : s->getValue(SETTINGS_ROOT + SETTINGS_HIGHGHLIGHT_NUCL, MSAHighlightingScheme::EMPTY_NUCL).toString();
    MSAHighlightingSchemeFactory* hsf = hsr->getMSAHighlightingSchemeFactoryById(hsid);
    if (hsf == NULL) {
        hsf = hsr->getMSAHighlightingSchemeFactoryById(atype == DNAAlphabet_AMINO ? MSAHighlightingScheme::EMPTY_AMINO : MSAHighlightingScheme::EMPTY_NUCL);
    }
    assert(hsf!=NULL);
    highlightingScheme = hsf->create(this, maObj);

    QList<MSAHighlightingSchemeFactory*> factories= hsr->getMSAHighlightingSchemes(atype);
    foreach(MSAHighlightingSchemeFactory* f, factories) {
        QAction* action = new QAction(f->getName(), this);
        action->setObjectName(f->getName());
        action->setCheckable(true);
        action->setChecked(f == hsf);
        action->setData(f->getId());
        connect(action, SIGNAL(triggered()), SLOT(sl_changeHighlightScheme()));
        highlightingSchemeMenuActions.append(action);
    }
    useDotsAction = new QAction(QString("Use dots"), this);
    useDotsAction->setCheckable(true);
    useDotsAction->setChecked(false);
    connect(useDotsAction, SIGNAL(triggered()), SLOT(sl_useDots()));
}

void MSAEditorSequenceArea::initCustomSchemeActions(const QString& id, DNAAlphabetType atype){
    deleteOldCustomSchemes();

    MSAColorSchemeRegistry* csr = AppContext::getMSAColorSchemeRegistry();

    QList<MSAColorSchemeFactory*> customFactories = csr->getMSACustomColorSchemes(atype);
    foreach(MSAColorSchemeFactory* f, customFactories) {
        QAction* action = new QAction(f->getName(), NULL);
        action->setObjectName(f->getName());
        action->setCheckable(true);
        action->setChecked(id == f->getId());
        action->setData(f->getId());
        connect(action, SIGNAL(triggered()), SLOT(sl_changeColorScheme()));
        customColorSchemeMenuActions.append(action);
    }
}

QStringList MSAEditorSequenceArea::getAvailableColorSchemes() const{
    QStringList allSchemas;
    foreach(QAction *a, colorSchemeMenuActions){
        allSchemas.append(a->text());
    }

    foreach(QAction *a, customColorSchemeMenuActions){
        allSchemas.append(a->text());
    }
    return allSchemas;
}

QStringList MSAEditorSequenceArea::getAvailableHighlightingSchemes() const{
    QStringList allSchemas;
    foreach(QAction *a, highlightingSchemeMenuActions){
        allSchemas.append(a->text());
    }
    return allSchemas;
}

bool MSAEditorSequenceArea::hasAminoAlphabet() {
    MAlignmentObject* maObj = editor->getMSAObject();
    SAFE_POINT(NULL != maObj, tr("MAlignmentObject is null in MSAEditorSequenceArea::hasAminoAlphabet()"), false);
    const DNAAlphabet* alphabet = maObj->getAlphabet();
    SAFE_POINT(NULL != maObj, tr("DNAAlphabet is null in MSAEditorSequenceArea::hasAminoAlphabet()"), false);
    return DNAAlphabet_AMINO == alphabet->getType();
}

bool MSAEditorSequenceArea::paintAllToPixmap(QPixmap &pixmap) {
    CHECK( editor->getColumnWidth() * editor->getAlignmentLen() < 32768 &&
           editor->getRowHeight() * editor->getNumSequences() < 32768, false);

    pixmap = QPixmap( editor->getColumnWidth() * editor->getAlignmentLen(),
                      editor->getRowHeight() * editor->getNumSequences());
    QPainter p(&pixmap);
    drawContent(p, QRect(0, 0, editor->getAlignmentLen(), editor->getNumSequences()));
    return true;
}

bool MSAEditorSequenceArea::paintToPixmap(QPixmap &pixmap,
                                          const U2Region &region,
                                          const QList<qint64> &seqIdx) {
    CHECK( !region.isEmpty(), false);
    CHECK( !seqIdx.isEmpty(), false);

    CHECK( editor->getColumnWidth() * region.length < 32768 &&
           editor->getRowHeight() * seqIdx.size() < 32768, false);
    pixmap = QPixmap(editor->getColumnWidth() * region.length,
                     editor->getRowHeight() * seqIdx.size());
    QPainter p(&pixmap);
    drawContent(p, region, seqIdx);
    return true;
}

void MSAEditorSequenceArea::sl_customColorSettingsChanged(){
    DNAAlphabetType atype = editor->getMSAObject()->getAlphabet()->getType();
    QAction* a = GUIUtils::getCheckedAction(customColorSchemeMenuActions);
    initCustomSchemeActions(a == NULL ? "" :a->data().toString(), atype);
    if(a != NULL && !colorSchemeMenuActions.contains(a)){
        bool containsActionByName = false;
        foreach(QAction *customAction, customColorSchemeMenuActions){
            if(customAction->objectName() != "" && customAction->objectName() == prevSchemeName){
                containsActionByName = true;
                break;
            }
        }
        if (!containsActionByName){
            colorSchemeMenuActions.first()->setChecked(true);
            colorSchemeMenuActions.first()->trigger();
            return;
        }
    }
    if(!a){
        QAction* a = GUIUtils::findActionByData(QList<QAction*>() << colorSchemeMenuActions << customColorSchemeMenuActions, atype == DNAAlphabet_AMINO ? MSAColorScheme::UGENE_AMINO : MSAColorScheme::UGENE_NUCL);
        if (a != NULL) {
            a->setChecked(true);
        }
    }

    emit si_highlightingChanged();
    sl_changeColorScheme();
}

void MSAEditorSequenceArea::deleteOldCustomSchemes(){
    foreach(QAction* a, customColorSchemeMenuActions){
        delete a;
    }
    customColorSchemeMenuActions.clear();
}

void MSAEditorSequenceArea::sl_changeColorSchemeOutside( const QString &name ){
    QAction* a = GUIUtils::findAction(QList<QAction*>() << colorSchemeMenuActions << customColorSchemeMenuActions << highlightingSchemeMenuActions, name);
    if (a != NULL) {
        a->trigger();
    }
}

void MSAEditorSequenceArea::sl_doUseDots(){
    useDotsAction->trigger();
}

void MSAEditorSequenceArea::sl_useDots(){
    completeRedraw = true;
    update();
    emit si_highlightingChanged();
}

void MSAEditorSequenceArea::sl_changeColorScheme() {
    QAction* a = qobject_cast<QAction*>(sender());
    if(!a){a = GUIUtils::getCheckedAction(customColorSchemeMenuActions);}
    if(!a){return;}

    QString id = a->data().toString();
    MSAColorSchemeFactory* f = AppContext::getMSAColorSchemeRegistry()->getMSAColorSchemeFactoryById(id);
    delete colorScheme;
    if (ui->editor->getMSAObject() == NULL)
        return;

    colorScheme = f->create(this, ui->editor->getMSAObject());
    QList<QAction*> tmpActions; tmpActions << colorSchemeMenuActions << customColorSchemeMenuActions;
    foreach(QAction* action, tmpActions) {
        action->setChecked(action == a);
    }
    if (f->getAlphabetType() == DNAAlphabet_AMINO) {
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_COLOR_AMINO, id);
    } else {
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_COLOR_NUCL, id);
    }

    prevSchemeName = a->objectName();
    completeRedraw = true;
    update();
    emit si_highlightingChanged();
}

void MSAEditorSequenceArea::sl_changeHighlightScheme(){
    QAction* a = qobject_cast<QAction*>(sender());
    if(!a){a = GUIUtils::getCheckedAction(customColorSchemeMenuActions);}
    if(!a){return;}

    QString id = a->data().toString();
    MSAHighlightingSchemeFactory* f = AppContext::getMSAHighlightingSchemeRegistry()->getMSAHighlightingSchemeFactoryById(id);
    delete highlightingScheme;
    if (ui->editor->getMSAObject() == NULL)
        return;

    highlightingScheme = f->create(this, ui->editor->getMSAObject());

    const MAlignment msa = ui->editor->getMSAObject()->getMAlignment();

    U2OpStatusImpl os;
    const int refSeq = msa.getRowIndexByRowId(editor->getReferenceRowId(), os);

    MSAHighlightingFactory msaHighlightingFactory;
    QString msaHighlightingId = msaHighlightingFactory.getOPGroupParameters().getGroupId();

    CHECK(ui->getEditor(), );
    CHECK(ui->getEditor()->getOptionsPanel(), );

    if(!f->isRefFree() && refSeq == -1 && ui->getEditor()->getOptionsPanel()->getActiveGroupId() != msaHighlightingId) {
        QMessageBox::warning(ui, tr("No reference sequence selected"),
            tr("Reference sequence for current highlighting scheme is not selected.\r\nUse context menu or Highlighting tab on Options panel to select it"));
    }

    foreach(QAction* action, highlightingSchemeMenuActions) {
        action->setChecked(action == a);
    }
    if (f->getAlphabetType() == DNAAlphabet_AMINO) {
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_HIGHGHLIGHT_AMINO, id);
    } else {
        AppContext::getSettings()->setValue(SETTINGS_ROOT + SETTINGS_HIGHGHLIGHT_NUCL, id);
    }

    completeRedraw = true;
    update();
    emit si_highlightingChanged();
}

void MSAEditorSequenceArea::updateActions() {
    MAlignmentObject* maObj = editor->getMSAObject();
    assert(maObj != NULL);
    bool readOnly = maObj->isStateLocked();

    delSelectionAction->setEnabled(!readOnly && !isAlignmentEmpty());
    delColAction->setEnabled(!readOnly && !isAlignmentEmpty());
    insSymAction->setEnabled(!readOnly && !isAlignmentEmpty());
    createSubaligniment->setEnabled(!isAlignmentEmpty());
    saveSequence->setEnabled(!isAlignmentEmpty());
    removeAllGapsAction->setEnabled(!readOnly && !isAlignmentEmpty());
    addSeqFromProjectAction->setEnabled(!readOnly);
    addSeqFromFileAction->setEnabled(!readOnly);
    sortByNameAction->setEnabled(!readOnly && !isAlignmentEmpty());
    collapseModeSwitchAction->setEnabled(!readOnly && !isAlignmentEmpty());
    reverseComplementAction->setEnabled(!readOnly && maObj->getAlphabet()->isNucleic() && !isAlignmentEmpty());
    reverseAction->setEnabled(!readOnly && maObj->getAlphabet()->isNucleic() && !isAlignmentEmpty());
    complementAction->setEnabled(!readOnly && maObj->getAlphabet()->isNucleic() && !isAlignmentEmpty());

    assert(checkState());
}

void MSAEditorSequenceArea::paintEvent(QPaintEvent *e) {
    drawAll();
    QWidget::paintEvent(e);
}

void MSAEditorSequenceArea::drawAll() {
    QSize s = size();
    if (cachedView->size() != s) {
        assert(completeRedraw);
        delete cachedView;
        cachedView = new QPixmap(s);
    }
    if (completeRedraw) {
        QPainter pCached(cachedView);
        drawContent(pCached);
        completeRedraw = false;
    }
    QPainter p(this);
    p.drawPixmap(0, 0, *cachedView);
    //drawCursor(p);
    drawSelection(p);
    drawFocus(p);
}

void MSAEditorSequenceArea::drawContent(QPainter& p) {
    drawContent(p, QRect(startPos, getFirstVisibleSequence(), getNumVisibleBases(false), getNumVisibleSequences(true)));
}

void MSAEditorSequenceArea::drawContent(QPainter &p, const QRect &area) {
    QVector<U2Region> range;
    if (ui->isCollapsibleMode()) {
        ui->getCollapseModel()->getVisibleRows(area.y(), area.bottom(), range);
    } else {
        range.append(U2Region(area.y(), area.height()));
    }

    QList <qint64> seqIdx;
    foreach(U2Region region, range) {
        for (qint64 i = region.startPos; i < region.endPos(); i++) {
            seqIdx.append(i);
        }
    }
    p.fillRect(cachedView->rect(), Qt::white);
    drawContent(p, U2Region(area.x(), area.width()), seqIdx);
    emit si_visibleRangeChanged();
}

void MSAEditorSequenceArea::drawContent(QPainter &p, const U2Region &region, const QList<qint64> &seqIdx) {
    CHECK( !region.isEmpty(), );
    CHECK( !seqIdx.isEmpty(), );

    p.fillRect(QRect(0, 0, editor->getColumnWidth() * region.length,
                      editor->getRowHeight() * seqIdx.size()),
               Qt::white);
    p.setFont(editor->getFont());

    MAlignmentObject* maObj = editor->getMSAObject();
    SAFE_POINT(maObj != NULL, tr("Alignment object is NULL"), );
    const MAlignment msa = maObj->getMAlignment();

    U2OpStatusImpl os;
    const int refSeq = msa.getRowIndexByRowId(editor->getReferenceRowId(), os);
    QString refSeqName = editor->getReferenceRowName();
    const MAlignmentRow *r = NULL;
    if (MAlignmentRow::invalidRowId() != refSeq) {
        r = &(msa.getRow(refSeq));
    }

    //Use dots to draw regions, which are similar to reference sequence
    highlightingScheme->setUseDots(useDotsAction->isChecked());
    //Highlighting scheme's settings
    QString schemeName = highlightingScheme->metaObject()->className();
    bool isGapsScheme = schemeName == "U2::MSAHighlightingSchemeGaps";

    U2Region baseYRange = U2Region(0, editor->getRowHeight());
    int columnWidth = editor->getColumnWidth();

    bool isResizeMode = editor->getResizeMode() == MSAEditor::ResizeMode_FontAndContent;
    for (qint64 iSeq = 0; iSeq < seqIdx.size(); iSeq++) {
        qint64 seq = seqIdx[iSeq];
        qint64 regionEnd = region.endPos() - (int)(region.endPos() == editor->getAlignmentLen());
        for (int pos = region.startPos; pos <= regionEnd; pos++) {
            U2Region baseXRange = U2Region(columnWidth * (pos - region.startPos), columnWidth);
            QRect cr(baseXRange.startPos, baseYRange.startPos, baseXRange.length + 1, baseYRange.length);
            char c = msa.charAt(seq, pos);

            QColor color = colorScheme->getColor(seq, pos, c);
            bool drawColor = false;
            if (isGapsScheme || highlightingScheme->getFactory()->isRefFree()){ //schemes which applied without reference
                const char refChar = 'z';
                highlightingScheme->process(refChar, c, drawColor, pos, seq);
                if(isGapsScheme){
                    color = QColor(192, 192, 192);
                }
                if (color.isValid() && drawColor) {
                    p.fillRect(cr, color);
                }
                if (editor->getResizeMode() == MSAEditor::ResizeMode_FontAndContent) {
                    p.drawText(cr, Qt::AlignCenter, QString(c));
                }
            }else if(seq == refSeq || refSeqName.isEmpty()){
                if (color.isValid()) {
                    p.fillRect(cr, color);
                }
                if (isResizeMode) {
                    p.drawText(cr, Qt::AlignCenter, QString(c));
                }
            }else{
                const char refChar = r->charAt(pos);
                highlightingScheme->process(refChar, c, drawColor, pos, seq);

                if(isGapsScheme){
                    color = QColor(192, 192, 192);
                }
                if (color.isValid()  &&  drawColor ) {
                    p.fillRect(cr, color);
                }
                if (isResizeMode) {
                    p.drawText(cr, Qt::AlignCenter, QString(c));
                }
            }
        }
        baseYRange.startPos += editor->getRowHeight();
    }
}

void MSAEditorSequenceArea::drawSelection( QPainter &p )
{
    int x = selection.x();
    int y = selection.y();

    U2Region xRange = getBaseXRange(x, true);
    U2Region yRange = getSequenceYRange(y, true);

    QPen pen(highlightSelection || hasFocus()? Qt::black : Qt::gray);
    pen.setStyle(Qt::DashLine);
    pen.setWidth(highlightSelection ? 2 : 1);
    p.setPen(pen);
    if(yRange.startPos > 0) {
        p.drawRect(xRange.startPos, yRange.startPos, xRange.length*selection.width(), yRange.length*selection.height());
    }
    else {
        qint64 regionHeight = yRange.length*selection.height() + yRange.startPos + 1;
        if(regionHeight <= 0) {
            return;
        }
        p.drawRect(xRange.startPos, -1, xRange.length*selection.width(), regionHeight);
    }
}

void MSAEditorSequenceArea::drawCursor(QPainter& p) {
    if (!isVisible(cursorPos, true)) {
        return;
    }
    U2Region xRange = getBaseXRange(cursorPos.x(), true);
    U2Region yRange = getSequenceYRange(cursorPos.y(), true);

    QPen pen(highlightSelection || hasFocus()? Qt::black : Qt::gray);
    pen.setStyle(Qt::DashLine);
    pen.setWidth(highlightSelection ? 2 : 1);
    p.setPen(pen);
    p.drawRect(xRange.startPos, yRange.startPos, xRange.length, yRange.length);
}

void MSAEditorSequenceArea::drawFocus(QPainter& p) {
    if (hasFocus()) {
        p.setPen(QPen(Qt::black, 1, Qt::DotLine));
        p.drawRect(0, 0, width()-1, height()-1);
    }
}

bool MSAEditorSequenceArea::isPosInRange(int p) const {
    return p >= 0 && p < editor->getAlignmentLen();
}

bool MSAEditorSequenceArea::isSeqInRange( int s ) const {
    return ( s >= 0 && s < getNumDisplayedSequences( ) );
}

bool MSAEditorSequenceArea::isPosVisible(int pos, bool countClipped) const {
    if (pos < getFirstVisibleBase() || pos > getLastVisibleBase(countClipped)) {
        return false;
    }
    return true;
}

bool MSAEditorSequenceArea::isSeqVisible(int seq, bool countClipped) const {
    if (seq < getFirstVisibleSequence() || seq > getLastVisibleSequence(countClipped)) {
        return false;
    }
    return true;
}

void MSAEditorSequenceArea::setFirstVisibleBase(int pos) {
    if (pos == startPos) {
        return;
    }

    if (!isAlignmentEmpty()) {
        SAFE_POINT(isPosInRange(pos), tr("Position is out of range: %1").arg(QString::number(pos)), );

        QPoint prev(startPos, startSeq);

        int aliLen = editor->getAlignmentLen();
        int effectiveFirst = qMin(aliLen - countWidthForBases(false), pos);
        startPos = qMax(0, effectiveFirst);

        QPoint p(startPos, startSeq);
        emit si_startChanged(p, prev);
    } else {
        startPos = -1;
    }

    updateHScrollBar();

    completeRedraw = true;
    update();
}

void MSAEditorSequenceArea::setFirstVisibleSequence(int seq) {
    if (seq == startSeq) {
        return;
    }

    if (!isAlignmentEmpty()) {
        SAFE_POINT(isSeqInRange(seq), tr("Sequence is out of range: %1").arg(QString::number(seq)), );

        QPoint prev(startPos, startSeq);

        int nSeq = editor->getNumSequences();
        int effectiveFirst = qMin(nSeq - countHeightForSequences(false), seq);
        startSeq = qMax(0, effectiveFirst);

        QPoint p(startPos, startSeq);
        emit si_startChanged(p, prev);
    }

    updateVScrollBar();

    completeRedraw = true;
    update();
}

void MSAEditorSequenceArea::resizeEvent(QResizeEvent *e) {
    completeRedraw = true;
    validateRanges();
    QWidget::resizeEvent(e);
}

void MSAEditorSequenceArea::validateRanges() {
    //check x dimension
    int aliLen = editor->getAlignmentLen();
    int visibleBaseCount = countWidthForBases(false);

    if (isAlignmentEmpty()) {
        setFirstVisibleBase(-1);
    } else if (visibleBaseCount > aliLen) {
        setFirstVisibleBase(0);
    } else if (startPos + visibleBaseCount > aliLen) {
        setFirstVisibleBase(aliLen - visibleBaseCount);
    }

    SAFE_POINT(0 <= startPos || isAlignmentEmpty(), tr("Negative startPos with non-empty alignment"), );
    SAFE_POINT(startPos + visibleBaseCount <= aliLen || aliLen < visibleBaseCount, tr("startPos is too big"), );

    updateHScrollBar();

    //check y dimension
    if (ui->isCollapsibleMode()) {
        sl_modelChanged();
        return;
    }

    int nSeqs = editor->getNumSequences();
    int visibleSequenceCount = countHeightForSequences(false);

    if (isAlignmentEmpty()) {
        setFirstVisibleSequence(-1);
    } else if (visibleSequenceCount > nSeqs) {
        setFirstVisibleSequence(0);
    } else if (startSeq + visibleSequenceCount > nSeqs) {
        setFirstVisibleSequence(nSeqs - visibleSequenceCount);
    }

    SAFE_POINT(0 <= startSeq || isAlignmentEmpty(), tr("Negative startSeq with non-empty alignment"), );
    SAFE_POINT(startSeq + visibleSequenceCount <= nSeqs || nSeqs < visibleSequenceCount, tr("startSeq is too big"), );

    updateVScrollBar();
}

void MSAEditorSequenceArea::sl_onHScrollMoved(int pos) {
    if (isAlignmentEmpty()) {
        setFirstVisibleBase(-1);
    } else {
        SAFE_POINT(0 <= pos && pos <= editor->getAlignmentLen() - getNumVisibleBases(false), tr("Position is out of range: %1").arg(QString::number(pos)), );
        setFirstVisibleBase(pos);
    }
}

void MSAEditorSequenceArea::sl_onVScrollMoved(int seq) {
    if (isAlignmentEmpty()) {
        setFirstVisibleSequence(-1);
    } else {
        SAFE_POINT(0 <= seq && seq <= editor->getNumSequences() - getNumVisibleSequences(false), tr("Sequence is out of range: %1").arg(QString::number(seq)), );
        setFirstVisibleSequence(seq);
    }
}

void MSAEditorSequenceArea::updateHScrollBar() {
    shBar->disconnect(this);

    if (isAlignmentEmpty()) {
        shBar->setDisabled(true);
    } else {
        int numVisibleBases = getNumVisibleBases(false);
        int alignmentLen = editor->getAlignmentLen();

        SAFE_POINT(numVisibleBases <= alignmentLen, tr("Horizontal scrollbar appears unexpectedly: numVisibleBases is too small"), );

        shBar->setMinimum(0);
        shBar->setMaximum(alignmentLen - numVisibleBases);
        shBar->setSliderPosition(getFirstVisibleBase());

        shBar->setSingleStep(1);
        shBar->setPageStep(numVisibleBases);

        shBar->setDisabled(numVisibleBases == alignmentLen);
    }

    connect(shBar, SIGNAL(valueChanged(int)), SLOT(sl_onHScrollMoved(int)));
}

void MSAEditorSequenceArea::updateVScrollBar() {
    svBar->disconnect(this);

    if (isAlignmentEmpty()) {
        svBar->setDisabled(true);
    } else {
        int start = getFirstVisibleSequence();
        int numVisibleSequences = getNumVisibleSequences(false);
        int nSeqs = getNumDisplayedSequences();

        SAFE_POINT(numVisibleSequences <= nSeqs, tr("Vertical scrollbar appears unexpectedly: numVisibleSequences is too small"), );

        svBar->setMinimum(0);
        svBar->setMaximum(nSeqs - numVisibleSequences);
        svBar->setSliderPosition(start);

        svBar->setSingleStep(1);
        svBar->setPageStep(numVisibleSequences);

        svBar->setDisabled(numVisibleSequences == nSeqs);
    }

    connect(svBar, SIGNAL(valueChanged(int)), SLOT(sl_onVScrollMoved(int)));

    onVisibleRangeChanged();
}

void MSAEditorSequenceArea::onVisibleRangeChanged() {
    qint64 firstVisibleSeq = getFirstVisibleSequence();
    qint64 lastVisibleSeq  = getLastVisibleSequence(true);

    QStringList rowNames = editor->getMSAObject()->getMAlignment().getRowNames();
    QStringList visibleSeqs;

    if (!isAlignmentEmpty()) {
        QVector<U2Region> range;
        if (ui->isCollapsibleMode()) {
            ui->getCollapseModel()->getVisibleRows(firstVisibleSeq, lastVisibleSeq, range);
        } else {
            range.append(U2Region(firstVisibleSeq, lastVisibleSeq - firstVisibleSeq + 1));
        }

        foreach(const U2Region& region, range) {
            int start = region.startPos;
            int end = qMin(region.endPos(), lastVisibleSeq);
            for (int seq = start; seq <= end; seq++) {
                visibleSeqs.append(rowNames.at(seq));
            }
        }
    }

    emit si_visibleRangeChanged(visibleSeqs, getHeight());
}

bool MSAEditorSequenceArea::isAlignmentLocked() {
    MAlignmentObject* obj = editor->getMSAObject();
    SAFE_POINT(NULL != obj, tr("Alignment object is not available"), true);
    return obj->isStateLocked();
}

int MSAEditorSequenceArea::countWidthForBases(bool countClipped, bool forOffset) const {
    int seqAreaWidth = width();
    int colWidth = editor->getColumnWidth();
    int nVisible = seqAreaWidth / colWidth;

    if(countClipped) {
        bool colIsVisible = ((float)(seqAreaWidth % colWidth) / colWidth < 0.5) ? 0 : 1;
        colIsVisible |= !forOffset;
        nVisible += colIsVisible && (seqAreaWidth % colWidth != 0);
    }

    return nVisible;
}

int MSAEditorSequenceArea::countHeightForSequences(bool countClipped) const {
    int seqAreaHeight = height();
    int nVisible = seqAreaHeight / editor->getRowHeight() + (countClipped && (seqAreaHeight % editor->getRowHeight() != 0) ? 1 : 0);
    return nVisible;
}

int MSAEditorSequenceArea::getNumVisibleBases(bool countClipped, bool forOffset) const {
    if (isAlignmentEmpty()) {
        return 0;
    }

    int lastVisible = getLastVisibleBase(countClipped, forOffset);
    SAFE_POINT((startPos <= lastVisible || (!countClipped && lastVisible + 1 == startPos /*1 symbol is visible & clipped*/)),
               tr("Last visible base is less than startPos"), 0);
    SAFE_POINT(lastVisible < editor->getAlignmentLen(), tr("Last visible base is out of range"), 0);
    int res = lastVisible - startPos + 1;
    return res;
}

int MSAEditorSequenceArea::getLastVisibleBase(bool countClipped, bool forOffset) const {
    if (isAlignmentEmpty()) {
        return 0;
    }

    int nVisible = countWidthForBases(countClipped, forOffset);
    int alignLen = editor->getAlignmentLen();
    int res = qBound(0, startPos + nVisible - 1, alignLen - 1);
    return res;
}

int MSAEditorSequenceArea::getLastVisibleSequence(bool countClipped) const {
    if (isAlignmentEmpty()) {
        return 0;
    }

    int nVisible = countHeightForSequences(countClipped);
    int numSeqs = getNumDisplayedSequences();
    int res = qBound(0, startSeq + nVisible - 1, numSeqs - 1);
    return res;
}

int MSAEditorSequenceArea::getNumVisibleSequences(bool countClipped) const {
    if (isAlignmentEmpty()) {
        return 0;
    }

    int lastVisible =  getLastVisibleSequence(countClipped);
    SAFE_POINT(startSeq <= lastVisible, tr("Last visible sequence is less than startSeq"), 0);
    SAFE_POINT(lastVisible < editor->getNumSequences(), tr("Last visible sequence is out of range"), 0);

    int sequencesNumber = 0;
    if (ui->isCollapsibleMode()) {
        QVector<U2Region> range;
        ui->getCollapseModel()->getVisibleRows(startSeq, lastVisible, range);
        foreach(U2Region region, range) {
            sequencesNumber += region.length;
        }
        return sequencesNumber;
    }
    else {
        sequencesNumber = lastVisible - startSeq + 1;
        return sequencesNumber;
    }
}

int MSAEditorSequenceArea::getNumDisplayedSequences( ) const {
    if (isAlignmentEmpty()) {
        return 0;
    }

    MSACollapsibleItemModel *model = ui->getCollapseModel( );
    SAFE_POINT( NULL != model, tr("Invalid collapsible item model!"), -1 );
    return model->displayedRowsCount( );
}

int MSAEditorSequenceArea::getColumnNumByX(int x, bool selecting) const {
    int colOffs = x / editor->getColumnWidth();
    int pos = startPos + colOffs;
    if (!selecting) {
        if ((pos >= editor->getAlignmentLen()) || (pos < 0)) {
            return -1;
        }
    }
    else {
        if (pos < 0) {
            pos = 0;
        }
        if (pos >= editor->getAlignmentLen()) {
            pos = editor->getAlignmentLen() - 1;
        }
    }
    return pos;

}

int MSAEditorSequenceArea::getXByColumnNum(int columnNum) const {
    return (columnNum + 0.5f)*editor->getColumnWidth();
}

int MSAEditorSequenceArea::getSequenceNumByY(int y) const {
    int seqOffs = y / editor->getRowHeight();
    int seq = startSeq + seqOffs;
    const int countOfVisibleSeqs = getNumDisplayedSequences( );
    if (!selecting) {
        if ((seqOffs >= countOfVisibleSeqs) || (seq < 0)) {
            return -1;
        }
    }
    else {
        if (seq < 0) {
            seq = 0;
        }
        if (seq >= countOfVisibleSeqs) {
            seq = countOfVisibleSeqs - 1;
        }
    }
    return seq;
}

int MSAEditorSequenceArea::getYBySequenceNum(int sequenceNum) const {
    return (sequenceNum + 0.5f)*editor->getRowHeight();
}

U2Region MSAEditorSequenceArea::getBaseXRange(int pos, bool useVirtualCoords) const {
    U2Region res(editor->getColumnWidth() * (pos - startPos), editor->getColumnWidth());
    if (!useVirtualCoords) {
        int w = width();
        res = res.intersect(U2Region(0, w));
    }
    return res;
}

U2Region MSAEditorSequenceArea::getSequenceYRange(int seq, bool useVirtualCoords) const {
    U2Region res(editor->getRowHeight()* (seq - startSeq), editor->getRowHeight());
    if (!useVirtualCoords) {
        int h = height();
        res = res.intersect(U2Region(0, h));
    }
    return res;
}

#define SCROLL_STEP 1

void MSAEditorSequenceArea::updateSelection( const QPoint& newPos) {
    int width = qAbs(newPos.x() - cursorPos.x()) + 1;
    int height = qAbs(newPos.y() - cursorPos.y()) + 1;
    int left = qMin(newPos.x(), cursorPos.x());
    int top = qMin(newPos.y(), cursorPos.y());

    MSAEditorSelection s(left, top, width, height);
    if (newPos.x()!=-1 && newPos.y()!=-1) {
        setSelection(s);
    }
    if (selection.isNull()){
        ui->getCopySelectionAction()->setDisabled(true);
    }else{
        ui->getCopySelectionAction()->setEnabled(true);
    }
}

void MSAEditorSequenceArea::updateSelection() {
    CHECK(!baseSelection.isNull(), );

    if (!ui->isCollapsibleMode()) {
        setSelection(baseSelection);
        return;
    }
    MSACollapsibleItemModel* m = ui->getCollapseModel();
    CHECK(NULL != m, );

    int startPos = baseSelection.y();
    int endPos = startPos + baseSelection.height();

    // convert selected rows indexes to indexes of selected collapsible items
    int newStart = m->rowToMap(startPos);
    int newEnd = m->rowToMap(endPos);

    int selectionHeight = newEnd - newStart;
    // accounting of collapsing children items
    int itemIndex = m->itemAt(newEnd);
    if (selectionHeight <= 1 && itemIndex >= 0) {
        const MSACollapsableItem& collapsibleItem = m->getItem(itemIndex);
        if(newEnd == collapsibleItem.row && !collapsibleItem.isCollapsed) {
            newEnd = collapsibleItem.row ;
            selectionHeight = qMax(selectionHeight, endPos - newStart + collapsibleItem.numRows);
        }
    }
    if(selectionHeight > 0 && newStart + selectionHeight <= m->displayedRowsCount()) {
        MSAEditorSelection s(selection.topLeft().x(), newStart, selection.width(), selectionHeight);
        setSelection(s);
    }
    else {
        cancelSelection();
    }
}

void MSAEditorSequenceArea::mouseMoveEvent( QMouseEvent* e )
{
    if ( e->buttons( ) & Qt::LeftButton ) {
        QPoint newCurPos = coordToAbsolutePosOutOfRange(e->pos());
        if (isInRange(newCurPos)) {
            updateHBarPosition(newCurPos.x());
            updateVBarPosition(newCurPos.y());
        }

        if (shifting) {
            shiftSelectedRegion(newCurPos.x() - cursorPos.x());
            shiftingWasPerformed = true;
        } else if (selecting) {
            rubberBand->setGeometry(QRect(origin, e->pos()).normalized());
        }
    }

    QWidget::mouseMoveEvent(e);
}

void MSAEditorSequenceArea::mouseReleaseEvent(QMouseEvent *e) {
    rubberBand->hide();
    if (shifting) {
        changeTracker.finishTracking();
        editor->getMSAObject()->releaseState();
    }

    QPoint newCurPos = coordToAbsolutePos(e->pos());

    int firstVisibleSeq = ui->seqArea->getFirstVisibleSequence();
    int visibleRowsNums = getNumDisplayedSequences() - 1;

    int yPosWithValidations = qMax(firstVisibleSeq, newCurPos.y());
    yPosWithValidations = qMin(yPosWithValidations, visibleRowsNums + firstVisibleSeq);

    newCurPos.setY(yPosWithValidations);

    if (shifting) {
        const int shift = (!shiftingWasPerformed)
            ? newCurPos.x() - ui->seqArea->getSelection().getRect().center().x()
            : newCurPos.x() - cursorPos.x();
        if (0 != shift && !isAlignmentLocked()) {
            U2OpStatus2Log os;
            U2UseCommonUserModStep userModStep(editor->getMSAObject()->getEntityRef(), os);
            Q_UNUSED(userModStep);
            shiftSelectedRegion(shift);
        }
        emit si_stopMSAChanging(true);
    } else if (Qt::LeftButton == e->button() && Qt::LeftButton == prevPressedButton) {
        updateSelection(newCurPos);
    }
    shifting = false;
    selecting = false;
    shiftingWasPerformed = false;

    shBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
    svBar->setupRepeatAction(QAbstractSlider::SliderNoAction);

    QWidget::mouseReleaseEvent(e);
}

void MSAEditorSequenceArea::mousePressEvent(QMouseEvent *e) {
    prevPressedButton = e->button();

    if (!hasFocus()) {
        setFocus();
    }

    if ((e->button() == Qt::LeftButton)) {
        if (Qt::ShiftModifier == e->modifiers()) {
            QWidget::mousePressEvent(e);
            return;
        }

        origin = e->pos();
        QPoint p = coordToPos(e->pos());
        if(isInRange(p)) {
            setCursorPos(p);

            const MSAEditorSelection &s = ui->seqArea->getSelection();
            if (s.getRect().contains(cursorPos) && !isAlignmentLocked()) {
                shifting = true;
                U2OpStatus2Log os;
                changeTracker.startTracking(os);
                CHECK_OP(os, );
                editor->getMSAObject()->saveState();
                emit si_startMSAChanging();
            }
        }

        if (!shifting) {
            selecting = true;
            origin = e->pos();
            QPoint q = coordToAbsolutePos(e->pos());
            if (isInRange(q)) {
                setCursorPos(q);
            }
            rubberBand->setGeometry(QRect(origin, QSize()));
            rubberBand->show();
            ui->seqArea->cancelSelection();
        }
    }

    QWidget::mousePressEvent(e);
}

void MSAEditorSequenceArea::keyPressEvent(QKeyEvent *e) {
    if (!hasFocus()) {
        return;
    }

    int key = e->key();
    bool shift = e->modifiers().testFlag(Qt::ShiftModifier);
    const bool ctrl = e->modifiers( ).testFlag( Qt::ControlModifier );
#ifdef Q_OS_MAC
    // In one case it is better to use a Command key as modifier,
    // in another - a Control key. genuineCtrl - Control key on Mac OS X.
    const bool genuineCtrl = e->modifiers( ).testFlag( Qt::MetaModifier );
#else
    const bool genuineCtrl = ctrl;
#endif
    static QPoint selectionStart(0, 0);
    static QPoint selectionEnd(0, 0);

    if (ctrl && (key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_Up || key == Qt::Key_Down)) {
        //remap to page_up/page_down
        shift = key == Qt::Key_Up || key == Qt::Key_Down;
        key =  (key == Qt::Key_Up || key == Qt::Key_Left) ? Qt::Key_PageUp : Qt::Key_PageDown;
    }
    //part of these keys are assigned to actions -> so them never passed to keyPressEvent (action handling has higher priority)
    int endX, endY;
    switch(key) {
        case Qt::Key_Escape:
             cancelSelection();
             break;
        case Qt::Key_Left:
            if(!(Qt::ShiftModifier & e->modifiers())) {
                moveSelection(-1,0);
                break;
            }
            if( selectionEnd.x() < 1 ) {
                break;
            }
            selectionEnd.setX(selectionEnd.x() - 1);
            endX = selectionEnd.x();
            if (isPosInRange(endX) ) {
                if (endX != -1) {
                    int firstColumn = qMin(selectionStart.x(),endX);
                    int width = qAbs(endX - selectionStart.x()) + 1;
                    int startSeq = selection.y();
                    int height = selection.height();
                    if (selection.isNull()) {
                        startSeq = cursorPos.y();
                        height = 1;
                    }
                    MSAEditorSelection _selection(firstColumn, startSeq, width, height);
                    setSelection(_selection);
                }
            }
            break;
        case Qt::Key_Right:
            if(!(Qt::ShiftModifier & e->modifiers())) {
                moveSelection(1,0);
                break;
            }
            if( selectionEnd.x() >= ( editor->getAlignmentLen() - 1 ) ) {
                break;
            }
            selectionEnd.setX(selectionEnd.x() +  1);
            endX = selectionEnd.x();
            if (isPosInRange(endX) ) {
                if (endX != -1) {
                    int firstColumn = qMin(selectionStart.x(),endX);
                    int width = qAbs(endX - selectionStart.x()) + 1;
                    int startSeq = selection.y();
                    int height = selection.height();
                    if (selection.isNull()) {
                        startSeq = cursorPos.y();
                        height = 1;
                    }
                    MSAEditorSelection _selection(firstColumn, startSeq, width, height);
                    setSelection(_selection);
                }
            }
            break;
        case Qt::Key_Up:
            if(!(Qt::ShiftModifier & e->modifiers())) {
                moveSelection(0,-1);
                break;
            }
            if( selectionEnd.y() < 1 ) {
                break;
            }
            selectionEnd.setY(selectionEnd.y() - 1);
            endY = selectionEnd.y();
            if (isSeqInRange(endY) ) {
                if (endY != -1) {
                    int startSeq = qMin(selectionStart.y(),endY);
                    int height = qAbs(endY - selectionStart.y()) + 1;
                    int firstColumn = selection.x();
                    int width = selection.width();
                    if (selection.isNull()) {
                        firstColumn = cursorPos.x();
                        width = 1;
                    }
                    MSAEditorSelection _selection(firstColumn, startSeq, width, height );
                    setSelection(_selection);
                }
            }
            break;
        case Qt::Key_Down:
            if(!(Qt::ShiftModifier & e->modifiers())) {
                moveSelection(0,1);
                break;
            }
            if( selectionEnd.y() >= ( ui->collapseModel->displayedRowsCount() - 1 ) ) {
                break;
            }
            selectionEnd.setY(selectionEnd.y() + 1);
            endY = selectionEnd.y();
            if (isSeqInRange(endY) ) {
                if (endY != -1) {
                    int startSeq = qMin(selectionStart.y(),endY);
                    int height = qAbs(endY - selectionStart.y()) + 1;
                    int firstColumn = selection.x();
                    int width = selection.width();
                    if (selection.isNull()) {
                        firstColumn = cursorPos.x();
                        width = 1;
                    }
                    MSAEditorSelection _selection(firstColumn, startSeq, width, height );
                    setSelection(_selection);
                }
            }
            break;
        case Qt::Key_Home:
            cancelSelection();
            if (shift) { //scroll namelist
                setFirstVisibleSequence(0);
                setCursorPos(QPoint(cursorPos.x(), 0));
            } else { //scroll sequence
                cancelSelection();
                setFirstVisibleBase(0);
                setCursorPos(QPoint(0, cursorPos.y()));
            }
            break;
        case Qt::Key_End:
            cancelSelection();
            if (shift) { //scroll namelist
                int n = editor->getNumSequences() - 1;
                setFirstVisibleSequence(n);
                setCursorPos(QPoint(cursorPos.x(), n));
            } else { //scroll sequence
                int n = editor->getAlignmentLen() - 1;
                setFirstVisibleBase(n);
                setCursorPos(QPoint(n, cursorPos.y()));
            }
            break;
        case Qt::Key_PageUp:
            cancelSelection();
            if (shift) { //scroll namelist
                int nVis = getNumVisibleSequences(false);
                int fp = qMax(0, getFirstVisibleSequence() - nVis);
                int cp = qMax(0, cursorPos.y() - nVis);
                setFirstVisibleSequence(fp);
                setCursorPos(QPoint(cursorPos.x(), cp));
            } else { //scroll sequence
                int nVis = getNumVisibleBases(false);
                int fp = qMax(0, getFirstVisibleBase() - nVis);
                int cp = qMax(0, cursorPos.x() - nVis);
                setFirstVisibleBase(fp);
                setCursorPos(QPoint(cp, cursorPos.y()));
            }
            break;
        case Qt::Key_PageDown:
            cancelSelection();
            if (shift) { //scroll namelist
                int nVis = getNumVisibleSequences(false);
                int nSeq = editor->getNumSequences();
                int fp = qMin(nSeq-1, getFirstVisibleSequence() + nVis);
                int cp = qMin(nSeq-1, cursorPos.y() + nVis);
                setFirstVisibleSequence(fp);
                setCursorPos(QPoint(cursorPos.x(), cp));
            } else { //scroll sequence
                int nVis = getNumVisibleBases(false);
                int len = editor->getAlignmentLen();
                int fp  = qMin(len-1, getFirstVisibleBase() + nVis);
                int cp  = qMin(len-1, cursorPos.x() + nVis);
                setFirstVisibleBase(fp);
                setCursorPos(QPoint(cp, cursorPos.y()));
            }
            break;
        case Qt::Key_Delete:
            if(!isAlignmentLocked()) {
                if (shift) {
                    sl_delCol();
                } else {
                    deleteCurrentSelection();
                    emit si_startMSAChanging();
                }
            }
            break;
        case Qt::Key_Backspace:
            removeGapsPrecedingSelection( genuineCtrl ? 1 : -1 );
            break;
        case Qt::Key_Insert:
        case Qt::Key_Space:
            // We can't use Command+Space on Mac OS X - it is reserved
            if(!isAlignmentLocked()) {
                insertGapsBeforeSelection( genuineCtrl ? 1 : -1 );
                emit si_startMSAChanging();
            }
            break;
        case Qt::Key_Shift:
            if (!selection.isNull()) {
                selectionStart = selection.topLeft();
                selectionEnd = selection.getRect().bottomRight();
            } else {
                selectionStart = cursorPos;
                selectionEnd = cursorPos;
            }
            break;
    }
    QWidget::keyPressEvent(e);
}

void MSAEditorSequenceArea::keyReleaseEvent(QKeyEvent *ke) {
    if ( ((ke->key() == Qt::Key_Space || ke->key() == Qt::Key_Delete) && !isAlignmentLocked() )
            && (!ke->isAutoRepeat()) ){
        emit si_stopMSAChanging(true);
    }

    QWidget::keyPressEvent(ke);
}

void MSAEditorSequenceArea::focusInEvent(QFocusEvent* fe) {
    QWidget::focusInEvent(fe);
    update();
}

void MSAEditorSequenceArea::focusOutEvent(QFocusEvent* fe) {
    QWidget::focusOutEvent(fe);
    update();
}

void MSAEditorSequenceArea::moveSelection( int dx, int dy )
{
    int leftX = selection.x();
    int topY = selection.y();
    int bottomY = selection.y() + selection.height() - 1;
    int rightX = selection.x() + selection.width() - 1;
    QPoint baseTopLeft(leftX, topY);
    QPoint baseBottomRight(rightX,bottomY);

    QPoint newTopLeft = baseTopLeft  + QPoint(dx,dy);
    QPoint newBottomRight = baseBottomRight + QPoint(dx,dy);

    if ((!isInRange(newTopLeft)) || (!isInRange(newBottomRight)) ) {
        return;
    }

     if (!isVisible(newTopLeft, false)) {
         if (isVisible(newTopLeft, true)) {
             if (dx != 0) {
                 setFirstVisibleBase(startPos + dx);
             }
             if (dy!=0) {
                 setFirstVisibleSequence(getFirstVisibleSequence()+dy);
             }
         } else {
             if (dx != 0) {
                setFirstVisibleBase(newTopLeft.x());
             }
             if (dy !=0 ) {
                setFirstVisibleSequence(newTopLeft.y());
             }
         }
     }

   MSAEditorSelection newSelection(newTopLeft, selection.width(), selection.height());
   setSelection(newSelection);

}

void MSAEditorSequenceArea::moveCursor(int dx, int dy) {
    QPoint p = cursorPos + QPoint(dx, dy);
    if (!isInRange(p)) {
        return;
    }

    // Move only one cell selection?
    // TODO: consider selection movement
    int sz = selection.width()*selection.height();
    if (sz != 1) {
        return;
    }

    if (!isVisible(p, false)) {
        if (isVisible(cursorPos, true)) {
            if (dx != 0) {
                setFirstVisibleBase(startPos + dx);
            }
            if (dy!=0) {
                setFirstVisibleSequence(getFirstVisibleSequence()+dy);
            }
        } else {
            setFirstVisibleBase(p.x());
            setFirstVisibleSequence(p.y());
        }
    }
    setCursorPos(p);
    //setSelection(MSAEditorSelection(p, 1,1));
}

int MSAEditorSequenceArea::coordToPos(int x) const {
    int y = getSequenceYRange(getFirstVisibleSequence(), false).startPos;
    return coordToPos(QPoint(x, y)).x();
}

QPoint MSAEditorSequenceArea::coordToAbsolutePos(const QPoint& coord) const {
    int column = getColumnNumByX(coord.x(), selecting);
    int row = getSequenceNumByY(coord.y());

    return QPoint(column, row);
}

QPoint MSAEditorSequenceArea::coordToAbsolutePosOutOfRange(const QPoint& coord) const {
    CHECK(editor->getColumnWidth() > 0, QPoint(0, 0));
    CHECK(editor->getRowHeight() > 0, QPoint(0, 0));
    int column = startPos + (coord.x() / editor->getColumnWidth());
    int row = startSeq + (coord.y() / editor->getRowHeight());

    return QPoint(column, row);
}

QPoint MSAEditorSequenceArea::coordToPos(const QPoint& coord) const {
    QPoint res(-1, -1);
    //Y: row
    int lastSeq = getLastVisibleSequence(true);
    if (ui->isCollapsibleMode()) {
        lastSeq = getNumDisplayedSequences( );
    }
    for (int i=getFirstVisibleSequence(); i<=lastSeq; i++) {
        U2Region r = getSequenceYRange(i, false);
        if (r.contains(coord.y())) {
            res.setY(i);
            break;
        }
    }

    //X: position in sequence
    for (int i=getFirstVisibleBase(), n = getLastVisibleBase(true); i<=n; i++) {
        U2Region r = getBaseXRange(i, false);
        if (r.contains(coord.x())) {
            res.setX(i);
            break;
        }
    }
    return res;
}

void MSAEditorSequenceArea::setSelection(const MSAEditorSelection& s) {
    // TODO: assert(isInRange(s));
    if (s == selection) {
        return;
    }

    MSAEditorSelection prevSelection = selection;
    selection = s;

    int selEndPos = s.x() + s.width() - 1;
    int ofRange = selEndPos - editor->getAlignmentLen();
    if(ofRange>=0) {
        selection = MSAEditorSelection(s.topLeft(), s.width() - ofRange - 1, s.height());
    }

    if (selection.isNull()){
        ui->getCopySelectionAction()->setDisabled(true);
    }else{
        ui->getCopySelectionAction()->setEnabled(true);
    }

    U2Region selectedRowsRegion = getSelectedRows();
    baseSelection = MSAEditorSelection(selection.topLeft().x(), getSelectedRows().startPos, selection.width(), selectedRowsRegion.length);

    selectedRowNames.clear();
    for(int x = selectedRowsRegion.startPos; x < selectedRowsRegion.endPos(); x++)
        selectedRowNames.append(editor->getMSAObject()->getRow(x).getName());
    emit si_selectionChanged(selectedRowNames);
    emit si_selectionChanged(selection, prevSelection);
    update();
}

void MSAEditorSequenceArea::setCursorPos(const QPoint& p) {
    SAFE_POINT(isInRange(p), tr("Cursor position is out of range"), );
    if (p == cursorPos) {
        return;
    }

    cursorPos = p;

    highlightSelection = false;
    updateActions();
}

void MSAEditorSequenceArea::removeGapsPrecedingSelection( int countOfGaps ) {
    const MSAEditorSelection selectionBackup = selection;
    // check if selection exists
    if ( selectionBackup.isNull( ) ) {
        return;
    }

    const QPoint selectionTopLeftCorner( selectionBackup.topLeft( ) );
    // don't perform the deletion if the selection is at the alignment start
    if ( 0 == selectionTopLeftCorner.x( ) || -1 > countOfGaps
        || 0 == countOfGaps )
    {
        return;
    }

    int removedRegionWidth = ( -1 == countOfGaps ) ? selectionBackup.width( )
        : countOfGaps;
    QPoint topLeftCornerOfRemovedRegion( selectionTopLeftCorner.x( ) - removedRegionWidth,
        selectionTopLeftCorner.y( ) );
    if ( 0 > topLeftCornerOfRemovedRegion.x( ) ) {
        removedRegionWidth -= qAbs( topLeftCornerOfRemovedRegion.x( ) );
        topLeftCornerOfRemovedRegion.setX( 0 );
    }

    MAlignmentObject *maObj = editor->getMSAObject( );
    if ( NULL == maObj || maObj->isStateLocked( ) ) {
        return;
    }

    // if this method was invoked during a region shifting
    // then shifting should be canceled
    cancelShiftTracking( );

    const U2Region rowsContainingRemovedGaps( getSelectedRows( ) );
    U2OpStatus2Log os;
    U2UseCommonUserModStep userModStep( maObj->getEntityRef( ), os );
    Q_UNUSED(userModStep);

    const int countOfDeletedSymbols = maObj->deleteGap( rowsContainingRemovedGaps,
        topLeftCornerOfRemovedRegion.x( ), removedRegionWidth, os );

    // if some symbols were actually removed and the selection is not located
    // at the alignment end, then it's needed to move the selection
    // to the place of the removed symbols
    if ( 0 < countOfDeletedSymbols) {
        const MSAEditorSelection newSelection( selectionBackup.x( ) - countOfDeletedSymbols,
            topLeftCornerOfRemovedRegion.y( ), selectionBackup.width( ),
            selectionBackup.height( ) );
        setSelection( newSelection );
    }
}

void MSAEditorSequenceArea::sl_alignmentChanged(const MAlignment&, const MAlignmentModInfo& modInfo) {
    int nSeq = editor->getNumSequences();
    int aliLen = editor->getAlignmentLen();
    if (ui->isCollapsibleMode()) {
        nSeq = ui->getCollapseModel()->getLastPos() + 1;
        updateCollapsedGroups(modInfo);
    }

    editor->updateReference();

    //todo: set in one method!
    setFirstVisibleBase(qBound(0, startPos, aliLen-countWidthForBases(false)));
    setFirstVisibleSequence(qBound(0, startSeq, nSeq - countHeightForSequences(false)));

    if ( ( selection.x( ) > aliLen - 1 ) || ( selection.y( ) > nSeq - 1 ) ) {
        cancelSelection( );
    } else {
        const QPoint selTopLeft( qMin( selection.x( ), aliLen - 1 ),
            qMin( selection.y( ), nSeq - 1 ) );
        const QPoint selBottomRight( qMin(selection.x( ) + selection.width( ) - 1, aliLen - 1 ),
            qMin( selection.y( ) + selection.height( ) - 1, nSeq -1 ) );

        MSAEditorSelection newSelection(selTopLeft, selBottomRight);
        // we don't emit "selection changed" signal to avoid redrawing
        setSelection( newSelection );
    }

    updateHScrollBar();
    updateVScrollBar();

    completeRedraw = true;
    updateActions();
    update();
}

void MSAEditorSequenceArea::updateCollapsedGroups(const MAlignmentModInfo& modInfo) {
    U2OpStatus2Log os;
    if(modInfo.sequenceContentChanged) {
        QList<qint64> updatedRows;
        bool isModelChanged = false;
        QMap<qint64, QList<U2MsaGap> > curGapModel = editor->getMSAObject()->getGapModel();
        foreach(qint64 modifiedSeqId, modInfo.modifiedRowIds) {
            int modifiedRowPos = editor->getMSAObject()->getRowPosById(modifiedSeqId);
            const MAlignmentRow& modifiedRowRef = editor->getMSAObject()->getRow(modifiedRowPos);
            modifiedRowPos = ui->getCollapseModel()->rowToMap(modifiedRowPos);
            U2Region rowsCollapsibleGroup = ui->getCollapseModel()->mapSelectionRegionToRows(U2Region(modifiedRowPos, 1));
            for(int i = rowsCollapsibleGroup.startPos; i < rowsCollapsibleGroup.endPos(); i++) {
                qint64 identicalRowId = editor->getMSAObject()->getRow(i).getRowId();
                if(!updatedRows.contains(identicalRowId) && !modInfo.modifiedRowIds.contains(identicalRowId)) {
                    isModelChanged = isModelChanged || modifiedRowRef.getGapModel() != curGapModel[identicalRowId];
                    curGapModel[identicalRowId] = modifiedRowRef.getGapModel();
                    updatedRows.append(identicalRowId);
                }
            }
        }
        if(isModelChanged) {
            editor->getMSAObject()->updateGapModel(curGapModel, os);
            return;
        }
    }
}

void MSAEditorSequenceArea::sl_buildStaticToolbar(GObjectView*, QToolBar* t) {
    t->addAction(ui->getUndoAction());
    t->addAction(ui->getRedoAction());
    t->addAction(gotoAction);
    t->addAction(removeAllGapsAction);
    t->addSeparator();
    t->addAction(collapseModeSwitchAction);
    t->addAction(collapseModeUpdateAction);
    t->addSeparator();
}

void MSAEditorSequenceArea::sl_buildStaticMenu(GObjectView*, QMenu* m) {
    buildMenu(m);
}

void MSAEditorSequenceArea::sl_buildContextMenu(GObjectView*, QMenu* m) {
    buildMenu(m);

    QMenu* editMenu = GUIUtils::findSubMenu(m, MSAE_MENU_EDIT);
    SAFE_POINT(editMenu != NULL, "editMenu", );

    QList<QAction*> actions;
    actions << delSelectionAction << insSymAction << reverseComplementAction << reverseAction << complementAction << delColAction << removeAllGapsAction;

    QMenu* copyMenu = GUIUtils::findSubMenu(m, MSAE_MENU_COPY);
    SAFE_POINT(copyMenu != NULL, "copyMenu", );

    if (rect().contains( mapFromGlobal(QCursor::pos()) ) ) {
        editMenu->addActions(actions);
        copyMenu->addAction(ui->getCopySelectionAction());
    }

    m->setObjectName( "msa sequence area context menu" ); // for GUI testing
}

void MSAEditorSequenceArea::sl_showCustomSettings(){
    AppContext::getAppSettingsGUI()->showSettingsDialog(ColorSchemaSettingsPageId);
}

void MSAEditorSequenceArea::buildMenu(QMenu* m) {
    QAction* copyMenuAction = GUIUtils::findAction(m->actions(), MSAE_MENU_LOAD);
    m->insertAction(copyMenuAction, gotoAction);

    QMenu* loadSeqMenu = GUIUtils::findSubMenu(m, MSAE_MENU_LOAD);
    SAFE_POINT(loadSeqMenu != NULL, "loadSeqMenu", );
    loadSeqMenu->addAction(addSeqFromProjectAction);
    loadSeqMenu->addAction(addSeqFromFileAction);

    QMenu* editMenu = GUIUtils::findSubMenu(m, MSAE_MENU_EDIT);
    SAFE_POINT(editMenu != NULL, "editMenu", );
    QList<QAction*> actions;
    actions << reverseComplementAction  << reverseAction << complementAction << removeAllGapsAction;
    editMenu->insertActions(editMenu->isEmpty() ? NULL : editMenu->actions().first(), actions);

    QMenu * exportMenu = GUIUtils::findSubMenu(m, MSAE_MENU_EXPORT);
    SAFE_POINT(exportMenu != NULL, "exportMenu", );
    exportMenu->addAction(createSubaligniment);
    exportMenu->addAction(saveSequence);

    QMenu* copyMenu = GUIUtils::findSubMenu(m, MSAE_MENU_COPY);
    SAFE_POINT(copyMenu != NULL, "copyMenu", );
    ui->getCopySelectionAction()->setDisabled(selection.isNull());
    copyMenu->addAction(ui->getCopySelectionAction());

    QMenu* viewMenu = GUIUtils::findSubMenu(m, MSAE_MENU_VIEW);
    SAFE_POINT(viewMenu != NULL, "viewMenu", );
    viewMenu->addAction(sortByNameAction);

    QMenu* colorsSchemeMenu = new QMenu(tr("Colors"), NULL);
    colorsSchemeMenu->menuAction()->setObjectName("Colors");
    colorsSchemeMenu->setIcon(QIcon(":core/images/color_wheel.png"));
    foreach(QAction* a, colorSchemeMenuActions) {
        colorsSchemeMenu->addAction(a);
    }

    QMenu* customColorSchemaMenu = new QMenu(tr("Custom schemes"), colorsSchemeMenu);
    customColorSchemaMenu->menuAction()->setObjectName("Custom schemes");

    foreach(QAction* a, customColorSchemeMenuActions) {
        customColorSchemaMenu->addAction(a);
    }

    if (!customColorSchemeMenuActions.isEmpty()){
        customColorSchemaMenu->addSeparator();
    }

    lookMSASchemesSettingsAction = new QAction(tr("Create new color scheme"), this);
    lookMSASchemesSettingsAction->setObjectName( "Create new color scheme" );
    connect(lookMSASchemesSettingsAction, SIGNAL(triggered()), SLOT(sl_showCustomSettings()));
    customColorSchemaMenu->addAction(lookMSASchemesSettingsAction);

    colorsSchemeMenu->addMenu(customColorSchemaMenu);
    m->insertMenu(GUIUtils::findAction(m->actions(), MSAE_MENU_EDIT), colorsSchemeMenu);

    QMenu* highlightSchemeMenu = new QMenu(tr("Highlighting"), NULL);

    highlightSchemeMenu->menuAction()->setObjectName("Highlighting");

    foreach(QAction* a, highlightingSchemeMenuActions) {
        highlightSchemeMenu->addAction(a);
    }
    highlightSchemeMenu->addSeparator();
    highlightSchemeMenu->addAction(useDotsAction);
    m->insertMenu(GUIUtils::findAction(m->actions(), MSAE_MENU_EDIT), highlightSchemeMenu);
}

void MSAEditorSequenceArea::sl_fontChanged(QFont font) {
    Q_UNUSED(font);
    completeRedraw = true;
    repaint();
}

void MSAEditorSequenceArea::sl_delCol() {
    DeleteGapsDialog dlg(this, editor->getMSAObject()->getNumRows());

    if(dlg.exec() == QDialog::Accepted) {
        MSACollapsibleItemModel *collapsibleModel = ui->getCollapseModel();
        SAFE_POINT(NULL != collapsibleModel, tr("NULL collapsible model!"), );
        collapsibleModel->reset();

        DeleteMode deleteMode = dlg.getDeleteMode();
        int value = dlg.getValue();

        // if this method was invoked during a region shifting
        // then shifting should be canceled
        cancelShiftTracking( );

        MAlignmentObject* msaObj = editor->getMSAObject();
        int gapCount = GAP_COLUMN_ONLY;
        switch(deleteMode) {
        case DeleteByAbsoluteVal:
            gapCount = value;
            break;
        case DeleteByRelativeVal: {
            int absoluteValue = qRound(( msaObj->getNumRows() * value ) / 100.0);
            if (absoluteValue < 1) {
                absoluteValue = 1;
            }
            gapCount = absoluteValue;
            break;
        }
        case DeleteAll:
            gapCount = GAP_COLUMN_ONLY;
            break;
        default:
            FAIL("Unknown delete mode", );
        }

        QList<qint64> columnsToDelete = msaObj->getColumnsWithGaps(gapCount);
        if (columnsToDelete.isEmpty()) {
            return;
        }
        U2OpStatus2Log os;
        U2UseCommonUserModStep userModStep(msaObj->getEntityRef(), os);
        Q_UNUSED(userModStep);
        SAFE_POINT_OP(os, );
        msaObj->deleteColumnWithGaps(gapCount);
    }
}

void MSAEditorSequenceArea::sl_fillCurrentSelectionWithGaps() {
    if(!isAlignmentLocked()) {
        emit si_startMSAChanging();
        insertGapsBeforeSelection();
        emit si_stopMSAChanging(true);
    }
}

void MSAEditorSequenceArea::sl_goto() {
    QDialog dlg(this);
    dlg.setModal(true);
    dlg.setWindowTitle(tr("Go To"));
    int aliLen = editor->getAlignmentLen();
    PositionSelector* ps = new PositionSelector(&dlg, 1, aliLen, true);
    connect(ps, SIGNAL(si_positionChanged(int)), SLOT(sl_onPosChangeRequest(int)));
    dlg.exec();
    delete ps;
}

void MSAEditorSequenceArea::sl_onPosChangeRequest(int pos) {
    centerPos(pos-1);
    setSelection(MSAEditorSelection(pos-1,selection.y(),1,1));
}

void MSAEditorSequenceArea::sl_lockedStateChanged() {
    updateActions();
}

void MSAEditorSequenceArea::centerPos(const QPoint& pos) {
    assert(isInRange(pos));
    int newStartPos = qMax(0, pos.x() - getNumVisibleBases(false)/2);
    setFirstVisibleBase(newStartPos);

    int newStartSeq = qMax(0, pos.y() - getNumVisibleSequences(false)/2);
    setFirstVisibleSequence(newStartSeq);
}

void MSAEditorSequenceArea::centerPos(int pos) {
    centerPos(QPoint(pos, cursorPos.y()));
}

void MSAEditorSequenceArea::wheelEvent (QWheelEvent * we) {
    bool toMin = we->delta() > 0;
    if (we->modifiers() == 0) {
        shBar->triggerAction(toMin ? QAbstractSlider::SliderSingleStepSub : QAbstractSlider::SliderSingleStepAdd);
    }  else if (we->modifiers() & Qt::SHIFT) {
        svBar->triggerAction(toMin ? QAbstractSlider::SliderSingleStepSub : QAbstractSlider::SliderSingleStepAdd);
    }
    QWidget::wheelEvent(we);
}

void MSAEditorSequenceArea::sl_removeAllGaps() {
    MAlignmentObject* msa = editor->getMSAObject();
    SAFE_POINT(NULL != msa, tr("NULL msa object!"), );
    assert(!msa->isStateLocked());

    MSACollapsibleItemModel *collapsibleModel = ui->getCollapseModel();
    SAFE_POINT(NULL != collapsibleModel, tr("NULL collapsible model!"), );
    collapsibleModel->reset();

    // if this method was invoked during a region shifting
    // then shifting should be canceled
    cancelShiftTracking( );

    U2OpStatus2Log os;
    U2UseCommonUserModStep userModStep(msa->getEntityRef(), os);
    SAFE_POINT_OP(os, );

    QMap<qint64, QList<U2MsaGap> > noGapModel;
    MAlignment ma = msa->getMAlignment();
    foreach (qint64 rowId, ma.getRowsIds()) {
        noGapModel[rowId] = QList<U2MsaGap>();
    }

    msa->updateGapModel(noGapModel, os);
    setFirstVisibleBase(0);
    setFirstVisibleSequence(0);
    SAFE_POINT_OP(os, );
}

bool MSAEditorSequenceArea::checkState() const {
#ifdef _DEBUG
    MAlignmentObject* maObj = editor->getMSAObject();
    int aliLen = maObj->getLength();
    int nSeqs = maObj->getNumRows();

    assert((startPos >=0 && startSeq >=0) || isAlignmentEmpty());
    int lastPos = getLastVisibleBase(true);
    int lastSeq = getLastVisibleSequence(true);
    assert((lastPos < aliLen && lastSeq < nSeqs) || isAlignmentEmpty());

    // TODO: check selection is valid
    //int cx = cursorPos.x();
    //int cy = cursorPos.y();
    //assert(cx >= 0 && cy >= 0);
    //assert(cx < aliLen && cy < nSeqs);
#endif
    return true;
}

void MSAEditorSequenceArea::sl_zoomOperationPerformed( bool resizeModeChanged )
{
    Q_UNUSED(resizeModeChanged);
    completeRedraw = true;
    validateRanges();
    updateActions();
    update();
    onVisibleRangeChanged();
}

void MSAEditorSequenceArea::sl_modelChanged() {
    MSACollapsibleItemModel *collapsibleModel = ui->getCollapseModel();
    SAFE_POINT(NULL != collapsibleModel, tr("NULL collapsible model!"), );

    if (collapsibleModel->isEmpty()) {
        collapseModeSwitchAction->setChecked(false);
        collapseModeUpdateAction->setEnabled(false);
    }

    int startToLast = collapsibleModel->getLastPos() - getFirstVisibleSequence() + 1;
    int availableNum = countHeightForSequences(false);
    if (startToLast < availableNum) {
        int newStartSeq = qMax(0, startSeq - availableNum + startToLast);
        if (startSeq != newStartSeq) {
            setFirstVisibleSequence(newStartSeq);
            return;
        }
    }
    updateSelection();

    completeRedraw = true;
    updateVScrollBar();
    update();
}

void MSAEditorSequenceArea::sl_referenceSeqChanged(qint64){
    completeRedraw = true;
    update();
}

void MSAEditorSequenceArea::sl_createSubaligniment(){
    CreateSubalignmentDialogController dialog(editor->getMSAObject(), selection.getRect(), this);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        U2Region window = dialog.getRegion();
        bool addToProject = dialog.getAddToProjFlag();
        QString path = dialog.getSavePath();
        QStringList seqNames = dialog.getSelectedSeqNames();
        Task* csTask = new CreateSubalignmentAndOpenViewTask(editor->getMSAObject(),
            CreateSubalignmentSettings(window, seqNames, path, true, addToProject, dialog.getFormatId()) );
        AppContext::getTaskScheduler()->registerTopLevelTask(csTask);
    }
}

void MSAEditorSequenceArea::sl_saveSequence(){
    int seqIndex = selection.y();

    if(selection.height() > 1){
        QMessageBox::critical(NULL, tr("Warning!"), tr("You must select only one sequence for export."));
        return;
    }

    QString seqName = editor->getMSAObject()->getMAlignment().getRow(seqIndex).getName();
    SaveSelectedSequenceFromMSADialogController d((QWidget*)AppContext::getMainWindow()->getQMainWindow());
    int rc = d.exec();
    if (rc == QDialog::Rejected) {
        return;
    }
    //TODO: OPTIMIZATION code below can be wrapped to task
    DNASequence seq;
    foreach(DNASequence s,  MSAUtils::ma2seq(editor->getMSAObject()->getMAlignment(), d.trimGapsFlag)){
        if (s.getName() == seqName){
            seq = s;
            break;
        }
    }

    U2OpStatus2Log  os;
    QString fullPath = GUrlUtils::prepareFileLocation(d.url, os);
    CHECK_OP(os, );
    GUrl url(fullPath);

    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(d.url));
    DocumentFormat *df = AppContext::getDocumentFormatRegistry()->getFormatById(d.format);
    Document *doc;
    QList<GObject*> objs;
    doc = df->createNewLoadedDocument(iof, fullPath, os);
    CHECK_OP_EXT(os, delete doc, );
    U2SequenceObject* seqObj = DocumentFormatUtils::addSequenceObjectDeprecated(doc->getDbiRef(), U2ObjectDbi::ROOT_FOLDER, seq.getName(), objs, seq, os);
    CHECK_OP_EXT(os, delete doc, );
    doc->addObject(seqObj);
    SaveDocumentTask *t = new SaveDocumentTask(doc, doc->getIOAdapterFactory(), doc->getURL());

    if (d.addToProjectFlag){
        Project *p = AppContext::getProject();
        Document *loadedDoc=p->findDocumentByURL(url);
        if (loadedDoc) {
            coreLog.details("The document already in the project");
            QMessageBox::warning(this, tr("warning"), tr("The document already in the project"));
            return;
        }
        p->addDocument(doc);

        // Open view for created document
        DocumentSelection ds;
        ds.setSelection(QList<Document*>() <<doc);
        MultiGSelection ms;
        ms.addSelection(&ds);
        foreach(GObjectViewFactory *f, AppContext::getObjectViewFactoryRegistry()->getAllFactories()) {
            if(f->canCreateView(ms)) {
                Task *tt = f->createViewTask(ms);
                AppContext::getTaskScheduler()->registerTopLevelTask(tt);
                break;
            }
        }
    }else{
        t->addFlag(SaveDoc_DestroyAfter);
    }
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

void MSAEditorSequenceArea::cancelSelection()
{
    MSAEditorSelection emptySelection;
    setSelection(emptySelection);
}

void MSAEditorSequenceArea::setSelectionHighlighting( bool highlight ) {
    if ( highlightSelection != highlight ) {
        highlightSelection = highlight;
        update( );
    }
}

void MSAEditorSequenceArea::updateHBarPosition( int base )
{
    if (isAlignmentEmpty()) {
        shBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
        return;
    }

    if (base <= getFirstVisibleBase() ) {
        shBar->setupRepeatAction(QAbstractSlider::SliderSingleStepSub, 50, 10);
    } else  if (base >= getLastVisibleBase(true)) {
        shBar->setupRepeatAction(QAbstractSlider::SliderSingleStepAdd, 50, 10);
    } else {
        shBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
    }
}

void MSAEditorSequenceArea::updateVBarPosition( int seq )
{
    if (isAlignmentEmpty()) {
        svBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
        return;
    }

    if (seq <= getFirstVisibleSequence()) {
        svBar->setupRepeatAction(QAbstractSlider::SliderSingleStepSub, 50, 10);
    } else if (seq >= getLastVisibleSequence(true)) {
        svBar->setupRepeatAction(QAbstractSlider::SliderSingleStepAdd, 50, 10);
    } else {
        svBar->setupRepeatAction(QAbstractSlider::SliderNoAction);
    }
}

void MSAEditorSequenceArea::sl_delCurrentSelection()
{
    deleteCurrentSelection();
}

U2Region MSAEditorSequenceArea::getSelectedRows() const {
    return ui->getCollapseModel()->mapSelectionRegionToRows(U2Region(selection.y(), selection.height()));
}

U2Region MSAEditorSequenceArea::getRowsAt(int pos) const {
    if (!ui->isCollapsibleMode()) {
        return U2Region(pos, 1);
    }

    MSACollapsibleItemModel* m = ui->getCollapseModel();
    int itemIdx = m->itemAt(pos);
    if (itemIdx >= 0) {
        const MSACollapsableItem& item = m->getItem(itemIdx);
        return U2Region(item.row, item.numRows);
    }
    return U2Region(m->mapToRow(pos), 1);
}

void MSAEditorSequenceArea::sl_copyCurrentSelection()
{
    // TODO: probably better solution would be to export selection???

    assert(isInRange(selection.topLeft()));
    assert(isInRange( QPoint(selection.x() + selection.width() - 1, selection.y() + selection.height() - 1) ) );

    MAlignmentObject* maObj = editor->getMSAObject();
    if ( selection.isNull() ) {
        return;
    }
    const MAlignment& msa = maObj->getMAlignment();
    const U2Region& sel = getSelectedRows();
    QString selText;
    U2OpStatus2Log os;
    for (int i = sel.startPos; i < sel.endPos(); ++i) {
        const MAlignmentRow& row = msa.getRow(i);
        int len = selection.width();
        QByteArray seqPart = row.mid(selection.x(), len, os).toByteArray(len, os);
        selText.append(seqPart);
        if (i + 1 != sel.endPos()) { // do not add line break into the last line
            selText.append("\n");
        }
    }
    QApplication::clipboard()->setText(selText);
}

bool MSAEditorSequenceArea::shiftSelectedRegion( int shift ) {
    if ( 0 == shift ) {
        return true;
    }

    MAlignmentObject *maObj = editor->getMSAObject( );
    if ( !maObj->isStateLocked( ) ) {
        const U2Region rows = getSelectedRows( );
        const int x = selection.x( );
        const int y = rows.startPos;
        const int width = selection.width( );
        const int height = rows.length;
        if ( maObj->isRegionEmpty( x, y, width, height ) ) {
            return true;
        }
        // backup current selection for the case when selection might disappear
        const MSAEditorSelection selectionBackup = selection;

        const int resultShift = maObj->shiftRegion( x, y, width, height, shift );
        if ( 0 != resultShift ) {
            int newCursorPosX = cursorPos.x( ) + resultShift >= 0
                ? cursorPos.x( ) + resultShift : 0;
            setCursorPos( newCursorPosX );

            const MSAEditorSelection newSelection( selectionBackup.x( ) + resultShift,
                selectionBackup.y( ), selectionBackup.width( ), selectionBackup.height( ) );
            setSelection( newSelection );
            if ( ( selectionBackup.getRect( ).right( ) == getLastVisibleBase( false )
                && resultShift > 0 )
                || ( selectionBackup.x( ) == getFirstVisibleBase( ) && 0 > resultShift ) )
            {
                setFirstVisibleBase( startPos + resultShift );
            }
            return true;
        } else {
            return false;
        }
    }
    return false;
}

void MSAEditorSequenceArea::deleteCurrentSelection()
{
    if (selection.isNull()) {
        return;
    }
    assert(isInRange(selection.topLeft()));
    assert(isInRange( QPoint(selection.x() + selection.width() - 1, selection.y() + selection.height() - 1) ) );
    MAlignmentObject* maObj = editor->getMSAObject();
    if (maObj == NULL || maObj->isStateLocked()) {
        return;
    }

    const QRect areaBeforeSelection( 0, 0, selection.x( ), selection.height( ) );
    const QRect areaAfterSelection( selection.x( ) + selection.width( ), selection.y( ),
        maObj->getLength( ) - selection.x( ) - selection.width( ), selection.height( ) );
    if ( maObj->isRegionEmpty( areaBeforeSelection.x( ), areaBeforeSelection.y( ),
            areaBeforeSelection.width( ), areaBeforeSelection.height( ) )
        && maObj->isRegionEmpty( areaAfterSelection.x( ), areaAfterSelection.y( ),
            areaAfterSelection.width( ), areaAfterSelection.height( ) )
        && selection.height( ) == maObj->getNumRows( ) )
    {
        return;
    }

    // if this method was invoked during a region shifting
    // then shifting should be canceled
    cancelShiftTracking( );

    U2OpStatusImpl os;
    U2UseCommonUserModStep userModStep(maObj->getEntityRef(), os);
    SAFE_POINT_OP(os, );

    const U2Region& sel = getSelectedRows();
    maObj->removeRegion(selection.x(), sel.startPos, selection.width(), sel.length, true);

    if (selection.height() == 1 && selection.width() == 1) {
        if (isInRange(selection.topLeft())) {
            return;
        }
    }
    cancelSelection();
}

void MSAEditorSequenceArea::sl_addSeqFromFile()
{
    MAlignmentObject* msaObject = editor->getMSAObject();
    if (msaObject->isStateLocked()) {
        return;
    }

    QString filter = DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::SEQUENCE, true);

    LastUsedDirHelper lod;
    QStringList urls;
#ifdef Q_OS_MAC
    if (qgetenv("UGENE_GUI_TEST").toInt() == 1 && qgetenv("UGENE_USE_NATIVE_DIALOGS").toInt() == 0) {
        urls = U2FileDialog::getOpenFileNames(this, tr("Open file with sequences"), lod.dir, filter, 0, QFileDialog::DontUseNativeDialog );
    } else
#endif
    urls = U2FileDialog::getOpenFileNames(this, tr("Open file with sequences"), lod.dir, filter);

    if (!urls.isEmpty()) {
        lod.url = urls.first();
        cancelSelection();
        AddSequencesToAlignmentTask * task = new AddSequencesToAlignmentTask(msaObject, urls);
        AppContext::getTaskScheduler()->registerTopLevelTask(task);
    }

}

void MSAEditorSequenceArea::sl_addSeqFromProject()
{
    MAlignmentObject* msaObject = editor->getMSAObject();
    if (msaObject->isStateLocked()) {
        return;
    }

    ProjectTreeControllerModeSettings settings;
    settings.objectTypesToShow.insert(GObjectTypes::SEQUENCE);
    QScopedPointer<U2SequenceObjectConstraints> seqConstraints(new U2SequenceObjectConstraints());
    seqConstraints->alphabetType = msaObject->getAlphabet()->getType();
    settings.objectConstraints.insert(seqConstraints.data());

    QList<GObject*> objects = ProjectTreeItemSelectorDialog::selectObjects(settings,this);

    foreach(GObject* obj, objects) {
        if (obj->isUnloaded()) {
            continue;
        }
        U2SequenceObject *seqObj = qobject_cast<U2SequenceObject*>(obj);
        if (seqObj) {
            U2OpStatus2Log os;
            editor->copyRowFromSequence(seqObj, os);
            cancelSelection();
        }
    }
}

void MSAEditorSequenceArea::sl_sortByName() {
    MAlignmentObject* msaObject = editor->getMSAObject();
    if (msaObject->isStateLocked()) {
        return;
    }
    MAlignment ma = msaObject->getMAlignment();
    ma.sortRowsByName();
    QStringList rowNames = ma.getRowNames();
    if (rowNames != msaObject->getMAlignment().getRowNames()) {
        U2OpStatusImpl os;
        msaObject->updateRowsOrder(ma.getRowsIds(), os);
        SAFE_POINT_OP(os, );
    }
    if (ui->isCollapsibleMode()) {
        sl_updateCollapsingMode();
    }
}

void MSAEditorSequenceArea::sl_setCollapsingMode(bool enabled) {
    MAlignmentObject* msaObject = editor->getMSAObject();
    int prevNumVisibleSequences = getNumVisibleSequences(false);
    if (msaObject == NULL  || msaObject->isStateLocked() ) {
        if (collapseModeSwitchAction->isChecked()) {
            collapseModeSwitchAction->setChecked(false);
            collapseModeUpdateAction->setEnabled(false);
        }
        return;
    }

    if (enabled) {
        collapseModeUpdateAction->setEnabled(true);
        sl_updateCollapsingMode();
    }
    else {
        collapseModeUpdateAction->setEnabled(false);

        MSACollapsibleItemModel *collapsibleModel = ui->getCollapseModel();
        SAFE_POINT(NULL != collapsibleModel, tr("NULL collapsible model!"), );
        collapsibleModel->reset();
    }

    ui->setCollapsibleMode(enabled);
    updateSelection();
    updateVScrollBar();
    int emptyRowsCount = prevNumVisibleSequences - getNumVisibleSequences(false);
    if(emptyRowsCount > 0) {
        setFirstVisibleSequence(qMax(getFirstVisibleSequence() - emptyRowsCount, 0));
    }
}

void MSAEditorSequenceArea::sl_updateCollapsingMode() {
    MAlignmentObject *msaObject = editor->getMSAObject();
    SAFE_POINT(NULL != msaObject, tr("NULL Msa Object!"), );

    MSACollapsibleItemModel *collapsibleModel = ui->getCollapseModel();

    Document *doc = msaObject->getDocument();
    SAFE_POINT(NULL != doc, tr("NULL document!"), );

    MAlignment ma = msaObject->getMAlignment();
    QVector<U2Region> unitedRows;
    bool sorted = ma.sortRowsBySimilarity(unitedRows);
    collapsibleModel->reset(unitedRows);

    U2OpStatusImpl os;
    if (sorted) {
        msaObject->updateRowsOrder(ma.getRowsIds(), os);
        SAFE_POINT_OP(os, );
    }

    MAlignmentModInfo mi;
    msaObject->updateCachedMAlignment(mi);
}

void MSAEditorSequenceArea::insertGapsBeforeSelection( int countOfGaps )
{
    if ( selection.isNull( ) || 0 == countOfGaps || -1 > countOfGaps ) {
        return;
    }
    SAFE_POINT( isInRange( selection.topLeft( ) ),
        tr("Top left corner of the selection has incorrect coords"), );
    SAFE_POINT( isInRange( QPoint( selection.x( ) + selection.width( ) - 1,
        selection.y( ) + selection.height( ) - 1 ) ),
        tr("Bottom right corner of the selection has incorrect coords"), );

    // if this method was invoked during a region shifting
    // then shifting should be canceled
    cancelShiftTracking( );

    MAlignmentObject *maObj = editor->getMSAObject( );
    if ( NULL == maObj || maObj->isStateLocked( ) ) {
        return;
    }
    U2OpStatus2Log os;
    U2UseCommonUserModStep userModStep( maObj->getEntityRef( ), os );
    SAFE_POINT_OP( os, );

    const MAlignment &msa = maObj->getMAlignment( );
    if ( selection.width( ) == msa.getLength( ) && selection.height( ) == msa.getNumRows( ) ) {
        return;
    }

    const int removedRegionWidth = ( -1 == countOfGaps ) ? selection.width( )
        : countOfGaps;
    const U2Region& sequences = getSelectedRows( );
    maObj->insertGap( sequences,  selection.x( ) , removedRegionWidth );
    moveSelection( removedRegionWidth, 0 );
}

void MSAEditorSequenceArea::reverseComplementModification(ModificationType& type) {
    if (type == ModificationType::NoType)
        return;
    MAlignmentObject* maObj = editor->getMSAObject();
    if (maObj == NULL || maObj->isStateLocked()) {
        return;
    }
    if (!maObj->getAlphabet()->isNucleic()) {
        return;
    }
    if (selection.height() == 0) {
        return;
    }
    assert(isInRange(selection.topLeft()));
    assert(isInRange( QPoint(selection.x() + selection.width() - 1,
        selection.y() + selection.height() - 1) ) );
    if ( !selection.isNull()) {
        // if this method was invoked during a region shifting
        // then shifting should be canceled
        cancelShiftTracking( );

        MAlignment ma = maObj->getMAlignment();
        DNATranslation* trans =
            AppContext::getDNATranslationRegistry()->lookupComplementTranslation(ma.getAlphabet());
        if (trans == NULL || !trans->isOne2One()) {
            return;
        }

        U2OpStatus2Log os;
        U2UseCommonUserModStep userModStep(maObj->getEntityRef(), os);
        SAFE_POINT_OP(os, );

        const U2Region& sel = getSelectedRows();

        QList<qint64> modifiedRowIds;
        modifiedRowIds.reserve( sel.length );
        for (int i = sel.startPos; i < sel.endPos(); i++) {
            const MAlignmentRow &currentRow = ma.getRow( i );
            QByteArray currentRowContent = currentRow.toByteArray(ma.getLength(), os);
            switch (type.getType())
            {
            case ModificationType::Reverse:
                TextUtils::reverse(currentRowContent.data(), currentRowContent.length());
                break;
            case ModificationType::Complement:
                trans->translate(currentRowContent.data(), currentRowContent.length());
                break;
            case ModificationType::ReverseComplement:
                TextUtils::reverse(currentRowContent.data(), currentRowContent.length());
                trans->translate(currentRowContent.data(), currentRowContent.length());
                break;
            }
            QString name = currentRow.getName();
            ModificationType oldType(ModificationType::NoType);
            if (name.endsWith("|revcompl")) {
                name.resize(name.length() - QString("|revcompl").length());
                oldType = ModificationType::ReverseComplement;
            } else if (name.endsWith("|compl")) {
                name.resize(name.length() - QString("|compl").length());
                oldType = ModificationType::Complement;
            } else if (name.endsWith("|rev")) {
                name.resize(name.length() - QString("|rev").length());
                oldType = ModificationType::Reverse;
            }
            ModificationType newType = type + oldType;
            switch (newType.getType())
            {
            case ModificationType::NoType:
                break;
            case ModificationType::Reverse:
                name.append("|rev");
                break;
            case ModificationType::Complement:
                name.append("|compl");
                break;
            case ModificationType::ReverseComplement:
                name.append("|revcompl");
                break;
            }

            // Split the sequence into gaps and chars
            QByteArray seqBytes;
            QList<U2MsaGap> gapModel;
            MsaDbiUtils::splitBytesToCharsAndGaps(currentRowContent, seqBytes, gapModel);

            maObj->updateRow(i, name, seqBytes, gapModel, os);
            modifiedRowIds << currentRow.getRowId( );
        }

        MAlignmentModInfo modInfo;
        modInfo.modifiedRowIds = modifiedRowIds;
        maObj->updateCachedMAlignment(modInfo);
    }
}

void MSAEditorSequenceArea::sl_reverseComplementCurrentSelection() {
    ModificationType type(ModificationType::ReverseComplement);
    reverseComplementModification(type);
}

void MSAEditorSequenceArea::sl_reverseCurrentSelection() {
    ModificationType type(ModificationType::Reverse);
    reverseComplementModification(type);
}

void MSAEditorSequenceArea::sl_complementCurrentSelection() {
    ModificationType type(ModificationType::Complement);
    reverseComplementModification(type);
}

QPair<QString, int> MSAEditorSequenceArea::getGappedColumnInfo() const{
    if (isAlignmentEmpty()) {
        return QPair<QString, int>(QString::number(0), 0);
    }

    const MAlignment& msa = editor->getMSAObject()->getMAlignment();
    const MAlignmentRow& row = msa.getRow(getSelectedRows().startPos);
    int len = row.getUngappedLength();
    QChar current = row.charAt(selection.topLeft().x());
    if(current == MAlignment_GapChar){
        return QPair<QString, int>(QString("gap"),len);
    }else{
        int pos = row.getUngappedPosition(selection.topLeft().x());
        return QPair<QString, int>(QString::number(pos + 1),len);
    }
}

void MSAEditorSequenceArea::sl_resetCollapsibleModel() {
    editor->resetCollapsibleModel();
}

void MSAEditorSequenceArea::sl_setCollapsingRegions(const QList<QStringList>& collapsedGroups) {
    MSACollapsibleItemModel* m = ui->getCollapseModel();
    SAFE_POINT(NULL != m, tr("Incorrect pointer to MSACollapsibleItemModel"),);
    m->reset();

    MAlignmentObject* msaObject = editor->getMSAObject();
    MAlignment ma = msaObject->getMAlignment();
    QStringList rowNames = ma.getRowNames();
    QVector<U2Region> collapsedRegions;

    //Calculate regions of the groups
    foreach(const QStringList& seqsGroup, collapsedGroups) {
        int regionStart = rowNames.size(), regionEnd = 0;
        foreach(const QString& seqName, seqsGroup) {
            int seqPos = rowNames.indexOf(seqName);
            regionStart = qMin(seqPos, regionStart);
            regionEnd = qMax(seqPos, regionEnd);
        }
        if(regionStart > 0 && regionEnd <= rowNames.size() && regionEnd > regionStart) {
            collapsedRegions.append(U2Region(regionStart, regionEnd - regionStart + 1));
        }
    }
    //Function 'reset' in 'MSACollapsibleItemModel' work only with sorted regions
    qSort(collapsedRegions);

    if (msaObject == NULL || msaObject->isStateLocked()) {
        if (collapseModeSwitchAction->isChecked()) {
            collapseModeSwitchAction->setChecked(false);
        }
        return;
    }

    ui->setCollapsibleMode(true);

    m->reset(collapsedRegions);

    MAlignmentModInfo mi;
    msaObject->updateCachedMAlignment(mi);

    updateVScrollBar();
}

int MSAEditorSequenceArea::getHeight(){
    return editor->getRowHeight() * (getNumVisibleSequences(true) - 1);
}

QString MSAEditorSequenceArea::exportHighligtning( int startPos, int endPos, int startingIndex, bool keepGaps, bool dots, bool transpose){
    QStringList result;

    MAlignmentObject* maObj = editor->getMSAObject();
    assert(maObj!=NULL);

    const MAlignment& msa = maObj->getMAlignment();
    const MSAEditor *editor = ui->getEditor();
    const MAlignment alignment = editor->getMSAObject()->getMAlignment();
    U2OpStatusImpl os;
    const int refSeq = alignment.getRowIndexByRowId(editor->getReferenceRowId(), os);
    const MAlignmentRow *r = NULL;
    if (MAlignmentRow::invalidRowId() != refSeq) {
        r = &(msa.getRow(refSeq));
    }

    QString header;
    header.append("Position\t");
    QString refSeqName = editor->getReferenceRowName();
    header.append(refSeqName);
    header.append("\t");
    foreach(QString name, maObj->getMAlignment().getRowNames()){
        if(name != refSeqName){
            header.append(name);
            header.append("\t");
        }
    }
    header.remove(header.length()-1,1);
    result.append(header);


    int posInResult = startingIndex;

    for (int pos = startPos-1; pos < endPos; pos++) {
        QString rowStr;
        rowStr.append(QString("%1").arg(posInResult));
        rowStr.append(QString("\t") + QString(msa.charAt(refSeq, pos)) + QString("\t"));
        bool informative = false;
        for (int seq = 0; seq < msa.getNumRows(); seq++) {  //FIXME possible problems when sequences have moved in view
            if (seq == refSeq) continue;
            char c = msa.charAt(seq, pos);

            const char refChar = r->charAt(pos);
            if (refChar == '-' && !keepGaps) continue;
            bool drawColor = false;
            highlightingScheme->setUseDots(useDotsAction->isChecked());
            highlightingScheme->process(refChar, c, drawColor, pos, seq);

            if (drawColor) {
                rowStr.append(c);
                informative = true;
            }else{
                if (dots){
                    rowStr.append(".");
                }else{
                    rowStr.append(" ");
                }
            }
            rowStr.append("\t");
        }
        if(informative){
            header.remove(rowStr.length()-1,1);
            result.append(rowStr);
        }
        posInResult++;
    }


    if (!transpose){
        QStringList transposedRows = TextUtils::transposeCSVRows(result, "\t");
        return transposedRows.join("\n");
    }

    return result.join("\n");
}

void MSAEditorSequenceArea::cancelShiftTracking( ) {
    shifting = false;
    selecting = false;
    shiftingWasPerformed = false;
    changeTracker.finishTracking( );
    editor->getMSAObject( )->releaseState( );
}

ExportHighligtningTask::ExportHighligtningTask( ExportHighligtningDialogController *dialog, MSAEditorSequenceArea *msaese_ )
    : Task(tr("Export highlighting"), TaskFlags_FOSCOE)
{
    msaese = msaese_;
    startPos = dialog->startPos;
    endPos = dialog->endPos;
    startingIndex = dialog->startingIndex;
    keepGaps = dialog->keepGaps;
    dots = dialog->dots;
    transpose = dialog->transpose;
    url = dialog->url;
}

void ExportHighligtningTask::run(){
    QString exportedData = msaese->exportHighligtning(startPos, endPos, startingIndex, keepGaps, dots, transpose);

    QFile resultFile(url.getURLString());
    CHECK_EXT(resultFile.open( QFile::WriteOnly | QFile::Truncate ), url.getURLString(),);
    QTextStream contentWriter(&resultFile);
    contentWriter << exportedData;
}

}//namespace
