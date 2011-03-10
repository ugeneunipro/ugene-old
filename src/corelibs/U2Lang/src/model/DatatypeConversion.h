#ifndef _U2_DATATYPECONV_H_
#define _U2_DATATYPECONV_H_

namespace U2 {

//TODO rethink...
class U2LANG_EXPORT DataTypeConvertion {
public:
    enum ConvertionType {
        Equal,
        Narrow,
        Wide,
        None
    };
    virtual QVariant convert(const QVariant& d, const DataTypePtr from, const DataTypePtr to) = 0;
    virtual ConvertionType canConvert(const DataTypePtr from, const DataTypePtr to) = 0;
};

} //namespace
#endif
