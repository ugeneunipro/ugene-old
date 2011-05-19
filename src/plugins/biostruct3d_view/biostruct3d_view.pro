include (biostruct3d_view.pri)

# Input
HEADERS += src/BallAndStickGLRenderer.h \
           src/BioStruct3DColorScheme.h \
           src/BioStruct3DGLRender.h \
           src/BioStruct3DGLWidget.h \
           src/BioStruct3DSplitter.h \
           src/BioStruct3DViewPlugin.h \
           src/ExportImage3DGLDialog.h \
           src/GLFrameManager.h \
           src/GraphicUtils.h \
           src/MolecularSurfaceRenderer.h \
           src/TubeGLRenderer.h \
           src/VanDerWaalsGLRenderer.h \
           src/WormsGLRenderer.h \
           src/SettingsDialog.h \
           src/StructuralAlignmentDialog.h \
           src/gl2ps/gl2ps.h \
           src/BioStruct3DSubsetEditor.h

FORMS +=   src/ui/SettingsDialog.ui \
           src/ui/StructuralAlignmentDialog.ui \
           src/ui/BioStruct3DSubsetEditor.ui

SOURCES += src/BallAndStickGLRenderer.cpp \
           src/BioStruct3DColorScheme.cpp \
           src/BioStruct3DGLRender.cpp \
           src/BioStruct3DGLWidget.cpp \
           src/BioStruct3DSplitter.cpp \
           src/BioStruct3DViewPlugin.cpp \
           src/ExportImage3DGLDialog.cpp \
           src/GLFrameManager.cpp \
           src/GraphicUtils.cpp \
           src/MolecularSurfaceRenderer.cpp \
           src/TubeGLRenderer.cpp \
           src/VanDerWaalsGLRenderer.cpp \
           src/WormsGLRenderer.cpp \
           src/SettingsDialog.cpp \
           src/StructuralAlignmentDialog.cpp \
           src/gl2ps/gl2ps.cpp \
           src/BioStruct3DSubsetEditor.cpp

RESOURCES += biostruct3d_view.qrc
TRANSLATIONS += transl/english.ts transl/russian.ts
