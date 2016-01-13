/**
* UGENE - Integrated Bioinformatics Tools.
* Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#ifndef _CFG_EXTERNAL_TOOL_MODEL_H_
#define _CFG_EXTERNAL_TOOL_MODEL_H_

#include <QtCore/QAbstractTableModel>

#include <U2Lang/ExternalToolCfg.h>

namespace U2 {

class DataTypeRegistry;
class DocumentFormatRegistry;
class PropertyDelegate;

class CfgExternalToolItem {
public:
    CfgExternalToolItem();
    ~CfgExternalToolItem();

    QString getDataType() const;
    void setDataType(const QString& id);

    QString getName() const;
    void setName(const QString &_name);

    QString getFormat() const;
    void setFormat(const QString & f);

    QString getDescription() const;
    void setDescription(const QString & _descr);

    PropertyDelegate *delegateForTypes;
    PropertyDelegate *delegateForFormats;

    DataConfig itemData;

private:
    DocumentFormatRegistry *dfr;
    DataTypeRegistry *dtr;
};


class CfgExternalToolModel : public QAbstractTableModel {
    Q_OBJECT
public:
    CfgExternalToolModel(bool isInput, QObject *obj = NULL);

    int rowCount(const QModelIndex & /* = QModelIndex */) const;
    int columnCount(const QModelIndex & /* = QModelIndex */) const;
    Qt::ItemFlags flags(const QModelIndex &) const;
    CfgExternalToolItem* getItem(const QModelIndex &index) const;
    QList<CfgExternalToolItem*> getItems() const;
    QVariant data(const QModelIndex &index, int role /* = Qt::DisplayRole */) const;
    void createFormatDelegate(const QString &newType, CfgExternalToolItem *item);
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    bool insertRows(int row, int count = 0, const QModelIndex & parent = QModelIndex());
    bool removeRows(int row, int count = 0, const QModelIndex & parent = QModelIndex());

private:
    void init();
    void initFormats();
    void initTypes();

    bool isInput;
    QList<CfgExternalToolItem*> items;
    QVariantMap types;
    QVariantMap seqFormatsW;
    QVariantMap msaFormatsW;
    QVariantMap annFormatsW;
    QVariantMap seqFormatsR;
    QVariantMap msaFormatsR;
    QVariantMap annFormatsR;
    QVariantMap textFormat;
};


class AttributeItem {
public:
    QString getName() const;
    void setName(const QString& _name);
    QString getDataType() const;
    void setDataType(const QString &_type);
    QString getDescription() const;
    void setDescription(const QString &_description);
private:
    QString name;
    QString type;
    QString description;

};

class CfgExternalToolModelAttributes : public QAbstractTableModel {
    Q_OBJECT
public:
    CfgExternalToolModelAttributes();
    ~CfgExternalToolModelAttributes();

    int rowCount(const QModelIndex & /* = QModelIndex */) const;
    int columnCount(const QModelIndex & /* = QModelIndex */) const;
    Qt::ItemFlags flags(const QModelIndex &) const;
    AttributeItem* getItem(const QModelIndex &index) const;
    QList<AttributeItem*> getItems() const;
    QVariant data(const QModelIndex &index, int role /* = Qt::DisplayRole */) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    bool insertRows(int row, int count = 0, const QModelIndex & parent = QModelIndex());
    bool removeRows(int row, int count = 0, const QModelIndex & parent = QModelIndex());

private:
    QList<AttributeItem*> items;
    PropertyDelegate *delegate;
    QVariantMap types;
};

} // U2

#endif // _CFG_EXTERNAL_TOOL_MODEL_H_
