#ifndef _U2_DI_PROPERTIES_SITECON_H_
#define _U2_DI_PROPERTIES_SITECON_H_

#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QString>

#include <U2Core/DIProperties.h>

#include <assert.h>

namespace U2 {

class DiPropertySitecon;
class DinucleotitePropertyRegistry {
public:
    DinucleotitePropertyRegistry();
    ~DinucleotitePropertyRegistry();

    QList<DiPropertySitecon*> getProperties() const {return props;}

private:
    void registerProperty(const QString& str);
    QList<DiPropertySitecon*> props;
};

class DiPropertySitecon : public DiProperty {

public:
    //todo: default val;

    QMap<QString,QString>   keys;
    float                   original[16];
    float                   normalized[16];
    float                   average;
    float                   sdeviation;

    float getOriginal(char c1, char c2) const {return original[index(c1, c2)];}
    float getNormalized(char c1, char c2) const {return normalized[index(c1, c2)];}
};

}//namespace

#endif
