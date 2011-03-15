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

#include "WorkflowSamples.h"
#include "WorkflowViewController.h"
#include "HRSceneSerializer.h"
#include <util/SaveSchemaImageUtils.h>
#include <U2Core/Log.h>
#include <U2Core/L10n.h>
#include <U2Core/Settings.h>

#include <U2Designer/WorkflowGUIUtils.h>
#include <U2Lang/WorkflowUtils.h>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QUrl>
#include <QtCore/QTextStream>
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QToolButton>

#include <QtGui/QHeaderView>
#include <QtGui/QApplication>
#include <QtCore/QAbstractItemModel>
#include <QtGui/QTreeView>
#include <QtGui/QStyle>
#include <QtGui/QPainter>
#include <QtGui/QStyledItemDelegate>
#include <QtGui/QVBoxLayout>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QTextDocument>

Q_DECLARE_METATYPE(QTextDocument*);

namespace U2 {

const QString SamplesWidget::MIME_TYPE("application/x-ugene-sample-id");
QList<SampleCategory> SampleRegistry::data;

static QPixmap genSnap(const QDomDocument& xml);

#define DATA_ROLE Qt::UserRole
#define INFO_ROLE Qt::UserRole + 1
#define ICON_ROLE Qt::UserRole + 2
#define DOC_ROLE Qt::UserRole + 3

class SampleDelegate : public QStyledItemDelegate {
public:
    SampleDelegate(QObject* parent = 0) : QStyledItemDelegate(parent){}
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QVariant value = index.data(Qt::SizeHintRole);
        if (value.isValid())
            return qvariant_cast<QSize>(value);

        QStyleOptionViewItemV4 opt = option;
        initStyleOption(&opt, index);
        const QWidget *widget = qobject_cast<QWidget*>(parent());//QStyledItemDelegatePrivate::widget(option);
        QStyle *style = widget ? widget->style() : QApplication::style();
        opt.rect.setSize(widget->size());
        return style->sizeFromContents(QStyle::CT_ItemViewItem, &opt, QSize(), widget);
    }

//     void QStyledItemDelegate::paint(QPainter *painter,
//         const QStyleOptionViewItem &option, const QModelIndex &index) const
//     {
//         Q_ASSERT(index.isValid());
// 
//         QStyleOptionViewItemV4 opt = option;
//         initStyleOption(&opt, index);
// 
//         const QWidget *widget = qobject_cast<QWidget*>(parent());//QStyledItemDelegatePrivate::widget(option);
//         QStyle *style = widget ? widget->style() : QApplication::style();
//         style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
//     }

    /*void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
    {
        QStyledItemDelegate::initStyleOption(option, index);
        QWidget* owner;
        if ((owner = qobject_cast<QWidget*>(parent()))) {
            option->rect.setWidth(owner->width());
            option->rect.setHeight(0);
        }
//         QVariant value = index.data(Qt::FontRole);
//         if (value.isValid() && !value.isNull()) {
//             option->font = qvariant_cast<QFont>(value).resolve(option->font);
//             option->fontMetrics = QFontMetrics(option->font);
//         }
// 
//         value = index.data(Qt::TextAlignmentRole);
//         if (value.isValid() && !value.isNull())
//             option->displayAlignment = (Qt::Alignment)value.toInt();
// 
//         value = index.data(Qt::ForegroundRole);
//         if (qVariantCanConvert<QBrush>(value))
//             option->palette.setBrush(QPalette::Text, qvariant_cast<QBrush>(value));

        if (QStyleOptionViewItemV2 *v2 = qstyleoption_cast<QStyleOptionViewItemV2 *>(option)) {
            v2->features |= QStyleOptionViewItemV2::WrapText;
        }
    }*/
};

SamplesWidget::SamplesWidget(WorkflowScene *scene, QWidget *parent) : QTreeWidget(parent) {
    setColumnCount(1);
    //header()->hide();
    setHeaderHidden(true);
    setItemDelegate(new SampleDelegate(this));
    setWordWrap(true);

    foreach(const SampleCategory& cat, SampleRegistry::getCategories()) {
        addCategory(cat);
    }

    expandAll();

    glass = new SamplePane(scene);

    //connect(this, SIGNAL(itemActivated(QTreeWidgetItem*,int)), SLOT(handleItem(QTreeWidgetItem*)));
    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(handleTreeItem(QTreeWidgetItem*)));
    //connect(this, SIGNAL(itemSelectionChanged()), glass, SLOT(test()));
    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(activateItem(QTreeWidgetItem*)));
    connect(glass, SIGNAL(itemActivated(QTreeWidgetItem*)), SLOT(activateItem(QTreeWidgetItem*)));
    connect(glass, SIGNAL(cancel()), SLOT(cancelItem()));
}

void SamplesWidget::activateItem(QTreeWidgetItem * item) {
    if (item && item->data(0, DATA_ROLE).isValid()) {
        emit sampleSelected(item->data(0, DATA_ROLE).toString());
    }
}

void SamplesWidget::handleTreeItem(QTreeWidgetItem * item) {
//     bool show = false;
     if (item && !item->data(0, DATA_ROLE).isValid()) {
         item = NULL;
//         glass->setItem(item);
//         show = true;
     }
//     emit setupGlass(show ? glass : NULL);

    glass->setItem(item);
    emit setupGlass(glass);
}

void SamplesWidget::cancelItem() {
     selectionModel()->clear();
     if (isHidden()) {
         emit setupGlass(NULL);
         glass->setItem(NULL);
     } else {
         emit setupGlass(glass);
     }
}

void SamplesWidget::addCategory( const SampleCategory& cat )
{
    QTreeWidgetItem* ci = new QTreeWidgetItem(this, QStringList(cat.d.getDisplayName()));
    ci->setFlags(Qt::ItemIsEnabled);
    QFont cf;
    cf.setBold(true);
    ci->setData(0, Qt::FontRole, cf);
    ci->setData(0, Qt::BackgroundRole, QColor(255,255,160, 127));
    //QFont sf;
    //sf.setBold(true);
    //sf.setItalic(true);

    foreach(const Sample& item, cat.items) {
        QTreeWidgetItem* ib = new QTreeWidgetItem(ci, QStringList(item.d.getDisplayName()));
        ib->setData(0, DATA_ROLE, item.content);
        //ib->setData(0, ICON_ROLE, item.ico.pixmap(200));
        //ib->setData(0, INFO_ROLE, qVariantFromValue<Descriptor>(item.d));
        QTextDocument* doc = new QTextDocument(this);
        ib->setData(0, DOC_ROLE, qVariantFromValue<QTextDocument*>(doc));
        //ib->setData(0, Qt::FontRole, sf);
        Descriptor d = item.d;
        QIcon ico = item.ico;
        if (ico.isNull()) {
            ico.addPixmap(SaveSchemaImageUtils::generateSchemaSnapshot(item.content.toUtf8()));
        }
        DesignerGUIUtils::setupSamplesDocument(d, ico, doc);
    }
}

void SamplePane::setItem(QTreeWidgetItem* it) {
    item = it;
//     if (!item) {
//         m_document->clear();
//         return;
//     }
//     Descriptor d = it->data(0, INFO_ROLE).value<Descriptor>();
//     QString text = 
//         "<html>"
//         "<table align='center' border='0'>"
//         "<tr><td><h1 align='center'>%1</h1></td></tr>"
//         "<tr><td><img src=\"%2\"/>%3</td></tr>"
//         "<tr><td bgcolor='gainsboro' align='center'><font color='maroon' size='+2'>%4</font></td></tr>"
//         "</table>"
//         "</html>";
//     QString img("img://img");
//     m_document->addResource(QTextDocument::ImageResource, QUrl(img), it->data(0, ICON_ROLE));
//     
//     //QString img = QString("<table align='left' width='250' border='0'><tr><td><img src=\"img://img\"/></td></tr></table>");
//     QString body = Qt::escape(d.getDocumentation()).replace("\n", "<br>");
//     text = text.arg(d.getDisplayName()).arg(img).arg(body).arg(tr("Double click to load the sample"));
//     m_document->setHtml(text);
}

void SamplePane::test() {
    uiLog.error("Acha!!!");
}

void SamplePane::mouseDoubleClickEvent( QMouseEvent *e) {
    if (!item) {
        return;
    }

    QTextDocument* doc = item->data(0, DOC_ROLE).value<QTextDocument*>();
    int pageWidth = qMax(width() - 100, 100);
    int pageHeight = qMax(height() - 100, 100);
    if (pageWidth != doc->pageSize().width()) {
        doc->setPageSize(QSize(pageWidth, pageHeight));
    }

    QSize ts = doc->size().toSize();
    QRect textRect;
    textRect.setSize(ts);

    QPoint position = e->pos();
    if(textRect.contains(position)) {
        emit itemActivated(item);
    }
    else {
        item = NULL;
        scene->update();
    }
}

void SamplePane::keyPressEvent(QKeyEvent * event ){
    if (event->key() == Qt::Key_Escape) {
        emit cancel();
    } else if (event->key() == Qt::Key_Enter) {
        emit itemActivated(item);
    }
}

SamplePane::SamplePane(WorkflowScene *_scene) : item(NULL), scene(_scene) {
    m_document = new QTextDocument(this);
}

void SamplePane::paint(QPainter* painter)
{
    if (!item && !scene->items().size()) {
        DesignerGUIUtils::paintSamplesArrow(painter);
        return;
    }

    if(item) {
        QTextDocument* doc = item->data(0, DOC_ROLE).value<QTextDocument*>();
        DesignerGUIUtils::paintSamplesDocument(painter, doc, width(), height(), palette());
    }
}


LoadSamplesTask::LoadSamplesTask( const QStringList& lst) 
: Task(tr("Load workflow samples"), TaskFlag_None), dirs(lst) {}

void LoadSamplesTask::run() {
    foreach(const QString& s, dirs) {
        scanDir(s);
    }
}

void LoadSamplesTask::scanDir( const QString& s) {
    QDir dir(s);
    if (!dir.exists()) {
        ioLog.error(tr("Sample dir does not exist: %1").arg(s));
        return;
    }
    SampleCategory category(s,dir.dirName());
    QStringList names;
    foreach(const QString & ext, WorkflowUtils::WD_FILE_EXTENSIONS) {
        names << "*." + ext;
    }
    
    foreach(const QFileInfo& fi, dir.entryInfoList(names, QDir::Files|QDir::NoSymLinks)) {
        QFile f(fi.absoluteFilePath());
        if(!f.open(QIODevice::ReadOnly)) {
            ioLog.error(tr("Failed to load sample: %1").arg(L10N::errorOpeningFileRead(fi.absoluteFilePath())));
            continue;
        }
        
        QTextStream in(&f);
        in.setCodec("UTF-8");
        Sample sample;
        sample.content = in.readAll();
        
        Metadata meta;
        QString err = HRSceneSerializer::string2Scene(sample.content, NULL, &meta);
        if(!err.isEmpty()) {
            coreLog.error(tr("Failed to load sample: %1").arg(err));
            continue;
        }
        sample.d = Descriptor(fi.absoluteFilePath(), meta.name.isEmpty()? fi.baseName() : meta.name, meta.comment);
        
        QString icoName = dir.absoluteFilePath(fi.baseName() + ".png");
        if (QFile::exists(icoName)) {
            sample.ico.addFile(icoName);
        }
        category.items << sample;
    }
    if (!category.items.isEmpty()) {
        result << category;
    }
    foreach(const QFileInfo& fi, dir.entryInfoList(QStringList(), QDir::AllDirs|QDir::NoSymLinks|QDir::NoDotAndDotDot)) {
        scanDir(fi.absoluteFilePath());
    }
}

Task::ReportResult LoadSamplesTask::report()
{
    SampleRegistry::data = result;
    return ReportResult_Finished;
}

Task* SampleRegistry::init( const QStringList& lst) {
    return new LoadSamplesTask(lst);
}

} //namespace
