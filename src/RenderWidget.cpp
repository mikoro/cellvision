// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#include "stdafx.h"

#include "RenderWidget.h"
#include "MainWindow.h"
#include "Log.h"

using namespace CellVision;

RenderWidget::RenderWidget(QWidget* parent) : QOpenGLWidget(parent), volumeTexture(QOpenGLTexture::Target3D)
{
	connect(this, SIGNAL(frameSwapped()), this, SLOT(update()));

	setFocus();
}

void RenderWidget::uploadImageData(const ImageLoaderResult& result)
{
	volumeTexture.destroy();
	volumeTexture.create();
	volumeTexture.bind();
	volumeTexture.setFormat(QOpenGLTexture::RGBA8_UNorm);
	volumeTexture.setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
	volumeTexture.setWrapMode(QOpenGLTexture::ClampToBorder);
	volumeTexture.setBorderColor(0.0f, 0.0f, 0.0f, 1.0f);
	volumeTexture.setMipLevels(1);
	volumeTexture.setSize(result.width, result.height, result.depth);
	volumeTexture.allocateStorage(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8);
	volumeTexture.setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, &result.data[0]);
	volumeTexture.release();
}

bool RenderWidget::event(QEvent* e)
{
	keyboardHelper.event(e);
	return QOpenGLWidget::event(e);
}

void RenderWidget::mousePressEvent(QMouseEvent* me)
{
	setFocus();

	previousMousePosition = me->globalPos();

	me->accept();
}

void RenderWidget::mouseMoveEvent(QMouseEvent* me)
{
	QPoint mouseDelta = me->globalPos() - previousMousePosition;
	previousMousePosition = me->globalPos();

	cameraRotation += QVector2D(-mouseDelta.x(), -mouseDelta.y()) * mouseSpeedModifier;

	QMatrix4x4 rotateYaw;
	QMatrix4x4 rotatePitch;

	//QVector3D yawAxis = cameraMatrix.column(1).toVector3D();
	//QVector3D pitchAxis = cameraMatrix.column(0).toVector3D();

	QVector3D yawAxis = QVector3D(0, 1, 0);
	QVector3D pitchAxis = QVector3D(1, 0, 0);

	rotateYaw.rotate(cameraRotation.x(), yawAxis);
	rotatePitch.rotate(cameraRotation.y(), pitchAxis);

	cameraMatrix = rotateYaw * rotatePitch;

	me->accept();
}

void RenderWidget::wheelEvent(QWheelEvent* we)
{
	//tfm::printfln("x: %f y: %f", we->angleDelta().x() / 120.0, we->angleDelta().y() / 120.0);

	we->accept();
}

void RenderWidget::initializeGL()
{
	initializeOpenGLFunctions();

	MainWindow::getLog().logInfo("OpenGL Vendor: %s | Renderer: %s | Version: %s | GLSL: %s", glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));;

	// CUBE //

	QVector3D v1(0, 0, 1); QVector3D t1(0, 0, 1);
	QVector3D v2(1, 0, 1); QVector3D t2(1, 0, 1);
	QVector3D v3(0, 1, 1); QVector3D t3(0, 1, 1);
	QVector3D v4(1, 1, 1); QVector3D t4(1, 1, 1);
	QVector3D v5(0, 0, 0); QVector3D t5(0, 0, 0);
	QVector3D v6(1, 0, 0); QVector3D t6(1, 0, 0);
	QVector3D v7(0, 1, 0); QVector3D t7(0, 1, 0);
	QVector3D v8(1, 1, 0); QVector3D t8(1, 1, 0);

	const QVector3D cubeVertexData[] =
	{
		v1, t1, v2, t2, v4, t4,
		v1, t1, v4, t4, v3, t3,
		v2, t2, v6, t6, v8, t8,
		v2, t2, v8, t8, v4, t4,
		v6, t6, v5, t5, v7, t7,
		v6, t6, v7, t7, v8, t8,
		v5, t5, v1, t1, v3, t3,
		v5, t5, v3, t3, v7, t7,
		v1, t1, v6, t6, v5, t5,
		v1, t1, v2, t2, v6, t6,
		v3, t3, v8, t8, v7, t7,
		v3, t3, v4, t4, v8, t8
	};

	cube.program.addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/cube.vert");
	cube.program.addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/cube.frag");
	cube.program.link();
	cube.program.bind();

	cube.vbo.create();
	cube.vbo.bind();
	cube.vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	cube.vbo.allocate(cubeVertexData, sizeof(cubeVertexData));

	cube.vao.create();
	cube.vao.bind();

	cube.program.enableAttributeArray("position");
	cube.program.enableAttributeArray("texcoord");
	cube.program.setAttributeBuffer("position", GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
	cube.program.setAttributeBuffer("texcoord", GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

	cube.vao.release();
	cube.vbo.release();
	cube.program.release();

	// CUBE LINES //

	const QVector3D cubeLinesVertexData[] =
	{
		v1, v2,
		v2, v4,
		v4, v3,
		v3, v1,
		v2, v6,
		v6, v5,
		v5, v1,
		v6, v8,
		v8, v7,
		v7, v5,
		v4, v8,
		v3, v7,
	};

	cubeLines.program.addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/lines.vert");
	cubeLines.program.addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/lines.frag");
	cubeLines.program.link();
	cubeLines.program.bind();

	cubeLines.vbo.create();
	cubeLines.vbo.bind();
	cubeLines.vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	cubeLines.vbo.allocate(cubeLinesVertexData, sizeof(cubeLinesVertexData));

	cubeLines.vao.create();
	cubeLines.vao.bind();

	cubeLines.program.enableAttributeArray("position");
	cubeLines.program.setAttributeBuffer("position", GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));

	cubeLines.vao.release();
	cubeLines.vbo.release();
	cubeLines.program.release();

	// PLANE //

	v1 = QVector3D(-1, -1, 0);
	v2 = QVector3D(1, -1, 0);
	v3 = QVector3D(-1, 1, 0);
	v4 = QVector3D(1, 1, 0);

	const QVector3D planeVertexData[] =
	{
		v1, v2, v4,
		v1, v4, v3
	};

	plane.program.addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/plane.vert");
	plane.program.addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/plane.frag");
	plane.program.link();
	plane.program.bind();

	plane.vbo.create();
	plane.vbo.bind();
	plane.vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	plane.vbo.allocate(planeVertexData, sizeof(planeVertexData));

	plane.vao.create();
	plane.vao.bind();

	plane.program.enableAttributeArray("position");
	plane.program.setAttributeBuffer("position", GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));

	plane.vao.release();
	plane.vbo.release();
	plane.program.release();

	// PLANE LINES //

	const QVector3D planeLinesVertexData[] =
	{
		v1, v2,
		v2, v4,
		v4, v3,
		v3, v1
	};

	planeLines.program.addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/lines.vert");
	planeLines.program.addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/lines.frag");
	planeLines.program.link();
	planeLines.program.bind();

	planeLines.vbo.create();
	planeLines.vbo.bind();
	planeLines.vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	planeLines.vbo.allocate(planeLinesVertexData, sizeof(planeLinesVertexData));

	planeLines.vao.create();
	planeLines.vao.bind();

	planeLines.program.enableAttributeArray("position");
	planeLines.program.setAttributeBuffer("position", GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));

	planeLines.vao.release();
	planeLines.vbo.release();
	planeLines.program.release();

	// COORDINATE LINES //

	v1 = QVector3D(-1000, 0, 0); v2 = QVector3D(1000, 0, 0);
	v3 = QVector3D(0, -1000, 0); v4 = QVector3D(0, 1000, 0);
	v5 = QVector3D(0, 0, -1000); v6 = QVector3D(0, 0, 1000);
	
	QVector3D c1(1, 0, 0);
	QVector3D c2(0, 1, 0);
	QVector3D c3(0, 0, 1);

	const QVector3D coordinateLinesVertexData[] =
	{
		v1, c1, v2, c1,
		v3, c2, v4, c2,
		v5, c3, v6, c3
	};

	coordinateLines.program.addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/coordinates.vert");
	coordinateLines.program.addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/coordinates.frag");
	coordinateLines.program.link();
	coordinateLines.program.bind();

	coordinateLines.vbo.create();
	coordinateLines.vbo.bind();
	coordinateLines.vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	coordinateLines.vbo.allocate(coordinateLinesVertexData, sizeof(coordinateLinesVertexData));

	coordinateLines.vao.create();
	coordinateLines.vao.bind();

	coordinateLines.program.enableAttributeArray("position");
	coordinateLines.program.enableAttributeArray("color");
	coordinateLines.program.setAttributeBuffer("position", GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
	coordinateLines.program.setAttributeBuffer("color", GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

	coordinateLines.vao.release();
	coordinateLines.vbo.release();
	coordinateLines.program.release();

	// MISC //

	timeStepTimer.start();
	resetCamera();
}

void RenderWidget::resizeGL(int width, int height)
{
}

void RenderWidget::paintGL()
{
	updateLogic();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(4.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// COORDINATE LINES //

	if (renderCoordinates)
	{
		coordinateLines.program.bind();
		coordinateLines.vao.bind();

		coordinateLines.program.setUniformValue("mvp", coordinateLines.mvp);

		glDrawArrays(GL_LINES, 0, 6);

		coordinateLines.vao.release();
		coordinateLines.program.release();
	}

	// CUBE //

	cube.program.bind();
	cube.vao.bind();

	if (volumeTexture.isCreated())
		volumeTexture.bind();

	cube.program.setUniformValue("texture0", 0);
	cube.program.setUniformValue("mvp", cube.mvp);

	//glDrawArrays(GL_TRIANGLES, 0, 36);

	if (volumeTexture.isCreated())
		volumeTexture.release();

	cube.vao.release();
	cube.program.release();

	// CUBE LINES //

	glDisable(GL_DEPTH_TEST);

	cubeLines.program.bind();
	cubeLines.vao.bind();

	cubeLines.program.setUniformValue("lineColor", QVector4D(1.0f, 1.0f, 1.0f, 0.5f));
	cubeLines.program.setUniformValue("mvp", cube.mvp);

	glDrawArrays(GL_LINES, 0, 24);

	cubeLines.vao.release();
	cubeLines.program.release();

	// PLANE //

	glDisable(GL_DEPTH_TEST);

	plane.program.bind();
	plane.vao.bind();

	if (volumeTexture.isCreated())
		volumeTexture.bind();

	plane.program.setUniformValue("texture0", 0);
	plane.program.setUniformValue("modelMatrix", plane.modelMatrix);
	plane.program.setUniformValue("mvp", plane.mvp);

	//glDrawArrays(GL_TRIANGLES, 0, 6);

	if (volumeTexture.isCreated())
		volumeTexture.release();

	plane.vao.release();
	plane.program.release();

	// PLANE LINES //

	planeLines.program.bind();
	planeLines.vao.bind();

	planeLines.program.setUniformValue("lineColor", QVector4D(1.0f, 1.0f, 1.0f, 0.5f));
	planeLines.program.setUniformValue("mvp", plane.mvp);

	//glDrawArrays(GL_LINES, 0, 8);

	planeLines.vao.release();
	planeLines.program.release();
}

void RenderWidget::updateLogic()
{
	float timeStep = timeStepTimer.nsecsElapsed() / 1000000000.0f;
	timeStepTimer.restart();

	if (keyboardHelper.keyIsDown(Qt::Key_PageUp))
		moveSpeedModifier *= 1.05f;

	if (keyboardHelper.keyIsDown(Qt::Key_PageDown))
		moveSpeedModifier *= 0.95f;

	if (keyboardHelper.keyIsDown(Qt::Key_Home))
		mouseSpeedModifier *= 1.05f;

	if (keyboardHelper.keyIsDown(Qt::Key_End))
		mouseSpeedModifier *= 0.95f;

	float moveSpeed = 0.5f * moveSpeedModifier;

	if (keyboardHelper.keyIsDown(Qt::Key_Shift))
		moveSpeed *= 2.0f;

	if (keyboardHelper.keyIsDown(Qt::Key_Control))
		moveSpeed *= 0.5f;

	if (keyboardHelper.keyIsDown(Qt::Key_R))
		resetCamera();

	if (keyboardHelper.keyIsDownOnce(Qt::Key_C))
		renderCoordinates = !renderCoordinates;

	QVector3D cameraRight = cameraMatrix.column(0).toVector3D();
	QVector3D cameraUp = cameraMatrix.column(1).toVector3D();
	QVector3D cameraForward = -cameraMatrix.column(2).toVector3D();

	if (keyboardHelper.keyIsDown(Qt::Key_W) || keyboardHelper.keyIsDown(Qt::Key_Up))
		cameraPosition += cameraForward * moveSpeed * timeStep;

	if (keyboardHelper.keyIsDown(Qt::Key_S) || keyboardHelper.keyIsDown(Qt::Key_Down))
		cameraPosition -= cameraForward * moveSpeed * timeStep;

	if (keyboardHelper.keyIsDown(Qt::Key_D) || keyboardHelper.keyIsDown(Qt::Key_Right))
		cameraPosition += cameraRight * moveSpeed * timeStep;

	if (keyboardHelper.keyIsDown(Qt::Key_A) || keyboardHelper.keyIsDown(Qt::Key_Left))
		cameraPosition -= cameraRight * moveSpeed * timeStep;

	if (keyboardHelper.keyIsDown(Qt::Key_E))
		cameraPosition += cameraUp * moveSpeed * timeStep;

	if (keyboardHelper.keyIsDown(Qt::Key_Q))
		cameraPosition -= cameraUp * moveSpeed * timeStep;

	QMatrix4x4 tempCameraMatrix = cameraMatrix;
	tempCameraMatrix.setColumn(3, QVector4D(cameraPosition.x(), cameraPosition.y(), cameraPosition.z(), 1.0f));

	QMatrix4x4 viewMatrix = tempCameraMatrix.inverted();

	QMatrix4x4 projectionMatrix;
	projectionMatrix.setToIdentity();
	float aspectRatio = float(width()) / float(height());
	projectionMatrix.perspective(45.0f, aspectRatio, 0.01f, 1000.0f);

	cube.modelMatrix.setToIdentity();
	cube.mvp = projectionMatrix * viewMatrix * cube.modelMatrix;

	QVector3D planePosition = -cameraPosition + 3.0f * cameraForward;

	plane.modelMatrix.setToIdentity();
	plane.modelMatrix.setColumn(0, QVector4D(cameraRight.x(), cameraRight.y(), cameraRight.z(), 0.0f));
	plane.modelMatrix.setColumn(1, QVector4D(cameraUp.x(), cameraUp.y(), cameraUp.z(), 0.0f));
	plane.modelMatrix.setColumn(2, QVector4D(cameraForward.x(), cameraForward.y(), cameraForward.z(), 0.0f));
	plane.modelMatrix.setColumn(3, QVector4D(planePosition.x(), planePosition.y(), planePosition.z(), 1.0f));
	plane.mvp = projectionMatrix * viewMatrix * plane.modelMatrix;

	coordinateLines.modelMatrix.setToIdentity();
	coordinateLines.mvp = projectionMatrix * viewMatrix * coordinateLines.modelMatrix;
}

void RenderWidget::resetCamera()
{
	cameraPosition = QVector3D(0.5f, 0.5f, 3.0f);
	cameraRotation = QVector2D(0.0f, 0.0f);
	cameraMatrix.setToIdentity();
}
