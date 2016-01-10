// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#include "stdafx.h"

#include "RenderWidget.h"
#include "MainWindow.h"
#include "Log.h"
#include "ImageLoader.h"

using namespace CellVision;

RenderWidget::RenderWidget(QWidget* parent) : QOpenGLWidget(parent), volumeTexture(QOpenGLTexture::Target3D)
{
	connect(this, SIGNAL(frameSwapped()), this, SLOT(update()));

	setFocus();
}

void RenderWidget::initialize(const RenderWidgetSettings& settings_)
{
	settings = settings_;

	ImageLoaderResult result = ImageLoader::loadFromMultipageTiff(settings.imageLoaderInfo);

	if (result.data.size() > 0)
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

	std::array<QVector3D, 72> cubeVertexData;
	std::array<QVector3D, 24> cubeLinesVertexData;
	generateCubeVertices(cubeVertexData, cubeLinesVertexData, settings.imageWidth, settings.imageHeight, settings.imageDepth);

	cube.vbo.bind();
	cube.vbo.write(0, cubeLinesVertexData.data(), sizeof(cubeLinesVertexData));
	cube.vbo.release();

	std::array<float, 30> backgroundVertexData;
	generateBackgroundVertices(backgroundVertexData, settings.backgroundColor);

	background.vbo.bind();
	background.vbo.write(0, backgroundVertexData.data(), sizeof(backgroundVertexData));
	background.vbo.release();

	resetCamera();
}

bool RenderWidget::event(QEvent* e)
{
	keyboardHelper.event(e);
	return QOpenGLWidget::event(e);
}

void RenderWidget::mousePressEvent(QMouseEvent* me)
{
	setFocus();

	if (me->buttons() == Qt::LeftButton)
		mouseMode = MouseMode::ROTATE;
	else if (me->buttons() == Qt::RightButton)
		mouseMode = MouseMode::ORBIT;
	else if (me->buttons() == (Qt::LeftButton | Qt::RightButton))
		mouseMode = MouseMode::MOVE;
	else if (me->buttons() == Qt::MidButton)
		mouseMode = MouseMode::MEASURE;
	else
		mouseMode = MouseMode::NONE;

	previousMousePosition = me->globalPos();

	if (mouseMode == MouseMode::ORBIT)
	{
		QPointF point = me->localPos();
		float x = point.x() / float(width());
		float y = point.y() / float(height());
		x = (x - 0.5f) * 2.0f;
		y = (y - 0.5f) * -2.0f;

		tfm::printfln("x: %f y: %f", x, y);
	}

	me->accept();
}

void RenderWidget::mouseReleaseEvent(QMouseEvent* me)
{
	if (me->buttons() == Qt::LeftButton)
		mouseMode = MouseMode::ROTATE;
	else if (me->buttons() == Qt::RightButton)
		mouseMode = MouseMode::ORBIT;
	else if (me->buttons() == (Qt::LeftButton | Qt::RightButton))
		mouseMode = MouseMode::MOVE;
	else if (me->buttons() == Qt::MidButton)
		mouseMode = MouseMode::MEASURE;
	else
		mouseMode = MouseMode::NONE;

	me->accept();
}

void RenderWidget::mouseMoveEvent(QMouseEvent* me)
{
	QPoint mouseDelta = me->globalPos() - previousMousePosition;
	previousMousePosition = me->globalPos();

	if (mouseMode == MouseMode::ROTATE)
	{
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
	}
	else if (mouseMode == MouseMode::MOVE)
	{
		float moveSpeed = mouseSpeedModifier * 0.05f;

		if (keyboardHelper.keyIsDown(Qt::Key_Shift))
			moveSpeed *= 2.0f;

		if (keyboardHelper.keyIsDown(Qt::Key_Control))
			moveSpeed *= 0.5f;

		cameraPosition += cameraForward * -mouseDelta.y() * moveSpeed;
	}

	me->accept();
}

void RenderWidget::wheelEvent(QWheelEvent* we)
{
	float moveSpeed = mouseWheelSpeedModifier;

	if (keyboardHelper.keyIsDown(Qt::Key_Shift))
		moveSpeed *= 2.0f;

	if (keyboardHelper.keyIsDown(Qt::Key_Control))
		moveSpeed *= 0.5f;

	planeDistance += we->angleDelta().y() / 120.0f * moveSpeed;

	we->accept();
}

void RenderWidget::initializeGL()
{
	initializeOpenGLFunctions();

	MainWindow::getLog().logInfo("OpenGL Vendor: %s | Renderer: %s | Version: %s | GLSL: %s", glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));;

	// CUBE //

	std::array<QVector3D, 72> cubeVertexData;
	std::array<QVector3D, 24> cubeLinesVertexData;
	generateCubeVertices(cubeVertexData, cubeLinesVertexData, 1.0f, 1.0f, 1.0f);

	cube.program.addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/cube.vert");
	cube.program.addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/cube.frag");
	cube.program.link();
	cube.program.bind();

	cube.vbo.create();
	cube.vbo.bind();
	cube.vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	cube.vbo.allocate(cubeLinesVertexData.data(), sizeof(cubeLinesVertexData));

	cube.vao.create();
	cube.vao.bind();

	cube.program.enableAttributeArray("position");
	cube.program.setAttributeBuffer("position", GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));

	cube.vao.release();
	cube.vbo.release();
	cube.program.release();

	// PLANE //

	QVector3D v1(-10, -10, 0);
	QVector3D v2(10, -10, 0);
	QVector3D v3(-10, 10, 0);
	QVector3D v4(10, 10, 0);

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

	// COORDINATES //
	
	const float coordinatesVertexData[] =
	{
		-10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, -10.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 10.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, -10.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 1.0f, 1.0f
	};

	coordinates.program.addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/coordinates.vert");
	coordinates.program.addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/coordinates.frag");
	coordinates.program.link();
	coordinates.program.bind();

	coordinates.vbo.create();
	coordinates.vbo.bind();
	coordinates.vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	coordinates.vbo.allocate(coordinatesVertexData, sizeof(coordinatesVertexData));

	coordinates.vao.create();
	coordinates.vao.bind();

	coordinates.program.enableAttributeArray("position");
	coordinates.program.enableAttributeArray("color");
	coordinates.program.enableAttributeArray("distance");
	coordinates.program.setAttributeBuffer("position", GL_FLOAT, 0, 3, 7 * sizeof(GLfloat));
	coordinates.program.setAttributeBuffer("color", GL_FLOAT, 3 * sizeof(GLfloat), 3, 7 * sizeof(GLfloat));
	coordinates.program.setAttributeBuffer("distance", GL_FLOAT, 6 * sizeof(GLfloat), 1, 7 * sizeof(GLfloat));

	coordinates.vao.release();
	coordinates.vbo.release();
	coordinates.program.release();

	// BACKGROUND //

	std::array<float, 30> backgroundVertexData;
	generateBackgroundVertices(backgroundVertexData, settings.backgroundColor);

	background.program.addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/background.vert");
	background.program.addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/background.frag");
	background.program.link();
	background.program.bind();

	background.vbo.create();
	background.vbo.bind();
	background.vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	background.vbo.allocate(backgroundVertexData.data(), sizeof(backgroundVertexData));

	background.vao.create();
	background.vao.bind();

	background.program.enableAttributeArray("position");
	background.program.enableAttributeArray("color");
	background.program.setAttributeBuffer("position", GL_FLOAT, 0, 2, 5 * sizeof(GLfloat));
	background.program.setAttributeBuffer("color", GL_FLOAT, 2 * sizeof(GLfloat), 3, 5 * sizeof(GLfloat));

	background.vao.release();
	background.vbo.release();
	background.program.release();

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

	glClearColor(settings.backgroundColor.redF(), settings.backgroundColor.greenF(), settings.backgroundColor.blueF(), settings.backgroundColor.alphaF());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(4.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// BACKGROUND //

	if (renderBackground)
	{
		background.program.bind();
		background.vao.bind();

		glDrawArrays(GL_TRIANGLES, 0, 6);

		background.vao.release();
		background.program.release();
	}

	// COORDINATES //

	if (renderCoordinates)
	{
		coordinates.program.bind();
		coordinates.vao.bind();

		coordinates.program.setUniformValue("mvp", coordinates.mvp);

		glDrawArrays(GL_LINES, 0, 6);

		coordinates.vao.release();
		coordinates.program.release();
	}

	// CUBE //

	cube.program.bind();
	cube.vao.bind();

	cube.program.setUniformValue("lineColor", settings.lineColor);
	cube.program.setUniformValue("mvp", cube.mvp);

	glDrawArrays(GL_LINES, 0, 24);

	cube.vao.release();
	cube.program.release();

	// PLANE //

	plane.program.bind();
	plane.vao.bind();

	if (volumeTexture.isCreated())
		volumeTexture.bind();

	plane.program.setUniformValue("texture0", 0);
	plane.program.setUniformValue("modelMatrix", plane.modelMatrix);
	plane.program.setUniformValue("mvp", plane.mvp);
	plane.program.setUniformValue("scaleY", settings.imageHeight / settings.imageWidth);
	plane.program.setUniformValue("scaleZ", settings.imageDepth / settings.imageWidth);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	if (volumeTexture.isCreated())
		volumeTexture.release();

	plane.vao.release();
	plane.program.release();

	// TEXTS //

	QLocale locale(QLocale::English);

	QVector3D realCameraPosition = cameraPosition * settings.imageWidth;
	float realPlaneDistance = planeDistance * settings.imageWidth;

	QFont font("mono", 10, QFont::Normal);
	font.setHintingPreference(QFont::PreferFullHinting);
	font.setStyleStrategy(QFont::PreferAntialias);

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::TextAntialiasing);
	painter.setRenderHint(QPainter::SmoothPixmapTransform);
	painter.setRenderHint(QPainter::HighQualityAntialiasing);
	painter.setPen(Qt::white);
	painter.setFont(QFont("mono", 10, QFont::Normal));
	painter.drawText(5, 15, QString("Camera position: (%1, %2, %3)").arg(locale.toString(realCameraPosition.x(), 'e', 6), locale.toString(realCameraPosition.y(), 'e', 6), locale.toString(realCameraPosition.z(), 'e', 6)));
	painter.drawText(5, 32, QString("Plane distance: %1").arg(locale.toString(realPlaneDistance, 'e', 6)));
}

void RenderWidget::generateCubeVertices(std::array<QVector3D, 72>& cubeVertexData, std::array<QVector3D, 24>& cubeLinesVertexData, float width, float height, float depth)
{
	float actualWidth = 1.0f;
	float actualHeight = height / width;
	float actualDepth = depth / width;

	QVector3D v1(0, 0, actualDepth); 
	QVector3D v2(actualWidth, 0, actualDepth); 
	QVector3D v3(0, actualHeight, actualDepth); 
	QVector3D v4(actualWidth, actualHeight, actualDepth); 
	QVector3D v5(0, 0, 0); 
	QVector3D v6(actualWidth, 0, 0); 
	QVector3D v7(0, actualHeight, 0); 
	QVector3D v8(actualWidth, actualHeight, 0); 

	QVector3D t1(0, 0, 1);
	QVector3D t2(1, 0, 1);
	QVector3D t3(0, 1, 1);
	QVector3D t4(1, 1, 1);
	QVector3D t5(0, 0, 0);
	QVector3D t6(1, 0, 0);
	QVector3D t7(0, 1, 0);
	QVector3D t8(1, 1, 0);

	std::array<QVector3D, 72> cubeVertexDataTemp =
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

	cubeVertexData = cubeVertexDataTemp;

	std::array<QVector3D, 24> cubeLinesVertexDataTemp =
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

	cubeLinesVertexData = cubeLinesVertexDataTemp;
}

void RenderWidget::generateBackgroundVertices(std::array<float, 30>& backgroundVertexData, QColor color)
{
	float alpha1 = 0.6f;
	float alpha2 = 1.4f;

	float red1 = std::max(0.0f, std::min(1.0f, float(alpha1 * color.redF())));
	float green1 = std::max(0.0f, std::min(1.0f, float(alpha1 * color.greenF())));
	float blue1 = std::max(0.0f, std::min(1.0f, float(alpha1 * color.blueF())));

	float red2 = std::max(0.0f, std::min(1.0f, float(alpha2 * color.redF())));
	float green2 = std::max(0.0f, std::min(1.0f, float(alpha2 * color.greenF())));
	float blue2 = std::max(0.0f, std::min(1.0f, float(alpha2 * color.blueF())));

	std::array<float, 30> backgroundVertexDataTemp =
	{
		-1.0f, -1.0f, red1, green1, blue1,
		1.0f, -1.0f, red1, green1, blue1,
		1.0f, 1.0f, red2, green2, blue2,

		-1.0f, -1.0f, red1, green1, blue1,
		1.0f, 1.0f, red2, green2, blue2,
		-1.0f, 1.0f, red2, green2, blue2,
	};

	backgroundVertexData = backgroundVertexDataTemp;
}

void RenderWidget::updateLogic()
{
	float timeStep = timeStepTimer.nsecsElapsed() / 1000000000.0f;
	timeStepTimer.restart();

	if (keyboardHelper.keyIsDownOnce(Qt::Key_U))
		moveSpeedModifier *= 2.0f;

	if (keyboardHelper.keyIsDownOnce(Qt::Key_J))
		moveSpeedModifier *= 0.5f;

	if (keyboardHelper.keyIsDownOnce(Qt::Key_I))
		mouseWheelSpeedModifier *= 2.0f;

	if (keyboardHelper.keyIsDownOnce(Qt::Key_K))
		mouseWheelSpeedModifier *= 0.5f;

	if (keyboardHelper.keyIsDownOnce(Qt::Key_O))
		mouseSpeedModifier *= 2.0f;

	if (keyboardHelper.keyIsDownOnce(Qt::Key_L))
		mouseSpeedModifier *= 0.5f;

	float moveSpeed = moveSpeedModifier;

	if (keyboardHelper.keyIsDown(Qt::Key_Shift))
		moveSpeed *= 2.0f;

	if (keyboardHelper.keyIsDown(Qt::Key_Control))
		moveSpeed *= 0.5f;

	if (keyboardHelper.keyIsDown(Qt::Key_R))
		resetCamera();

	if (keyboardHelper.keyIsDownOnce(Qt::Key_B))
		renderBackground = !renderBackground;

	if (keyboardHelper.keyIsDownOnce(Qt::Key_C))
		renderCoordinates = !renderCoordinates;

	cameraRight = cameraMatrix.column(0).toVector3D();
	cameraUp = cameraMatrix.column(1).toVector3D();
	cameraForward = -cameraMatrix.column(2).toVector3D();

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
	projectionMatrix.perspective(45.0f, aspectRatio, 0.001f, 100.0f);

	cube.modelMatrix.setToIdentity();
	cube.mvp = projectionMatrix * viewMatrix * cube.modelMatrix;

	QVector3D planePosition = cameraPosition + planeDistance * cameraForward;

	plane.modelMatrix.setToIdentity();
	plane.modelMatrix.setColumn(0, QVector4D(cameraRight.x(), cameraRight.y(), cameraRight.z(), 0.0f));
	plane.modelMatrix.setColumn(1, QVector4D(cameraUp.x(), cameraUp.y(), cameraUp.z(), 0.0f));
	plane.modelMatrix.setColumn(2, QVector4D(cameraForward.x(), cameraForward.y(), cameraForward.z(), 0.0f));
	plane.modelMatrix.setColumn(3, QVector4D(planePosition.x(), planePosition.y(), planePosition.z(), 1.0f));
	plane.mvp = projectionMatrix * viewMatrix * plane.modelMatrix;

	coordinates.modelMatrix.setToIdentity();
	coordinates.mvp = projectionMatrix * viewMatrix * coordinates.modelMatrix;
}

void RenderWidget::resetCamera()
{
	float distance = 2.3f;
	float depth = settings.imageDepth / settings.imageWidth;

	cameraPosition = QVector3D(0.5f, 0.5f, distance);
	cameraRotation = QVector2D(0.0f, 0.0f);
	cameraMatrix.setToIdentity();

	planeDistance = distance - depth / 2.0f;

	moveSpeedModifier = 1.0f;
	mouseSpeedModifier = 0.25f;
	mouseWheelSpeedModifier = 0.05f;
}
