#include "CodonTable.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DNATranslationImpl.h>
#include <U2Core/U2SafePoints.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/AnnotatedDNAView.h>

#include <QtCore/QUrl>
#include <QtGui/QScrollArea>
#include <QtGui/QScrollBar>
#include <QtGui/QSizePolicy>
#include <QtGui/QTableWidget>
#include <QtGui/QHeaderView>
#include <QtGui/QDesktopServices>
#include <QtGui/QApplication>

namespace U2 {

const QColor CodonTableView::NONPOLAR_COLOR = QColor("#FFEE00").lighter();
const QColor CodonTableView::POLAR_COLOR = QColor("#3DF490").lighter();
const QColor CodonTableView::BASIC_COLOR = QColor("#FF5082").lighter();
const QColor CodonTableView::ACIDIC_COLOR = QColor("#00ABED").lighter();
const QColor CodonTableView::STOP_CODON_COLOR = QColor(Qt::gray);

// CodonTableView
CodonTableView::CodonTableView(AnnotatedDNAView *view)
    : ADVSplitWidget(view)
{
    table = new QTableWidget(18, 10);
    table->setSelectionMode(QAbstractItemView::NoSelection);

    table->horizontalHeader()->hide();
    table->verticalHeader()->hide();

    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setResizeMode(QHeaderView::Fixed);

    const int rowHeight = 18;
    table->verticalHeader()->setDefaultSectionSize(rowHeight);

    table->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setResizeMode(3, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setResizeMode(5, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setResizeMode(7, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setResizeMode(9, QHeaderView::ResizeToContents);

    // Fill the table
    QStringList nucleobases;
    nucleobases << "U" << "C" << "A" << "G";
    addItemToTable(0, 0, tr("1st base"), 2, 1);
    addItemToTable(0, 1, tr("2nd base"), 1, 8);
    addItemToTable(0, 9, tr("3nd base"), 2, 1);
    for (int i = 0; i < 4; i++) {
        // 1 column
        addItemToTable(2 + i*4, 0, nucleobases[i], 4, 1);
        // 2 row
        addItemToTable(1, 1 + 2*i, nucleobases[i], 1, 2);
        for (int j = 0; j < 4; j++) {
            // last column
            addItemToTable(2 + i*4 + j, 9, nucleobases[j], 1, 1);
            for (int k = 0; k < 4; k++) {
                // codon variations
                addItemToTable(2 + i*4 + k, 1 + j*2, nucleobases[i] + nucleobases[j] + nucleobases[k], 1, 1);
            }
        }
    }

    QVBoxLayout *l = new QVBoxLayout(this);
    l->addWidget(table);
    l->setMargin(0);
    l->setSpacing(0);

    setMaximumHeight(rowHeight*18 + 10);
    setMinimumHeight(70);

    setLayout(l);
    setVisible(false);

    QList<ADVSequenceObjectContext*> list = view->getSequenceContexts();
    foreach(ADVSequenceObjectContext* ctx, list) {
        // find first with translation table
        if (ctx->getAminoTT() != NULL) {
            setAminoTranslation(ctx->getAminoTT()->getTranslationId());
            return;
        }
    }
    // set standart genetic code table
    setAminoTranslation(DNATranslationID(1));
}

void CodonTableView::sl_setVisible() {
    setVisible(!isVisible());
}

void CodonTableView::sl_setAminoTranslation() {
    QAction* a = qobject_cast<QAction*>(sender());
    if (a != NULL) {
        QString tid = a->data().toString();
        setAminoTranslation(tid);
        table->resize(table->width() - 1, table->height());
        table->updateGeometry();
    }
}

void CodonTableView::sl_onSequenceFocusChanged(ADVSequenceWidget */*from*/, ADVSequenceWidget *to) {
    CHECK(to != NULL, );

    const QList<ADVSequenceObjectContext*> ctx = to->getSequenceContexts();
    CHECK(ctx.first() != NULL, );
    CHECK(ctx.first()->getAminoTT() != NULL, );

    setAminoTranslation(ctx.first()->getAminoTT()->getTranslationId());
    table->repaint();
}

void CodonTableView::setAminoTranslation(const QString& trId) {
    DNATranslationRegistry* trReg = AppContext::getDNATranslationRegistry();
    SAFE_POINT(trReg != NULL, "DNATranslationRegistry is NULL!", );

    DNAAlphabetRegistry* alphReg = AppContext::getDNAAlphabetRegistry();
    SAFE_POINT(alphReg != NULL, "DNAAlphabetRegistry is NULL!", );
    const DNAAlphabet* alph = alphReg->findById(BaseDNAAlphabetIds::NUCL_RNA_DEFAULT());
    SAFE_POINT(alph != NULL, "Standart RNA alphabet not found!", );

    DNATranslation* tr = trReg->lookupTranslation(alph, trId);
    SAFE_POINT(tr != NULL, "No translation found!", );

    QList <char> nucleobases;
    nucleobases << 'U' << 'C' << 'A' << 'G';
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                char codon = tr->translate3to1(nucleobases[i], nucleobases[j], nucleobases[k]);
                addItemToTable(2 + i*4 + k, 2 + j*2, trReg->lookupCodon(codon));
            }
        }
    }
    spanEqualCells();
};

void CodonTableView::spanEqualCells() {
    table->clearSpans();
    table->setSpan(0, 0, 2, 1);
    table->setSpan(0, 1, 1, 8);
    table->setSpan(0, 9, 2, 1);
    for (int i = 0; i < 4; i++) {
        table->setSpan(2 + i*4, 0, 4, 1);
        table->setSpan(1, 1 + 2*i, 1, 2);
        int rowSpan = 1;
        for (int j = 2; j < table->rowCount() - 1; j++) {
            if ((table->cellWidget(j, 2 + i*2) == NULL) || (table->cellWidget(j + 1, 2 + i*2) == NULL)) {
                continue;
            }

            if (table->cellWidget(j, 2 + i*2)->objectName() == table->cellWidget(j + 1, 2 + i*2)->objectName()) {
                rowSpan++;
            } else {
                if (rowSpan != 1) {
                    table->setSpan(j - rowSpan + 1, 2 + i*2, rowSpan, 1);
                }
                rowSpan = 1;
            }
        }
        if (rowSpan != 1) {
            table->setSpan(17 - rowSpan + 1, 2 + i*2, rowSpan, 1);
        }
    }
}

void CodonTableView::addItemToTable(int row, int column, QString text, QColor backgroundColor, int rowSpan, int columnSpan) {
    QTableWidgetItem *item = new QTableWidgetItem(text);

    QFont font = item->font();
    font.setPointSize(10);
    item->setFont(font);

    item->setFlags(item->flags() & ~Qt::ItemIsEditable);

    item->setBackgroundColor(backgroundColor);
    item->setTextAlignment(Qt::AlignCenter);
    if ((rowSpan != 1) || (columnSpan != 1))
        table->setSpan(row, column, rowSpan, columnSpan);
    table->setItem(row, column, item);
}

void CodonTableView::addItemToTable(int row, int column, QString text, int rowSpan, int columnSpan) {
    addItemToTable(row, column, text, QColor(0, 0, 0, 0), rowSpan, columnSpan);
}

void CodonTableView::addItemToTable(int row, int column, QString text, QColor backgroundColor, QString link, int rowSpan, int columnSpan) {
    table->removeCellWidget(row, column);

    QColor appTextColor = QApplication::palette().text().color();
    QLabel *item = new QLabel("<a href=\"" + link + "\" style=\"color: " + appTextColor.name() + "\">" + text +"</a>");
    item->setObjectName(text);

    item->setAlignment(Qt::AlignCenter);
    item->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);

    QFont font = item->font();
    font.setPointSize(10);
    item->setFont(font);
    item->setStyleSheet("QLabel {background-color: " + backgroundColor.name() + ";}");

    item->setOpenExternalLinks(true);
    item->setTextFormat(Qt::RichText);
    item->setTextInteractionFlags(Qt::TextBrowserInteraction);

    if ((rowSpan != 1) || (columnSpan != 1))
        table->setSpan(row, column, rowSpan, columnSpan);

    if (backgroundColor == POLAR_COLOR) {
        item->setToolTip("Polar Codon");
    }
    if (backgroundColor == NONPOLAR_COLOR) {
        item->setToolTip("Nonpolar Codon");
    }
    if (backgroundColor == BASIC_COLOR) {
        item->setToolTip("Basic Codon");
    }
    if (backgroundColor == ACIDIC_COLOR) {
        item->setToolTip("Acidic Codon");
    }
    if (backgroundColor == STOP_CODON_COLOR) {
        item->setToolTip("Stop Codon");
    }

    table->setCellWidget(row, column, item);
}

void CodonTableView::addItemToTable(int row, int column, DNACodon *codon) {
    CHECK(codon != NULL, )
    addItemToTable(row, column,
                   codon->getFullName() + " (" + codon->getTreeLetterCode() + ")",
                   getColor(codon->getCodonGroup()),
                   codon->getLink());
}

QColor CodonTableView::getColor(DNACodonGroup gr) {
    if (gr == DNACodonGroup_POLAR) {
        return POLAR_COLOR;
    }
    if (gr == DNACodonGroup_NONPOLAR) {
        return NONPOLAR_COLOR;
    }
    if (gr == DNACodonGroup_BASIC) {
        return BASIC_COLOR;
    }
    if (gr == DNACodonGroup_ACIDIC) {
        return ACIDIC_COLOR;
    }
    if (gr == DNACodonGroup_STOP) {
        return STOP_CODON_COLOR;
    }
    return QColor();
}

// CodonTableAction
CodonTableAction::CodonTableAction(CodonTableView *_view)
    : ADVSequenceWidgetAction("codon_table_action", "CT"),
      view(_view)
{
    addToBar = true;
    connect(this, SIGNAL(triggered()), view, SLOT(sl_setVisible()));
}

}
