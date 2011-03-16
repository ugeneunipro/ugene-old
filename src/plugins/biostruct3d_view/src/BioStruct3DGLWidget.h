/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _U2_OPENGL_WIDGET_H_
#define _U2_OPENGL_WIDGET_H_

#include "BioStruct3DGLRender.h"
#include "BioStruct3DColorScheme.h"

#include <U2Core/Vector3D.h>
#include <U2Core/BioStruct3DObject.h>

#include <QtOpenGL/QGLWidget>
#include <QtGui/QMenu>
#include <QtGui/QActionGroup>
#include <QtGui/QAction>
#include <QtGui/QColor>
#include <QtCore/QTimer>
#include <memory>

namespace U2 { 

class Document;
class BioStruct3D;
class BioStruct3DColorSchemeFactory;
class BioStruct3DGLRendererFactory;
class MolecularSurfaceRendererFactory;
class AnnotatedDNAView;
class Annotation;
class AnnotationTableObject;
class MolecularSurfaceCalcTask;
class Task;
class AnnotationSelection;
class DNASequenceObject;
class ADVSequenceObjectContext;
class LRegionsSelection;
class GLFrameManager;
class GLFrame;
class MolecularSurface;
class MolecularSurfaceRenderer;
class U2Region;

class BioStruct3DRendererContext {
public:
    BioStruct3DRendererContext(BioStruct3DObject *obj)
            : obj(obj), biostruct(&obj->getBioStruct3D())
    {}

    const BioStruct3DObject *obj;
    const BioStruct3D *biostruct;

    QSharedPointer<BioStruct3DGLRenderer> renderer;
    QSharedPointer<BioStruct3DColorScheme> colorScheme;

    // this is just index numbers of models, NOT modelIds
    QList<int> shownModelsIndexes;
};

/*!
* @class BioStruct3DGLWidget BioStruct3DGLWidget.h
* @brief Class for BioStruct3DObject visualization.
*
* This widget provides a 3D graphical view of a macromolecular structure.   In terms
* of the Model-View architecture we consider
* the BioStruct3D the model and GLWidget a view of this model.
*
* The widget relies on various GLRenderer and ColorScheme subclasses to handle the
* rendering of the 3d objects.
*
* Also it includes actions for visualization control.
*/ 
class BioStruct3DGLWidget : public QGLWidget
{
    Q_OBJECT
    static int widgetCount;

public:
    /*!
    * Constructor.
    * @param bsObj BioStruct3DObject contains 3d model for visualization
    * @param view DnaView context for connecting structure 3D representation and sequence view
    * @param manager GlFrameManager is required for OpenGL frame manipulation
    * @param parent Parent widget
    */ 
    BioStruct3DGLWidget(BioStruct3DObject* bsObj, const AnnotatedDNAView* view, GLFrameManager* manager, QWidget *parent);

    //! Destructor.
    ~BioStruct3DGLWidget();

    /*!
    * @return Contexts for corresponding biostruct3d primary structure (sequences).
    */
    const QList<ADVSequenceObjectContext*> getSequenceContexts() const;

    //! @return BioStruct3D const reference.
    const BioStruct3D& getBioStruct3D() const { return *(contexts.first().biostruct); }

    //! @return BioStruct3D Protein Data Bank id.
    const QString getPDBId() const { return contexts.first().biostruct->pdbId; }

    //! @return BioStruct3DObject name.
    // unused
    //const QString getBioStruct3DObjectName() const;

    /*!
    * @return Menu containing display actions: renderers, color schemes etc.
    */
    QMenu* getDisplayMenu();
    /*!
    * Records widget active settings (part of UGENE architecture).
    * @return Current widgets settings.
    */
    QVariantMap getState();
    /*!
    * Restores widget settings with given (part of UGENE architecture).
    * @param state Widgets settings.
    */
    void setState(const QVariantMap& state);
    /*!
    * Restores default widget settings (viewMatrix, glRenderer, color scheme)
    */
    void restoreDefaultSettigns();
    /*!
    * @param delta Positive value: zoom in, negative value: zoom out.
    */
    void zoom(float delta);
    void shift(float deltaX, float deltaY);
    /*!
    * Creates 2d vector image of BioStruct3DGLWidget contents using gl2ps.
    */
    void writeImage2DToFile(int format, int options, int nbcol, const char *fileName);

    float getEyesShiftMult() const;
    float getRenderDetailLevel() const;
    void setBackgroundColor(QColor backgroundColor);
    float sumCorrectColors(float a, float b);

signals:
    void si_widgetClosed(BioStruct3DGLWidget* w);

protected:

    /*!
    * QGlWidget virtual function, initializes OpenGL params. See, Qt docs "QGLWidget" for details. 
    */
    void initializeGL();
    /*!
    * QGlWidget virtual function, sets viewport
    * @param width GLWidget width
    * @param height GLWidget height
    */
    void resizeGL(int width, int height);
    /*!
    * QGlWidget virtual function, draw GL scene.  
    */
    void paintGL();
    /*!
    * QWidget virtual function, mouse button down event handler.
    */
    void mousePressEvent(QMouseEvent *event);
    /*!
    * QWidget virtual function, mouse move event handler.
    */
    void mouseMoveEvent(QMouseEvent *event);
    /*!
    * QWidget virtual function, mouse wheel event handler.
    */
    void wheelEvent ( QWheelEvent * event );
    /*!
    * QWidget virtual function, executes context menu.
    */
    void contextMenuEvent(QContextMenuEvent *_event);
    
private:
    //! Sets unselected regions shading level
    void setUnselectedShadingLevel(int shading);

    /*!
    * Sets light position.
    * @param pos New light source position. Directional light is being used.
    */
    void setLightPosition(const Vector3D& pos);

    // controller logic
    //! Creates actions for existing GLRenderers, loads default renderer.
    void loadGLRenderers();
    //! Creates actions for existing ColorSchemes, loads default color scheme.
    void loadColorSchemes();

    //! Creates actions for select/deselect shown models.
    void createSelectModelsActions();

    //! Creates menu for select/deselect shown models.
    void createSelectModelsMenu();

    void createActions();
    void createMenus();

    // view logic
    //! Adds biostruct to scene.
    void addBiostruct(BioStruct3DObject *biostruct);

    //! Creates renderers for all biostructs.
    void setupRenderer(const QString &name);

    //! Creates color scheme for all biostructs.
    void setupColorScheme(const QString &name);

    //! Updates color scheme for all renderers.
    void updateAllColorSchemes();

    //! Show/hide selected model for first biostruct
    //! @param modelId - key from BioStruct3D::modelMap
    void showModel(int modelId, bool show);

    //! Show/hide all models for first biostruct
    void showAllModels(bool show);

    void saveDefaultSettings();

    int getChainIdForAnnotationObject(AnnotationTableObject* ao); 
    void connectExternalSignals();

    // anaglyph related
    unsigned int* emptyTextureData;
    GLuint anaglyphRenderTextureLeft, anaglyphRenderTextureRight, tempAnaglyphRenderTexture;

    void ViewOrtho();                                               // Set Up An Ortho View
    void ViewPerspective();                                         // Set Up A Perspective View
    GLuint getEmptyTexture(int textureWidth, int textureHeight);    // Create An Empty Texture

    void draw();
    void drawTexturesAnaglyph(GLuint anaglyphRenderTextureLeft, GLuint anaglyphRenderTextureRight);                                // Draw The Image
    void drawTexture(GLuint anaglyphRenderTexture, int red, int green, int blue, float alpha, bool alphaOnly);                     // Draw The Blurred Image
    void drawAll();
    void drawColoredPlane(int red, int green, int blue, float alpha);

    bool hasGlErrors();

    Vector3D getTrackballMapping(int x, int y);
    static int getSequenceChainId(const DNASequenceObject* seqObj);
    static int getWidgetCount(QString objectName);
    bool isSyncModeOn();

private:
    // related sequences view
    const AnnotatedDNAView* dnaView;

    QList<BioStruct3DRendererContext> contexts;

    GLFrameManager* frameManager;
    std::auto_ptr<GLFrame> glFrame;

    std::auto_ptr<MolecularSurface> molSurface;
    std::auto_ptr<MolecularSurfaceRenderer> surfaceRenderer;

    MolecularSurfaceCalcTask* surfaceCalcTask;

    QVariantMap defaultsSettings;

    // controller logic
    QString currentColorSchemeName;
    QString currentGLRendererName;

    QMap<const AnnotationTableObject*, int> chainIdCache;

    // camera
    GLfloat cameraClipNear, cameraClipFar; //cameraDistance, 
    GLfloat rotAngle, spinAngle;
    Vector3D rotAxis, lastPos;

    // light
    GLfloat lightPostion[4];

    QColor backgroundColor;
    QColor selectionColor;
    QTimer* animationTimer;

    int renderDetailLevel;
    int unselectedShadingLevel;

    // anaglyph related
    bool anaglyph, anaglyphAvailable, firstResize;
    int eyesShift;
    QColor leftEyeColor;
    QColor rightEyeColor;

    // controller logic
    QAction *spinAction;
    QAction *settingsAction;
    QAction *closeAction;
    QAction *exportImageAction;
    QAction *alignWithAction;

    // actions for select/deselect shown models
    QActionGroup *selectModelActions;
    QAction *selectModelsExclusiveAction;
    QAction *selectAllModelsAction;
    QAction *selectOneModelAction;

    QActionGroup *colorSchemeActions;
    QActionGroup *rendererActions;
    QActionGroup *molSurfaceRenderActions;
    QActionGroup *molSurfaceTypeActions;
    QMenu *selectColorSchemeMenu;
    QMenu *selectRendererMenu;
    QMenu *displayMenu;
    QMenu *modelsMenu;

private slots:
     void sl_selectColorScheme(QAction* action);
     void sl_selectGLRenderer(QAction* action);
     void sl_updateRenderSettings(const QStringList& list);
     void sl_acitvateSpin();
     void sl_onSequenceSelectionChanged(LRegionsSelection* s, const QVector<U2Region>& added, const QVector<U2Region>& removed);
     void sl_onAnnotationSelectionChanged(AnnotationSelection* thiz, const QList<Annotation*>& added, const QList<Annotation*>& removed);
     void sl_updateAnnimation();
     void sl_settings();
     void sl_closeWidget();
     void sl_exportImage();
     void sl_alignWith();

     // slots for select/deselect shown models actions
     // they affects only first biostruct
     void sl_selectModel(QAction *action);
     void sl_selectAllModels();
     void sl_selectOneModel();
     void sl_selectModelsExclusive();

     void sl_showSurface();
     void sl_hideSurface();
     void sl_selectSurfaceRenderer(QAction* surfaceRenderer);

     // used only for handling MolecularSurfaceCalcTask
     // should be in special MolecularSurfaceProxy
     void sl_onTaskFinished(Task* task);

public:
    static const QString BACKGROUND_COLOR_NAME;

    static const QString PRODUCT_NAME;
    static const QString PLUGIN_NAME;
    static const QString COLOR_SCHEME_NAME;
    static const QString RENDERER_NAME;
    // unused
    //static const QString OBJECT_ID_NAME;

    static const QString SELECTION_COLOR_NAME;

    static const QString LEFT_EYE_COLOR_NAME;
    static const QString RIGHT_EYE_COLOR_NAME;

    static const QString RENDER_DETAIL_LEVEL_NAME;
    static const QString SHADING_LEVEL_NAME;
    static const QString ANAGLYPH_NAME;
    static const QString EYES_SHIFT_NAME;
};

} //namespace


#endif // _U2_OPENGL_WIDGET_H
