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

#ifndef _U2_GL_FRAME_MANAGER_H_
#define _U2_GL_FRAME_MANAGER_H_

#include <U2Core/Matrix44.h>

#include <QtOpenGL/QGLWidget>
#include <QtCore/QVariantList>

namespace U2 { 

class Vector3D;

/*!
* @brief Class for OpenGL 3d scene manipulation
*/
class GLFrame {
    static const GLfloat DEFAULT_ZOOM;
//    static const Vector3D DEFAULT_CAMERA;
    QGLWidget* glWidget;
//    GLfloat zoomFactor;
    Matrix44 rotMatrix;
    bool synchLock;
    float cameraClipNear, cameraClipFar;
//    Vector3D cameraPosition;

public:
    GLfloat zoomFactor;
    Vector3D cameraPosition;
    GLFrame(QGLWidget* widget, float cameraNear, float cameraFar, float cameraZ);
    void setState(const QVariantMap& state);
    void writeStateToMap(QVariantMap& states);
    QGLWidget* getGLWidget() { return glWidget; }
    void makeCurrent() { glWidget->makeCurrent(); }
    void updateViewPort(int width, int height);
    void updateViewPort();
    void updateGL() { glWidget->updateGL(); }
    GLfloat getZoomFactor() const { return zoomFactor; }
    float* getRotationMatrix() { return rotMatrix.getData(); }
    void rotateCamera(const Vector3D& rotAxis, float rotAngle);

    const Vector3D getCameraPosition()const;
    void setCameraPosition(float x, float y, float z);

    void performZoom(float delta);
    void performShift(float deltaX, float deltaY);
};

class GLFrameManager {
    QMap<QGLWidget*, GLFrame*> widgetFrameMap;
    bool syncLock;

public:
    GLFrameManager() : syncLock(false) {}
    ~GLFrameManager();
    bool getSyncLock() const { return syncLock; }
    void setSyncLock(bool lockOn, QGLWidget* syncWidget);
    void addGLFrame(GLFrame* frame);
    GLFrame* getGLWidgetFrame(QGLWidget* widget);
    void removeGLWidgetFrame(QGLWidget* widget);
    QList<GLFrame*> getGLFrames();
    QList<GLFrame*> getActiveGLFrameList(GLFrame* currentFrame, bool syncModeOn);

};

} // namespace


#endif // _U2_GL_FRAME_MANAGER_H_


