/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <QOpenGLFunctions_2_0>

#include "AnaglyphRenderer.h"

#include "BioStruct3DGLWidget.h"
#include "GLFrameManager.h"
#include "GraphicUtils.h"

namespace U2 {

static const QString EYES_SHIFT_NAME("EyesShift");
static const QString LEFT_EYE_COLOR_NAME("LeftEyeColor");
static const QString RIGHT_EYE_COLOR_NAME("RightEyeColor");

QVariantMap AnaglyphSettings::toMap(QVariantMap &map) const {
    map[EYES_SHIFT_NAME] = qVariantFromValue(eyesShift);
    map[LEFT_EYE_COLOR_NAME] = qVariantFromValue(leftEyeColor);
    map[RIGHT_EYE_COLOR_NAME] = qVariantFromValue(rightEyeColor);

    return map;
}

AnaglyphSettings AnaglyphSettings::fromMap(const QVariantMap &map) {
    float eyesShift = map[EYES_SHIFT_NAME].value<float>();
    QColor leftEyeColor = map[LEFT_EYE_COLOR_NAME].value<QColor>();
    QColor rightEyeColor= map[RIGHT_EYE_COLOR_NAME].value<QColor>();

    return AnaglyphSettings(eyesShift, leftEyeColor, rightEyeColor);
}

void AnaglyphRenderer::init() {
    createEmptyTextures();
}

void AnaglyphRenderer::resize(int _width, int _height) {
    CHECK_GL_ERROR

    width = _width, height = _height;

    if ( !(width > 0 && height > 0) ) {
        return;
    }

    createEmptyTextures();

    CHECK_GL_ERROR
}

void AnaglyphRenderer::draw() {
    QOpenGLFunctions_2_0 *f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_2_0>();
    CHECK_GL_ERROR

    GLFrame *glFrame = renderer->getGLFrame();
    float eyesShift = 5.0 * settings.eyesShift * glFrame->getCameraPosition().z / 200.0;

    f->glMatrixMode(GL_MODELVIEW);

    // Prepare anaglyph textures
    f->glPushMatrix();
        //glTranslatef(eyesShift, 0, 0);
        f->glLoadIdentity();
        gluLookAt(eyesShift, 0.0, glFrame->getCameraPosition().z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

        // at this moment buffer must be clean glClear omitted as a slow operation
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // isolate errors from main scene renderer
        CHECK_GL_ERROR
        renderer->draw();
        f->glGetError();

        f->glBindTexture(GL_TEXTURE_2D, anaglyphRenderTextureRight);
        f->glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, width, height, 0);
    f->glPopMatrix();

    f->glPushMatrix();
        //glTranslatef(-eyesShift, 0, 0);
        f->glLoadIdentity();
        gluLookAt(-eyesShift, 0.0, glFrame->getCameraPosition().z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

        f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // isolate errors from main scene renderer
        CHECK_GL_ERROR
        renderer->draw();
        f->glGetError();

        f->glBindTexture(GL_TEXTURE_2D, anaglyphRenderTextureLeft);
        f->glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, width, height, 0);
    f->glPopMatrix();

    // Draw anaglyph textures in ortho projection
    f->glMatrixMode(GL_MODELVIEW);
    f->glPushMatrix();
    f->glLoadIdentity();

    f->glMatrixMode(GL_PROJECTION);
    f->glPushMatrix();

        setOrthoProjection();

        f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawTexturesAnaglyph();

    f->glMatrixMode(GL_PROJECTION);
    f->glPopMatrix();

    f->glMatrixMode(GL_MODELVIEW);
    f->glPopMatrix();

    CHECK_GL_ERROR
}

void AnaglyphRenderer::setOrthoProjection() {
    QOpenGLFunctions_2_0 *f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_2_0>();
    CHECK_GL_ERROR

    f->glMatrixMode(GL_PROJECTION);
    f->glLoadIdentity();
    f->glOrtho(0, 1, 1, 0, -1, 1);
    f->glMatrixMode(GL_MODELVIEW);

    CHECK_GL_ERROR
}

void AnaglyphRenderer::createEmptyTextures() {
    QOpenGLFunctions_2_0 *f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_2_0>();
    CHECK_GL_ERROR

    if(anaglyphRenderTextureLeft != 0) {
        f->glDeleteTextures(1, &anaglyphRenderTextureLeft);
    }
    if(anaglyphRenderTextureRight != 0) {
        f->glDeleteTextures(1, &anaglyphRenderTextureRight);
    }
    if(tempAnaglyphRenderTexture != 0) {
        f->glDeleteTextures(1, &tempAnaglyphRenderTexture);
    }

    // TODO : check for NPOT sizes
    GLuint texwidth = width, texheight = height;

    GLuint txtnumbers[3] = {0};
    f->glGenTextures(3, txtnumbers);

    char *txtdata = new char[texwidth * texheight * 4];

    for (GLuint *txtnumber = txtnumbers; txtnumber < txtnumbers + 3; ++txtnumber) {
        f->glBindTexture(GL_TEXTURE_2D, *txtnumber);
        f->glTexImage2D(GL_TEXTURE_2D, 0, 4, texwidth, texheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) txtdata);

        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    }

    delete[] txtdata;

    anaglyphRenderTextureLeft   = txtnumbers[0];
    anaglyphRenderTextureRight  = txtnumbers[1];
    tempAnaglyphRenderTexture   = txtnumbers[2];

    CHECK_GL_ERROR
}

void AnaglyphRenderer::drawTexturesAnaglyph() {
    QOpenGLFunctions_2_0 *f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_2_0>();
    CHECK_GL_ERROR

    drawTexture(anaglyphRenderTextureLeft, settings.rightEyeColor.red(), settings.rightEyeColor.green(), settings.rightEyeColor.blue(), 0.5f, false); // colored left image

    f->glBindTexture(GL_TEXTURE_2D, tempAnaglyphRenderTexture);
    f->glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, width, height, 0);    // Copy Our ViewPort To The Blur Texture (From 0,0 To 128,128... No Border)

    drawTexture(anaglyphRenderTextureRight, settings.leftEyeColor.red(), settings.leftEyeColor.green(), settings.leftEyeColor.blue(), 0.5f, false); // colored right image
    drawTexture(tempAnaglyphRenderTexture, 255, 255, 255, 1.0f, true);

    CHECK_GL_ERROR
}

void AnaglyphRenderer::drawTexture(GLuint anaglyphRenderTexture, int red, int green, int blue, float alpha, bool alphaOnly)
{
    QOpenGLFunctions_2_0 *f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_2_0>();
    CHECK_GL_ERROR

    f->glEnable(GL_TEXTURE_2D);

    if (alphaOnly){
        // the second image is this, it has needed alpha color
        f->glBlendFunc(GL_DST_ALPHA, GL_DST_ALPHA);
    }
    else {
        f->glBlendFunc(GL_ONE, GL_ONE);
    }

    f->glBindTexture(GL_TEXTURE_2D, anaglyphRenderTexture);

    f->glColor4ub(red, green, blue, (GLubyte) (alpha * 255.0));
    f->glBegin(GL_QUADS);
        f->glTexCoord2f(0, 1);
        f->glVertex2f(0,0);

        f->glTexCoord2f(0, 0);
        f->glVertex2f(0, 1);

        f->glTexCoord2f(1, 0);
        f->glVertex2f(1, 1);

        f->glTexCoord2f(1, 1);
        f->glVertex2f(1, 0);
    f->glEnd();

    // Unbind the blur texture
    f->glBindTexture(GL_TEXTURE_2D, 0);
    f->glDisable(GL_TEXTURE_2D);

    CHECK_GL_ERROR
}

AnaglyphRenderer::AnaglyphRenderer(BioStruct3DGLWidget *_renderer, const AnaglyphSettings &_settings) :
    renderer(_renderer),
    settings(_settings),
    width(1),
    height(1),
    anaglyphRenderTextureLeft(0),
    anaglyphRenderTextureRight(0),
    tempAnaglyphRenderTexture(0),
    hasErrors(false)
{

}

AnaglyphRenderer::~AnaglyphRenderer() {

}

bool AnaglyphRenderer::isAvailable() {
    draw();
    return !hasErrors;
}

}   // namespace U2

