#include <cassert>

#include "Descriptor.h"

namespace U2 {

/**************************
 * Descriptor
 **************************/
Descriptor::Descriptor(const QString& id, const QString& name, const QString& doc) : id(id), name(name), desc(doc) {
}

Descriptor::Descriptor(const QString& _id) : id(_id), name(_id), desc(_id) {
}

Descriptor::Descriptor(const char* _id) : id(_id), name(_id), desc(_id) {
}

Descriptor::Descriptor() {
}

QString Descriptor::getId() const {
    return id;
}

QString Descriptor::getDisplayName() const {
    return name;
}

QString Descriptor::getDocumentation() const {
    return desc;
}

void Descriptor::setId(const QString& i) {
    id = i;
}

void Descriptor::setDocumentation(const QString& d) {
    desc = d;
}

void Descriptor::setDisplayName(const QString& n) {
    name = n;
}

/**************************
 * VisualDescriptor
 **************************/
VisualDescriptor::VisualDescriptor(const Descriptor& d, const QString & _iconPath) : Descriptor(d), iconPath(_iconPath) {
}

void VisualDescriptor::setIconPath( const QString & ip ) {
    iconPath = ip;
}

QIcon VisualDescriptor::getIcon() {
    if( icon.isNull() && !iconPath.isEmpty() ) {
        icon = QIcon(iconPath);
    }
    return icon;
}

void VisualDescriptor::setIcon( QIcon i ) {
    assert( iconPath.isEmpty() );
    icon = i;
}

} // U2
