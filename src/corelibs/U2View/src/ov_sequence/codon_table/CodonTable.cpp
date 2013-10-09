#include "CodonTable.h"

#include <QtGui/QPixmap>
#include <QtGui/QScrollArea>
#include <QtGui/QScrollBar>
#include <QtGui/QSizePolicy>
#include <QtGui/QTableWidget>
#include <QtGui/QHeaderView>

#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>
#include <QtGui/QApplication>

#include <iostream>

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
    QTableWidget *table = new QTableWidget(18, 10);
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

    addItemToTable(table, 0, 0, "1st base", 2, 1);
    addItemToTable(table, 0, 1, "2nd base", 1, 8);
    addItemToTable(table, 0, 9, "3nd base", 2, 1);

    for (int i = 0; i < 4; i++) {
        // 1 column
        addItemToTable(table, 2 + i*4, 0, nucleobases[i], 4, 1);
        // 2 row
        addItemToTable(table, 1, 1 + 2*i, nucleobases[i], 1, 2);
        for (int j = 0; j < 4; j++) {
            // last column
            addItemToTable(table, 2 + i*4 + j, 9, nucleobases[j], 1, 1);
            for (int k = 0; k < 4; k++) {
                // codon variations
                addItemToTable(table, 2 + i*4 + k, 1 + j*2, nucleobases[i] + nucleobases[j] + nucleobases[k], 1, 1);
            }
        }
    }

    // "U" column
    addItemToTable(table, 2, 2, "(Phe/F) Phenylalanine", NONPOLAR_COLOR,"http://en.wikipedia.org/wiki/Phenylalanine", 2, 1);
    addItemToTable(table, 4, 2, "(Leu/L) Leucine", NONPOLAR_COLOR, "http://en.wikipedia.org/wiki/Leucine", 6, 1);
    addItemToTable(table, 10, 2, "(Ile/I) Isoleucine", NONPOLAR_COLOR, "http://en.wikipedia.org/wiki/Isoleucine", 3, 1);
    addItemToTable(table, 13, 2, "(Met/M) Methionine", NONPOLAR_COLOR, "http://en.wikipedia.org/wiki/Methionine");
    addItemToTable(table, 14, 2, "(Val/V) Valine", NONPOLAR_COLOR, "http://en.wikipedia.org/wiki/Valine", 4, 1);

    // "C" column
    addItemToTable(table, 2, 4, "(Ser/S) Serine", POLAR_COLOR, "http://en.wikipedia.org/wiki/Serine", 4, 1);
    addItemToTable(table, 6, 4, "(Pro/P) Proline", NONPOLAR_COLOR, "http://en.wikipedia.org/wiki/Proline", 4, 1);
    addItemToTable(table, 10, 4, "(Thr/T) Threonine", POLAR_COLOR, "http://en.wikipedia.org/wiki/Threonine", 4, 1);
    addItemToTable(table, 14, 4, "(Ala/A) Alanine", NONPOLAR_COLOR, "http://en.wikipedia.org/wiki/Alanine", 4, 1);

    // "A" column
    addItemToTable(table, 2, 6, "(Tyr/Y) Tyrosine", POLAR_COLOR, "http://en.wikipedia.org/wiki/Tyrosine", 2, 1);
    addItemToTable(table, 4, 6, "Stop (Ochre)", STOP_CODON_COLOR, "http://en.wikipedia.org/wiki/Stop_codon");
    addItemToTable(table, 5, 6, "Stop (Amber)", STOP_CODON_COLOR, "http://en.wikipedia.org/wiki/Stop_codon");
    addItemToTable(table, 6, 6, "(His/H) Histidine", BASIC_COLOR, "http://en.wikipedia.org/wiki/Histidine", 2, 1);
    addItemToTable(table, 8, 6, "(Gln/Q) Glutamine", POLAR_COLOR, "http://en.wikipedia.org/wiki/Glutamine", 2, 1);
    addItemToTable(table, 10, 6, "(Asn/N) Asparagine", POLAR_COLOR, "http://en.wikipedia.org/wiki/Asparagine", 2, 1);
    addItemToTable(table, 12, 6, "(Lys/K) Lysine", BASIC_COLOR, "http://en.wikipedia.org/wiki/Lysine", 2, 1);
    addItemToTable(table, 14, 6, "(Asp/D) Aspartic acid", ACIDIC_COLOR, "http://en.wikipedia.org/wiki/Aspartic_acid", 2, 1);
    addItemToTable(table, 16, 6, "(Glu/E) Glutamic acid", ACIDIC_COLOR, "http://en.wikipedia.org/wiki/Glutamic_acid", 2, 1);

    // "G" column
    addItemToTable(table, 2, 8, "(Cys/C) Cysteine", POLAR_COLOR, "http://en.wikipedia.org/wiki/Cysteine", 2, 1);
    addItemToTable(table, 4, 8, "Stop (Opal)", STOP_CODON_COLOR, "http://en.wikipedia.org/wiki/Stop_codon");
    addItemToTable(table, 5, 8, "(Trp/W) Tryptophan ", NONPOLAR_COLOR, "http://en.wikipedia.org/wiki/Tryptophan");
    addItemToTable(table, 6, 8, "(Arg/R) Arginine", BASIC_COLOR, "http://en.wikipedia.org/wiki/Arginine", 4, 1);
    addItemToTable(table, 10, 8, "(Ser/S) Serine", POLAR_COLOR, "http://en.wikipedia.org/wiki/Serine", 2, 1);
    addItemToTable(table, 12, 8, "(Arg/R) Arginine", BASIC_COLOR, "http://en.wikipedia.org/wiki/Arginine", 2, 1);
    addItemToTable(table, 14, 8, "(Gly/G) Glycine", NONPOLAR_COLOR, "http://en.wikipedia.org/wiki/Glycine", 4, 1);

    QVBoxLayout *l = new QVBoxLayout(this);
    l->addWidget(table);

    l->setMargin(0);
    l->setSpacing(0);

    setMaximumHeight(rowHeight*18 + 10);
    setMinimumHeight(70);

    setLayout(l);
    setVisible(false);
}

void CodonTableView::sl_setVisible() {
    setVisible(!isVisible());
}

void CodonTableView::addItemToTable(QTableWidget *table, int row, int column, QString text, QColor backgroundColor, int rowSpan, int columnSpan) {
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

void CodonTableView::addItemToTable(QTableWidget *table, int row, int column, QString text, int rowSpan, int columnSpan) {
    addItemToTable(table, row, column, text, QColor(0, 0, 0, 0), rowSpan, columnSpan);
}

void CodonTableView::addItemToTable(QTableWidget *table, int row, int column, QString text, QColor backgroundColor, QString link, int rowSpan, int columnSpan) {

    QColor appTextColor = QApplication::palette().text().color();
    QLabel *item = new QLabel("<a href=\"" + link + "\" style=\"color: " + appTextColor.name() + "\">" + text +"</a>");

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

// CodonTableAction
CodonTableAction::CodonTableAction(CodonTableView *_view)
    : ADVSequenceWidgetAction("codon_table_action", "CT"),
      view(_view)
{
    addToBar = true;
    connect(this, SIGNAL(triggered()), view, SLOT(sl_setVisible()));
}

}
