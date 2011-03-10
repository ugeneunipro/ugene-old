include (opencl_support.pri)

# Input
HEADERS += src/OpenCLSupportPlugin.h \
	   src/OpenCLSupportSettingsController.h \	   

SOURCES += src/OpenCLSupportPlugin.cpp \
           src/OpenCLSupportSettingsController.cpp \
           
TRANSLATIONS += transl/english.ts transl/russian.ts