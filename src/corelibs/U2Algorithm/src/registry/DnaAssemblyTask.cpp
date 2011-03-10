#include "DnaAssemblyTask.h"

namespace U2 {

DnaAssemblyToReferenceTask::DnaAssemblyToReferenceTask( const DnaAssemblyToRefTaskSettings& s, TaskFlags _flags, bool _justBuildIndex )
 : Task("DnaAssemblyToRefTask", _flags), settings(s), justBuildIndex(_justBuildIndex)
{
    result.setName(s.resultFileName.baseFileName());
}


QVariant DnaAssemblyToRefTaskSettings::getCustomValue( const QString& optionName, const QVariant& defaultVal ) const
{
    if (customSettings.contains(optionName)) {
        return customSettings.value(optionName);
    } else {
        return defaultVal;
    }    
}

void DnaAssemblyToRefTaskSettings::setCustomValue( const QString& optionName, const QVariant& val )
{
    customSettings.insert(optionName,val);
}

void DnaAssemblyToRefTaskSettings::setCustomSettings( const QMap<QString, QVariant>& settings )
{
    customSettings = settings;
}
} // U2

