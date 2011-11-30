include (U2Private.pri)

# Input
HEADERS += src/AppContextImpl.h \
           src/AppSettingsImpl.h \
           src/CrashHandler.h \
           src/DocumentFormatRegistryImpl.h \
           src/IOAdapterRegistryImpl.h \
           src/LogSettings.h \
           src/PluginDescriptor.h \
           src/PluginSupportImpl.h \
           src/ServiceRegistryImpl.h \
           src/SettingsImpl.h \
           src/TaskSchedulerImpl.h \
		   src/StackWalker.h 
SOURCES += src/AppContextImpl.cpp \
           src/AppSettingsImpl.cpp \
           src/CrashHandler.cpp \
           src/DocumentFormatRegistryImpl.cpp \
           src/IOAdapterRegistryImpl.cpp \
           src/LogSettings.cpp \
           src/PluginDescriptor.cpp \
           src/PluginSupportImpl.cpp \
           src/ServiceRegistryImpl.cpp \
           src/SettingsImpl.cpp \
           src/TaskSchedulerImpl.cpp \
		   src/StackWalker.cpp

TRANSLATIONS += transl/chinese.ts \
                transl/czech.ts \
                transl/english.ts \
                transl/russian.ts
