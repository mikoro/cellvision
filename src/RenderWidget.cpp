// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#include "stdafx.h"

#include "RenderWidget.h"
#include "MainWindow.h"
#include "Log.h"
#include "ImageLoader.h"
#include "MathHelper.h"

using namespace CellVision;

RenderWidget::RenderWidget(QWidget* parent) : QOpenGLWidget(parent), volumeTexture(QOpenGLTexture::Target3D), textTexture(QOpenGLTexture::Target2D)
{
	connect(this, SIGNAL(frameSwapped()), this, SLOT(update()));

	setFocus();
}

RenderWidget::~RenderWidget()
{
	QSettings settings("cellvision.ini", QSettings::IniFormat);

	settings.setValue("moveSpeedModifier", double(moveSpeedModifier));
	settings.setValue("mouseMoveSpeedModifier", double(mouseMoveSpeedModifier));
	settings.setValue("mouseRotateSpeedModifier", double(mouseRotateSpeedModifier));
	settings.setValue("mouseWheelStepSizeModifier", double(mouseWheelStepSizeModifier));
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
		volumeTexture.allocateStorage();
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

	resetCameraPosition();
	loadCameraSpeeds();
}

bool RenderWidget::event(QEvent* e)
{
	keyboardHelper.event(e);

	if (e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease)
	{
		QKeyEvent* ke = static_cast<QKeyEvent*>(e);

		if (ke->key() == Qt::Key_Space)
			setMouseMode();
	}

	return QOpenGLWidget::event(e);
}

void RenderWidget::mousePressEvent(QMouseEvent* me)
{
	setFocus();

	mouseButtons = me->buttons();
	setMouseMode();

	previousMousePosition = me->globalPos();
	
	if (mouseMode == MouseMode::ORBIT)
		orbitPointWorld = getPlaneIntersection(me->localPos());
	else if (mouseMode == MouseMode::MEASURE)
		measureStartPoint = measureEndPoint = getPlaneIntersection(me->localPos());

	me->accept();
}

void RenderWidget::mouseReleaseEvent(QMouseEvent* me)
{
	mouseButtons = me->buttons();
	setMouseMode();
	me->accept();
}

void RenderWidget::mouseMoveEvent(QMouseEvent* me)
{
	QPoint mouseDelta = me->globalPos() - previousMousePosition;
	previousMousePosition = me->globalPos();

	float yawAmount = -mouseDelta.x() * mouseRotateSpeedModifier;
	float pitchAmount = -mouseDelta.y() * mouseRotateSpeedModifier;
	float moveSpeed = mouseMoveSpeedModifier;
	
	if (keyboardHelper.keyIsDown(Qt::Key_Shift))
		moveSpeed *= 2.0f;

	if (keyboardHelper.keyIsDown(Qt::Key_Control))
		moveSpeed *= 0.5f;

	if (mouseMode == MouseMode::ROTATE)
	{
		if (keyboardHelper.keyIsDown(Qt::Key_Space))
		{
			QVector3D rollAxis = QVector3D(0, 0, 1);
			QMatrix4x4 rollMatrix = MathHelper::rotationMatrix(yawAmount, rollAxis);
			cameraOrientationMatrix = cameraOrientationMatrix * rollMatrix;
		}
		else
		{
			QVector3D yawAxis = QVector3D(0, 1, 0);
			QMatrix4x4 yawMatrix = MathHelper::rotationMatrix(yawAmount, yawAxis);
			cameraOrientationMatrix = cameraOrientationMatrix * yawMatrix;

			QVector3D pitchAxis = QVector3D(1, 0, 0);
			QMatrix4x4 pitchMatrix = MathHelper::rotationMatrix(pitchAmount, pitchAxis);
			cameraOrientationMatrix = cameraOrientationMatrix * pitchMatrix;
		}
		
		MathHelper::orthonormalize(cameraOrientationMatrix);
		cameraOrientationInvMatrix = cameraOrientationMatrix.inverted();
	}
	else if (mouseMode == MouseMode::ORBIT)
	{
		orbitPointCamera = viewMatrix * orbitPointWorld;

		QVector3D yawAxis = QVector3D(0, 1, 0);
		QMatrix4x4 yawMatrix = MathHelper::rotationMatrix(yawAmount, yawAxis);
		cameraOrientationMatrix = cameraOrientationMatrix * yawMatrix;

		QVector3D pitchAxis = QVector3D(1, 0, 0);
		QMatrix4x4 pitchMatrix = MathHelper::rotationMatrix(pitchAmount, pitchAxis);
		cameraOrientationMatrix = cameraOrientationMatrix * pitchMatrix;

		MathHelper::orthonormalize(cameraOrientationMatrix);
		cameraOrientationInvMatrix = cameraOrientationMatrix.inverted();

		cameraPosition = orbitPointWorld - cameraOrientationMatrix * orbitPointCamera;
	}
	else if (mouseMode == MouseMode::PAN)
		cameraPosition += (cameraRight * -mouseDelta.x() * moveSpeed + cameraUp * mouseDelta.y() * moveSpeed) * planeDistance;
	else if (mouseMode == MouseMode::ZOOM)
		cameraPosition += cameraForward * -mouseDelta.y() * moveSpeed;
	else if (mouseMode == MouseMode::MEASURE)
	{
		measureEndPoint = getPlaneIntersection(me->localPos());
		measureDistance = (measureEndPoint - measureStartPoint).length();
	}

	me->accept();
}

void RenderWidget::wheelEvent(QWheelEvent* we)
{
	float stepSize = mouseWheelStepSizeModifier;

	if (keyboardHelper.keyIsDown(Qt::Key_Shift))
		stepSize *= 2.0f;

	if (keyboardHelper.keyIsDown(Qt::Key_Control))
		stepSize *= 0.5f;

	float moveAmount = we->angleDelta().y() / 120.0f * stepSize;

	cameraPosition += cameraForward * moveAmount;
	planeDistance -= moveAmount;

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

	// MINI COORDINATES //

	const float miniCoordinatesVertexData[] =
	{
		0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f
	};

	miniCoordinates.program.addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/miniCoordinates.vert");
	miniCoordinates.program.addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/miniCoordinates.frag");
	miniCoordinates.program.link();
	miniCoordinates.program.bind();

	miniCoordinates.vbo.create();
	miniCoordinates.vbo.bind();
	miniCoordinates.vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	miniCoordinates.vbo.allocate(miniCoordinatesVertexData, sizeof(miniCoordinatesVertexData));

	miniCoordinates.vao.create();
	miniCoordinates.vao.bind();

	miniCoordinates.program.enableAttributeArray("position");
	miniCoordinates.program.enableAttributeArray("color");
	miniCoordinates.program.enableAttributeArray("distance");
	miniCoordinates.program.setAttributeBuffer("position", GL_FLOAT, 0, 3, 7 * sizeof(GLfloat));
	miniCoordinates.program.setAttributeBuffer("color", GL_FLOAT, 3 * sizeof(GLfloat), 3, 7 * sizeof(GLfloat));
	miniCoordinates.program.setAttributeBuffer("distance", GL_FLOAT, 6 * sizeof(GLfloat), 1, 7 * sizeof(GLfloat));

	miniCoordinates.vao.release();
	miniCoordinates.vbo.release();
	miniCoordinates.program.release();

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

	// MEASUREMENT //

	const float measurementVertexData[] =
	{
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
	};

	measurement.program.addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/measurement.vert");
	measurement.program.addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/measurement.frag");
	measurement.program.link();
	measurement.program.bind();

	measurement.vbo.create();
	measurement.vbo.bind();
	measurement.vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
	measurement.vbo.allocate(measurementVertexData, sizeof(measurementVertexData));

	measurement.vao.create();
	measurement.vao.bind();

	measurement.program.enableAttributeArray("position");
	measurement.program.setAttributeBuffer("position", GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));

	measurement.vao.release();
	measurement.vbo.release();
	measurement.program.release();

	// TEXT //

	const float textVertexData[] =
	{
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f,

		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 0.0f, 1.0f
	};

	text.program.addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/text.vert");
	text.program.addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/text.frag");
	text.program.link();
	text.program.bind();

	text.vbo.create();
	text.vbo.bind();
	text.vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	text.vbo.allocate(textVertexData, sizeof(textVertexData));

	text.vao.create();
	text.vao.bind();

	text.program.enableAttributeArray("position");
	text.program.enableAttributeArray("texcoord");
	text.program.setAttributeBuffer("position", GL_FLOAT, 0, 2, 4 * sizeof(GLfloat));
	text.program.setAttributeBuffer("texcoord", GL_FLOAT, 2 * sizeof(GLfloat), 2, 4 * sizeof(GLfloat));

	text.vao.release();
	text.vbo.release();
	text.program.release();

	// MISC //

	timeStepTimer.start();
	resetCameraPosition();
	loadCameraSpeeds();
	updateCamera();
}

void RenderWidget::resizeGL(int width, int height)
{
	float aspectRatio = float(width) / float(height);
	projectionMatrix.setToIdentity();
	projectionMatrix.perspective(45.0f, aspectRatio, 0.001f, 100.0f);

	textImage = QImage(width, height, QImage::Format_RGBA8888);
	textImage.fill(QColor(0, 0, 0, 0));

	textTexture.destroy();
	textTexture.create();
	textTexture.bind();
	textTexture.setData(textImage);
	textTexture.release();
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

	plane.program.setUniformValue("tex0", 0);
	plane.program.setUniformValue("modelMatrix", plane.modelMatrix);
	plane.program.setUniformValue("mvp", plane.mvp);
	plane.program.setUniformValue("scaleY", settings.imageHeight / settings.imageWidth);
	plane.program.setUniformValue("scaleZ", settings.imageDepth / settings.imageWidth);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	if (volumeTexture.isCreated())
		volumeTexture.release();

	plane.vao.release();
	plane.program.release();

	// MINI COORDINATES //

	if (renderMiniCoordinates)
	{
		glViewport(-50, -50, 200, 200);

		miniCoordinates.program.bind();
		miniCoordinates.vao.bind();

		miniCoordinates.program.setUniformValue("mvp", miniCoordinates.mvp);

		glDrawArrays(GL_LINES, 0, 6);

		miniCoordinates.vao.release();
		miniCoordinates.program.release();

		glViewport(0, 0, width(), height());
	}

	// MEASUREMENT //

	if (mouseMode == MouseMode::MEASURE)
	{
		measurement.program.bind();
		measurement.vao.bind();

		measurement.program.setUniformValue("lineColor", settings.lineColor);
		measurement.program.setUniformValue("mvp", measurement.mvp);

		glDrawArrays(GL_LINES, 0, 2);

		measurement.vao.release();
		measurement.program.release();
	}

	// TEXT //

	if (renderText)
	{
		QLocale locale(QLocale::English);

		QVector3D realCameraPosition = cameraPosition * settings.imageWidth;
		float realMeasuredDistance = measureDistance * settings.imageWidth;

		textImage.fill(QColor(0, 0, 0, 0));

		QPainter painter(&textImage);
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setRenderHint(QPainter::TextAntialiasing);
		painter.setRenderHint(QPainter::SmoothPixmapTransform);
		painter.setRenderHint(QPainter::HighQualityAntialiasing);

		painter.setPen(QColor(0, 0, 0, 96));
		painter.setBrush(QColor(0, 0, 0, 64));
		painter.drawRoundRect(-20, -360, 400, 400, 10, 10);

#ifdef __APPLE__
		int textSize = 12;
#else
		int textSize = 10;
#endif

		QFont font("Roboto Mono", textSize, QFont::Normal);
		font.setHintingPreference(QFont::PreferFullHinting);
		font.setStyleStrategy(QFont::PreferAntialias);

		painter.setBrush(Qt::NoBrush);
		painter.setPen(QColor(255, 255, 255, 255));
		painter.setFont(font);
		
		painter.drawText(5, 15, QString("Position: (%1, %2, %3)").arg(locale.toString(realCameraPosition.x(), 'e', 3), locale.toString(realCameraPosition.y(), 'e', 3), locale.toString(realCameraPosition.z(), 'e', 3)));
		painter.drawText(5, 32, QString("Distance: %1").arg(locale.toString(realMeasuredDistance, 'e', 3)));

		textTexture.bind();
		textTexture.setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, textImage.bits());

		text.program.bind();
		text.vao.bind();

		text.program.setUniformValue("tex0", 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		text.vao.release();
		text.program.release();

		textTexture.release();
	}
}

void RenderWidget::updateLogic()
{
	float timeStep = timeStepTimer.nsecsElapsed() / 1000000000.0f;
	timeStepTimer.restart();

	if (keyboardHelper.keyIsDownOnce(Qt::Key_Y))
		moveSpeedModifier *= 2.0f;

	if (keyboardHelper.keyIsDownOnce(Qt::Key_H))
		moveSpeedModifier *= 0.5f;

	if (keyboardHelper.keyIsDownOnce(Qt::Key_U))
		mouseMoveSpeedModifier *= 2.0f;

	if (keyboardHelper.keyIsDownOnce(Qt::Key_J))
		mouseMoveSpeedModifier *= 0.5f;

	if (keyboardHelper.keyIsDownOnce(Qt::Key_I))
		mouseRotateSpeedModifier *= 2.0f;

	if (keyboardHelper.keyIsDownOnce(Qt::Key_K))
		mouseRotateSpeedModifier *= 0.5f;

	if (keyboardHelper.keyIsDownOnce(Qt::Key_O))
		mouseWheelStepSizeModifier *= 2.0f;

	if (keyboardHelper.keyIsDownOnce(Qt::Key_L))
		mouseWheelStepSizeModifier *= 0.5f;

	float moveSpeed = moveSpeedModifier;

	if (keyboardHelper.keyIsDown(Qt::Key_Shift))
		moveSpeed *= 2.0f;

	if (keyboardHelper.keyIsDown(Qt::Key_Control))
		moveSpeed *= 0.5f;

	if (keyboardHelper.keyIsDown(Qt::Key_R))
	{
		if (keyboardHelper.keyIsDown(Qt::Key_Control))
			resetCameraSpeeds();
		else
			resetCameraPosition();
	}

	if (keyboardHelper.keyIsDownOnce(Qt::Key_B))
		renderBackground = !renderBackground;

	if (keyboardHelper.keyIsDownOnce(Qt::Key_C))
		renderCoordinates = !renderCoordinates;

	if (keyboardHelper.keyIsDownOnce(Qt::Key_V))
		renderMiniCoordinates = !renderMiniCoordinates;

	if (keyboardHelper.keyIsDownOnce(Qt::Key_T))
		renderText = !renderText;

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

	updateCamera();

	// CUBE //

	cube.modelMatrix.setToIdentity();
	cube.mvp = projectionMatrix * viewMatrix * cube.modelMatrix;

	// PLANE //

	planePosition = cameraPosition + planeDistance * cameraForward;
	planeNormal = -cameraForward;

	plane.modelMatrix = cameraOrientationMatrix;
	plane.modelMatrix.setColumn(3, QVector4D(planePosition.x(), planePosition.y(), planePosition.z(), 1.0f));
	plane.mvp = projectionMatrix * viewMatrix * plane.modelMatrix;

	// COORDINATES //

	coordinates.modelMatrix.setToIdentity();
	coordinates.mvp = projectionMatrix * viewMatrix * coordinates.modelMatrix;

	// MINI COORDINATES //

	QVector3D miniCoordPosition = cameraPosition + 1.0f * cameraForward;
	QMatrix4x4 miniCoordProjection;
	miniCoordProjection.perspective(45.0f, 1.0f, 0.1f, 10.0f);
	
	miniCoordinates.modelMatrix.setToIdentity();
	miniCoordinates.modelMatrix.scale(0.2f);
	miniCoordinates.modelMatrix.setColumn(3, QVector4D(miniCoordPosition.x(), miniCoordPosition.y(), miniCoordPosition.z(), 1.0f));
	miniCoordinates.mvp = miniCoordProjection * viewMatrix * miniCoordinates.modelMatrix;

	// MEASUREMENT //

	if (mouseMode == MouseMode::MEASURE)
	{
		measurement.modelMatrix.setToIdentity();
		measurement.mvp = projectionMatrix * viewMatrix * measurement.modelMatrix;

		const QVector3D measurementVertexData[] = { measureStartPoint, measureEndPoint };

		measurement.vbo.bind();
		measurement.vbo.write(0, measurementVertexData, sizeof(measurementVertexData));
		measurement.vbo.release();
	}
}

void RenderWidget::updateCamera()
{
	cameraMatrix = cameraOrientationMatrix;
	cameraMatrix.setColumn(3, QVector4D(cameraPosition.x(), cameraPosition.y(), cameraPosition.z(), 1.0f));
	viewMatrix = cameraMatrix.inverted();

	cameraRight = cameraOrientationMatrix.column(0).toVector3D();
	cameraUp = cameraOrientationMatrix.column(1).toVector3D();
	cameraForward = -cameraOrientationMatrix.column(2).toVector3D();
}

void RenderWidget::resetCameraPosition()
{
	float distance = 2.3f;
	float depth = settings.imageDepth / settings.imageWidth;

	cameraPosition = QVector3D(0.5f, 0.5f, distance);
	cameraOrientationMatrix.setToIdentity();
	cameraOrientationInvMatrix.setToIdentity();
	planeDistance = distance - depth / 2.0f;
}

void RenderWidget::resetCameraSpeeds()
{
	moveSpeedModifier = 0.8f;
	mouseMoveSpeedModifier = 0.001f;
	mouseRotateSpeedModifier = 0.2f;
	mouseWheelStepSizeModifier = 0.05f;
}

void RenderWidget::loadCameraSpeeds()
{
	QSettings settings("cellvision.ini", QSettings::IniFormat);

	moveSpeedModifier = settings.value("moveSpeedModifier", 0.8f).toFloat();
	mouseMoveSpeedModifier = settings.value("mouseMoveSpeedModifier", 0.001f).toFloat();
	mouseRotateSpeedModifier = settings.value("mouseRotateSpeedModifier", 0.2f).toFloat();
	mouseWheelStepSizeModifier = settings.value("mouseWheelStepSizeModifier", 0.05f).toFloat();
}

void RenderWidget::setMouseMode()
{
	if (mouseButtons == Qt::LeftButton)
		mouseMode = MouseMode::ROTATE;
	else if (mouseButtons == Qt::RightButton)
		mouseMode = MouseMode::ORBIT;
	else if (mouseButtons == Qt::MidButton)
	{
		if (keyboardHelper.keyIsDown(Qt::Key_Space))
			mouseMode = MouseMode::ZOOM;
		else
			mouseMode = MouseMode::PAN;
	}
	else if (mouseButtons == (Qt::LeftButton | Qt::RightButton))
		mouseMode = MouseMode::MEASURE;
	else
		mouseMode = MouseMode::NONE;
}

QVector3D RenderWidget::getPlaneIntersection(const QPointF& mousePosition)
{
	float ndcX = mousePosition.x() / float(width());
	float ndcY = mousePosition.y() / float(height());
	ndcX = (ndcX - 0.5f) * 2.0f;
	ndcY = (ndcY - 0.5f) * -2.0f;

	QVector4D ndcPosition(ndcX, ndcY, 1.0f, 1.0f);
	QMatrix4x4 tempMatrix = (projectionMatrix * viewMatrix).inverted();
	QVector4D worldPosition = tempMatrix * ndcPosition;
	QVector3D worldPosition3d(worldPosition.x() / worldPosition.w(), worldPosition.y() / worldPosition.w(), worldPosition.z() / worldPosition.w());
	QVector3D rayDirection = (worldPosition3d - cameraPosition).normalized();

	float denominator = QVector3D::dotProduct(rayDirection, planeNormal);

	if (std::abs(denominator) < std::numeric_limits<double>::epsilon())
		return QVector3D();

	float t = QVector3D::dotProduct(planePosition - cameraPosition, planeNormal) / denominator;

	if (t < 0.0)
		return QVector3D();;

	return cameraPosition + (t * rayDirection);
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
