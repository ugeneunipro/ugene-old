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

#ifndef _U2_WIZARDWIDGET_H_
#define _U2_WIZARDWIDGET_H_

#include <U2Core/U2OpStatus.h>

#include <U2Lang/ActorModel.h>
#include <U2Lang/Attribute.h>
#include <U2Lang/URLAttribute.h>

namespace U2 {

class WizardWidgetVisitor;

using namespace Workflow;

/************************************************************************/
/* WizardWidget */
/************************************************************************/
class U2LANG_EXPORT WizardWidget {
public:
    WizardWidget();
    virtual ~WizardWidget();

    virtual void accept(WizardWidgetVisitor *visitor) = 0;
    virtual void validate(const QList<Actor*> &actors, U2OpStatus &os) const;
};

/************************************************************************/
/* LogoWidget */
/************************************************************************/
class U2LANG_EXPORT LogoWidget : public WizardWidget {
public:
    LogoWidget(const QString &logoPath = "");
    virtual ~LogoWidget();

    virtual void accept(WizardWidgetVisitor *visitor);

    void setLogoPath(const QString &value);
    const QString & getLogoPath() const;
    bool isDefault() const;

    static const QString ID;

private:
    QString logoPath;
};

/************************************************************************/
/* WidgetsArea */
/************************************************************************/
class U2LANG_EXPORT WidgetsArea : public WizardWidget {
public:
    WidgetsArea(const QString &name);
    WidgetsArea(const QString &name, const QString &title);
    virtual ~WidgetsArea();

    virtual void accept(WizardWidgetVisitor *visitor);
    virtual void validate(const QList<Actor*> &actors, U2OpStatus &os) const;

    void addWidget(WizardWidget *widget);
    const QList<WizardWidget*> & getWidgets() const;

    const QString & getName() const;
    const QString & getTitle() const;
    void setTitle(const QString &value);

    bool hasLabelSize() const;
    int getLabelSize() const;
    void setLabelSize(int value);

private:
    bool titleable;
    QString name;
    QString title;
    int labelSize; // px
    QList<WizardWidget*> widgets;
};

/************************************************************************/
/* GroupWidget */
/************************************************************************/
class U2LANG_EXPORT GroupWidget : public WidgetsArea {
public:
    enum Type {
        DEFAULT,
        HIDEABLE
    };

    GroupWidget();
    GroupWidget(const QString &title, Type type);
    virtual ~GroupWidget();

    virtual void accept(WizardWidgetVisitor *visitor);

    void setType(Type value);
    Type getType() const;

    static const QString ID;

private:
    Type type;

    static const QString NAME;
};

/************************************************************************/
/* AttributeWidget */
/************************************************************************/
typedef QString AttributeWidgetType;

class U2LANG_EXPORT AttributeWidgetHints {
public:
    static const QString TYPE;
    static const QString DEFAULT;
    static const QString DATASETS;

    static const QString LABEL;
};

class U2LANG_EXPORT AttributeWidget : public WizardWidget {
public:
    AttributeWidget(const QString &actorId, const QString &attrId);
    virtual ~AttributeWidget();

    virtual void accept(WizardWidgetVisitor *visitor);
    virtual void validate(const QList<Actor*> &actors, U2OpStatus &os) const;

    QString getActorId() const;
    QString getAttributeId() const;

    void setWigdetHints(const QVariantMap &value);
    const QVariantMap & getWigdetHints() const;
    QVariantMap getProperties() const;
    QString getProperty(const QString &id) const;

private:
    QString actorId;
    QString attrId;
    QVariantMap hints;
};

} // U2

#endif // _U2_WIZARDWIDGET_H_
