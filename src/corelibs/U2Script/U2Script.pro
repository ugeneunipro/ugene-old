include (U2Script.pri)

HEADERS +=      src/CommonDbi.h \
                src/globals.h \
                src/FormatDetection.h \
                src/SchemeHandle.h \
                src/SchemeWrapper.h \
                src/TextConversionUtils.h \
                src/U2Script.h \
                src/UgeneContextWrapper.h \
                src/WorkflowElementFacade.h

SOURCES +=      src/CommonDbi.cpp \
                src/globals.cpp \
                src/FormatDetection.cpp \
                src/SchemeHandle.cpp \
                src/SchemeWrapper.cpp \
                src/TextConversionUtils.cpp \
                src/U2Script.cpp \
                src/UgeneContextWrapper.cpp \
                src/WorkflowElementFacade.cpp

#count( UGENE_NODE_DIR, 1 ) {
#    HEADERS +=  src/js/ActorWrap.h \
#                src/js/DebugStatusWrap.h \
#                src/js/JsContext.h \
#                src/js/JsScheduler.h \
#                src/js/NodeApiUtils.h
#
#    SOURCES +=  src/js/ActorWrap.cpp \
#                src/js/DebugStatusWrap.cpp \
#                src/js/JsContext.cpp \
#                src/js/JsScheduler.cpp \
#                src/js/NodeApiUtils.cpp
#}

TRANSLATIONS += transl/chinese.ts \
                transl/czech.ts \
                transl/english.ts \
                transl/russian.ts
