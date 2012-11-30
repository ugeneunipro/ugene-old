/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_PROPERTYWIDGET_H_
#define _U2_PROPERTYWIDGET_H_

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QToolButton>
#include <QVariant>
#include <QWidget>

#include <U2Lang/ConfigurationEditor.h>

#include <U2Designer/URLLineEdit.h>

namespace U2 {

/************************************************************************/
/* DefaultPropertyWidget */
/************************************************************************/
static const int NO_LIMIT = -1;

class DefaultPropertyWidget : public PropertyWidget {
    Q_OBJECT
public:
    DefaultPropertyWidget(int maxLength = NO_LIMIT, QWidget *parent = NULL);
    virtual QVariant value();
    virtual void setValue(const QVariant &value);

private:
    QLineEdit *lineEdit;

private slots:
    void sl_valueChanged(const QString &value);
};

/************************************************************************/
/* SpinBoxWidget */
/************************************************************************/
class SpinBoxWidget : public PropertyWidget {
    Q_OBJECT
public:
    SpinBoxWidget(const QVariantMap &spinProperties, QWidget *parent = NULL);
    virtual QVariant value();
    virtual void setValue(const QVariant &value);

    /** Override */
    bool setProperty(const char *name, const QVariant &value);

signals:
    void valueChanged(int value);

private:
    QSpinBox *spinBox;

private slots:
    void sl_valueChanged(int value);
};

/************************************************************************/
/* DoubleSpinBoxWidget */
/************************************************************************/
class DoubleSpinBoxWidget : public PropertyWidget {
    Q_OBJECT
public:
    DoubleSpinBoxWidget(const QVariantMap &spinProperties, QWidget *parent = NULL);
    virtual QVariant value();
    virtual void setValue(const QVariant &value);

private:
    QDoubleSpinBox *spinBox;

private slots:
    void sl_valueChanged(double value);
};

/************************************************************************/
/* ComboBoxWidget */
/************************************************************************/
class ComboBoxWidget : public PropertyWidget {
    Q_OBJECT
public:
    ComboBoxWidget(const QVariantMap &items, QWidget *parent = NULL);
    virtual QVariant value();
    virtual void setValue(const QVariant &value);

    static ComboBoxWidget * createBooleanWidget(QWidget *parent = NULL);

signals:
    void valueChanged(const QString &value);

private:
    QComboBox *comboBox;

private slots:
    void sl_valueChanged(int index);
};

/************************************************************************/
/* URLWidget */
/************************************************************************/
class URLWidget : public PropertyWidget {
    Q_OBJECT
public:
    URLWidget(URLLineEdit *urlLine, QWidget *parent = NULL);
    virtual QVariant value();
    virtual void setValue(const QVariant &value);

signals:
    void finished();

private:
    URLLineEdit *urlLine;
    QToolButton *addButton;

private slots:
    void sl_textChanged(const QString &text);
};

} // U2

#endif // _U2_PROPERTYWIDGET_H_
