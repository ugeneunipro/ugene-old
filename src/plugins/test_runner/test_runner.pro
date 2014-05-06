include (test_runner.pri)

# Input
HEADERS += resource.h \
           src/GTestScriptWrapper.h \
           src/TestRunnerPlugin.h \
           src/TestViewController.h \
           src/TestViewReporter.h \
           src/ExcludeReasonDialog.h
FORMS += src/ui/Reporter.ui src/ui/TestView.ui \
         src/ui/ExcludeResaonDialog.ui
SOURCES += src/GTestScriptWrapper.cpp \
           src/TestRunnerPlugin.cpp \
           src/TestViewController.cpp \
           src/TestViewReporter.cpp \
           src/ExcludeReasonDialog.cpp
RESOURCES += test_runner.qrc
TRANSLATIONS += transl/english.ts transl/russian.ts
