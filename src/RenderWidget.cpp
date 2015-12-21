#include "stdafx.h"

#include "RenderWidget.h"
#include "MainWindow.h"
#include "Log.h"

using namespace CellVision;

RenderWidget::RenderWidget(QWidget* parent) : QOpenGLWidget(parent), volumeTexture(QOpenGLTexture::Target3D)
{
	connect(this, SIGNAL(frameSwapped()), this, SLOT(update()));
}

void RenderWidget::setUI(Ui::MainWindowClass* ui_)
{
	ui = ui_;
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

bool RenderWidget::event(QEvent* event)
{
	if (event->type() == QEvent::MouseButtonPress)
	{
		setFocus();
	}

	return QOpenGLWidget::event(event);
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

	cubeProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/cube.vert");
	cubeProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/cube.frag");
	cubeProgram.link();
	cubeProgram.bind();

	cubeVbo.create();
	cubeVbo.bind();
	cubeVbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	cubeVbo.allocate(cubeVertexData, sizeof(cubeVertexData));

	cubeVao.create();
	cubeVao.bind();

	cubeProgram.enableAttributeArray("position");
	cubeProgram.enableAttributeArray("texcoord");
	cubeProgram.setAttributeBuffer("position", GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
	cubeProgram.setAttributeBuffer("texcoord", GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

	cubeVao.release();
	cubeVbo.release();
	cubeProgram.release();

	// CUBE LINES //

	const QVector3D cubeLinesVertexData[] =
	{
		v1, t1, v2, t2,
		v2, t2, v4, t4,
		v4, t4, v3, t3,
		v3, t3, v1, t1,
		v2, t2, v6, t6,
		v6, t6, v5, t5,
		v5, t5, v1, t1,
		v6, t6, v8, t8,
		v8, t8, v7, t7,
		v7, t7, v5, t5,
		v4, t4, v8, t8,
		v3, t3, v7, t7
	};

	cubeLinesProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/lines.vert");
	cubeLinesProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/lines.frag");
	cubeLinesProgram.link();
	cubeLinesProgram.bind();

	cubeLinesVbo.create();
	cubeLinesVbo.bind();
	cubeLinesVbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	cubeLinesVbo.allocate(cubeLinesVertexData, sizeof(cubeLinesVertexData));

	cubeLinesVao.create();
	cubeLinesVao.bind();

	cubeLinesProgram.enableAttributeArray("position");
	cubeLinesProgram.enableAttributeArray("texcoord");
	cubeLinesProgram.setAttributeBuffer("position", GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
	cubeLinesProgram.setAttributeBuffer("texcoord", GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

	cubeLinesVao.release();
	cubeLinesVbo.release();
	cubeLinesProgram.release();

	// PLANE //

	v1 = QVector3D(-1, -1, 0); t1 = QVector3D(-1, -1, 0);
	v2 = QVector3D(1, -1, 0); t2 = QVector3D(1, -1, 0);
	v3 = QVector3D(-1, 1, 0); t3 = QVector3D(-1, 1, 0);
	v4 = QVector3D(1, 1, 0); t4 = QVector3D(1, 1, 0);

	const QVector3D planeVertexData[] =
	{
		v1, t1, v2, t2, v4, t4,
		v1, t1, v4, t4, v3, t3
	};

	planeProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/plane.vert");
	planeProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/plane.frag");
	planeProgram.link();
	planeProgram.bind();

	planeVbo.create();
	planeVbo.bind();
	planeVbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	planeVbo.allocate(planeVertexData, sizeof(planeVertexData));

	planeVao.create();
	planeVao.bind();

	planeProgram.enableAttributeArray("position");
	planeProgram.enableAttributeArray("texcoord");
	planeProgram.setAttributeBuffer("position", GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
	planeProgram.setAttributeBuffer("texcoord", GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

	planeVao.release();
	planeVbo.release();
	planeProgram.release();

	// PLANE LINES //

	const QVector3D planeLinesVertexData[] =
	{
		v1, t1, v2, t2,
		v2, t2, v4, t4,
		v4, t4, v3, t3,
		v3, t3, v1, t1
	};

	planeLinesProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/lines.vert");
	planeLinesProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/lines.frag");
	planeLinesProgram.link();
	planeLinesProgram.bind();

	planeLinesVbo.create();
	planeLinesVbo.bind();
	planeLinesVbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	planeLinesVbo.allocate(planeLinesVertexData, sizeof(planeLinesVertexData));

	planeLinesVao.create();
	planeLinesVao.bind();

	planeLinesProgram.enableAttributeArray("position");
	planeLinesProgram.enableAttributeArray("texcoord");
	planeLinesProgram.setAttributeBuffer("position", GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
	planeLinesProgram.setAttributeBuffer("texcoord", GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

	planeLinesVao.release();
	planeLinesVbo.release();
	planeLinesProgram.release();

	// MISC //

	timer.start();
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

	// CUBE //

	cubeProgram.bind();
	cubeVao.bind();

	if (volumeTexture.isCreated())
		volumeTexture.bind();

	cubeProgram.setUniformValue("texture0", 0);
	cubeProgram.setUniformValue("mvp", cubeMvp);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	if (volumeTexture.isCreated())
		volumeTexture.release();

	cubeVao.release();
	cubeProgram.release();

	// CUBE LINES //

	cubeLinesProgram.bind();
	cubeLinesVao.bind();

	cubeLinesProgram.setUniformValue("lineColor", QVector4D(1.0f, 1.0f, 1.0f, 0.5f));
	cubeLinesProgram.setUniformValue("mvp", cubeMvp);

	glDrawArrays(GL_LINES, 0, 24);

	cubeLinesVao.release();
	cubeLinesProgram.release();

	// PLANE //

	glDisable(GL_DEPTH_TEST);

	planeProgram.bind();
	planeVao.bind();

	if (volumeTexture.isCreated())
		volumeTexture.bind();

	planeProgram.setUniformValue("texture0", 0);
	planeProgram.setUniformValue("model", planeModel);
	planeProgram.setUniformValue("mvp", planeMvp);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	if (volumeTexture.isCreated())
		volumeTexture.release();

	planeVao.release();
	planeProgram.release();

	// PLANE LINES //

	planeLinesProgram.bind();
	planeLinesVao.bind();

	planeLinesProgram.setUniformValue("lineColor", QVector4D(1.0f, 1.0f, 1.0f, 0.5f));
	planeLinesProgram.setUniformValue("mvp", planeMvp);

	glDrawArrays(GL_LINES, 0, 8);

	planeLinesVao.release();
	planeLinesProgram.release();
}

void RenderWidget::updateLogic()
{
	float timeStep = timer.nsecsElapsed() / 1000000000.0f;
	timer.restart();

	QMatrix4x4 view;
	view.setToIdentity();
	view.rotate(cameraRotation[0], QVector3D(1, 0, 0));
	view.rotate(cameraRotation[1], QVector3D(0, 1, 0));
	view.translate(cameraPosition);

	QMatrix4x4 projection;
	projection.setToIdentity();
	float aspectRatio = float(ui->renderWidget->width()) / float(ui->renderWidget->height());
	projection.perspective(45.0f, aspectRatio, 0.1f, 100.0f);

	float moveSpeed = 0.6f;

	if (MainWindow::keyIsDown(Qt::Key_Shift))
		moveSpeed *= 2.0f;

	if (MainWindow::keyIsDown(Qt::Key_Control))
		moveSpeed *= 0.5f;

	if (MainWindow::keyIsDown(Qt::Key_R))
		resetCamera();

	cameraRotation[0] += MainWindow::getMouseDelta().y() * 4.0f * timeStep;
	cameraRotation[1] += MainWindow::getMouseDelta().x() * 4.0f * timeStep;

	QVector3D cameraForward = -view.row(2).toVector3D().normalized();
	QVector3D cameraRight = view.row(0).toVector3D().normalized();
	QVector3D cameraUp = view.row(1).toVector3D().normalized();

	if (MainWindow::keyIsDown(Qt::Key_W))
		cameraPosition -= cameraForward * moveSpeed * timeStep;

	if (MainWindow::keyIsDown(Qt::Key_S))
		cameraPosition += cameraForward * moveSpeed * timeStep;

	if (MainWindow::keyIsDown(Qt::Key_D))
		cameraPosition -= cameraRight * moveSpeed * timeStep;

	if (MainWindow::keyIsDown(Qt::Key_A))
		cameraPosition += cameraRight * moveSpeed * timeStep;

	if (MainWindow::keyIsDown(Qt::Key_E))
		cameraPosition -= cameraUp * moveSpeed * timeStep;

	if (MainWindow::keyIsDown(Qt::Key_Q))
		cameraPosition += cameraUp * moveSpeed * timeStep;

	cubeModel.setToIdentity();
	cubeMvp = projection * view * cubeModel;

	QVector3D planePosition = -cameraPosition + 3.0f * cameraForward;

	planeModel.setToIdentity();
	planeModel.setColumn(0, QVector4D(cameraRight.x(), cameraRight.y(), cameraRight.z(), 0.0f));
	planeModel.setColumn(1, QVector4D(cameraUp.x(), cameraUp.y(), cameraUp.z(), 0.0f));
	planeModel.setColumn(2, QVector4D(cameraForward.x(), cameraForward.y(), cameraForward.z(), 0.0f));
	planeModel.setColumn(3, QVector4D(planePosition.x(), planePosition.y(), planePosition.z(), 1.0f));
	planeMvp = projection * view * planeModel;
}

void RenderWidget::resetCamera()
{
	cameraRotation = QVector2D(38.8330460f, -45.5694656f);
	cameraPosition = QVector3D(-2.71662402f, -2.97333407f, -2.68895960f);
}
