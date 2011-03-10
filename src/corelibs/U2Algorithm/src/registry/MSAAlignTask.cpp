#include "MSAAlignTask.h"

namespace U2 {

MSAAlignTask::MSAAlignTask(MAlignmentObject* _obj, const MSAAlignTaskSettings& s, TaskFlags _flags)
 : Task("MSAAlignTask", _flags), obj(_obj), settings(s) {}


QVariant MSAAlignTaskSettings::getCustomValue( const QString& optionName, const QVariant& defaultVal ) const
{
    if (customSettings.contains(optionName)) {
        return customSettings.value(optionName);
    } else {
        return defaultVal;
    }    
}

void MSAAlignTaskSettings::setCustomValue( const QString& optionName, const QVariant& val )
{
    customSettings.insert(optionName,val);
}

void MSAAlignTaskSettings::setCustomSettings( const QMap<QString, QVariant>& settings )
{
    customSettings = settings;
}
} // U2

