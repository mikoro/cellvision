#include "stdafx.h"

#include "RenderWidget.h"
#include "MainWindow.h"

using namespace CellVision;

RenderWidget::RenderWidget(QWidget* parent) : QOpenGLWidget(parent), volumeTexture(QOpenGLTexture::Target3D)
{
	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setStencilBufferSize(8);
	format.setSwapInterval(1);
	format.setSamples(8);
	//format.setVersion(3, 2);
	//format.setProfile(QSurfaceFormat::CoreProfile);
	setFormat(format);

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

void RenderWidget::initializeGL()
{
	initializeOpenGLFunctions();

#define VERTEX1 -1, -1, -1, 0, 0, 0
#define VERTEX2 1, -1, -1, 1, 0, 0
#define VERTEX3 -1, 1, -1, 0, 1, 0
#define VERTEX4 1, 1, -1, 1, 1, 0
#define VERTEX5 -1, -1, 1, 0, 0, 1
#define VERTEX6 1, -1, 1, 1, 0, 1
#define VERTEX7 -1, 1, 1, 0, 1, 1
#define VERTEX8 1, 1, 1, 1, 1, 1

	// CUBE //

	const GLfloat cubeVertexData[] =
	{
		VERTEX1, VERTEX2, VERTEX4,
		VERTEX1, VERTEX4, VERTEX3,
		VERTEX2, VERTEX6, VERTEX8,
		VERTEX2, VERTEX8, VERTEX4,
		VERTEX6, VERTEX5, VERTEX7,
		VERTEX6, VERTEX7, VERTEX8,
		VERTEX5, VERTEX1, VERTEX3,
		VERTEX5, VERTEX3, VERTEX7,
		VERTEX1, VERTEX6, VERTEX5,
		VERTEX1, VERTEX2, VERTEX6,
		VERTEX3, VERTEX8, VERTEX7,
		VERTEX3, VERTEX4, VERTEX8
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

	const GLfloat cubeLinesVertexData[] =
	{
		VERTEX1, VERTEX2,
		VERTEX2, VERTEX4,
		VERTEX4, VERTEX3,
		VERTEX3, VERTEX1,
		VERTEX2, VERTEX6,
		VERTEX6, VERTEX5,
		VERTEX5, VERTEX1,
		VERTEX6, VERTEX8,
		VERTEX8, VERTEX7,
		VERTEX7, VERTEX5,
		VERTEX4, VERTEX8,
		VERTEX3, VERTEX7
	};

	linesProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/lines.vert");
	linesProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/lines.frag");
	linesProgram.link();
	linesProgram.bind();

	cubeLinesVbo.create();
	cubeLinesVbo.bind();
	cubeLinesVbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	cubeLinesVbo.allocate(cubeLinesVertexData, sizeof(cubeLinesVertexData));

	cubeLinesVao.create();
	cubeLinesVao.bind();

	cubeProgram.enableAttributeArray("position");
	cubeProgram.enableAttributeArray("texcoord");
	cubeProgram.setAttributeBuffer("position", GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
	cubeProgram.setAttributeBuffer("texcoord", GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

	cubeLinesVao.release();
	cubeLinesVbo.release();
	linesProgram.release();

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
	glEnable(GL_MULTISAMPLE);
	glLineWidth(4.0f);

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

	linesProgram.bind();
	cubeLinesVao.bind();

	linesProgram.setUniformValue("mvp", cubeMvp);

	glDrawArrays(GL_LINES, 0, 24);

	cubeLinesVao.release();
	linesProgram.release();
}

void RenderWidget::updateLogic()
{
	float timeStep = timer.nsecsElapsed() / 1000000000.0f;
	timer.restart();

	QMatrix4x4 cubeModel;
	cubeModel.setToIdentity();

	QMatrix4x4 view;
	view.setToIdentity();
	view.rotate(cameraRotation[0], QVector3D(1, 0, 0));
	view.rotate(cameraRotation[1], QVector3D(0, 1, 0));
	view.translate(cameraPosition);

	QMatrix4x4 projection;
	projection.setToIdentity();
	float aspectRatio = float(ui->renderWidget->width()) / float(ui->renderWidget->height());
	projection.perspective(45.0f, aspectRatio, 0.1f, 100.0f);

	cubeMvp = projection * view * cubeModel;

	if (MainWindow::keyIsDown(Qt::Key_R))
		resetCamera();

	cameraRotation[0] += MainWindow::getMouseDelta().y() * 4.0f * timeStep;
	cameraRotation[1] += MainWindow::getMouseDelta().x() * 4.0f * timeStep;

	QVector3D cameraForward = view.row(2).toVector3D().normalized();
	QVector3D cameraRight = -view.row(0).toVector3D().normalized();
	QVector3D cameraUp = -view.row(1).toVector3D().normalized();

	if (MainWindow::keyIsDown(Qt::Key_W))
		cameraPosition += cameraForward * timeStep;

	if (MainWindow::keyIsDown(Qt::Key_S))
		cameraPosition -= cameraForward * timeStep;

	if (MainWindow::keyIsDown(Qt::Key_D))
		cameraPosition += cameraRight * timeStep;

	if (MainWindow::keyIsDown(Qt::Key_A))
		cameraPosition -= cameraRight * timeStep;

	if (MainWindow::keyIsDown(Qt::Key_E))
		cameraPosition += cameraUp * timeStep;

	if (MainWindow::keyIsDown(Qt::Key_Q))
		cameraPosition -= cameraUp * timeStep;
}

void RenderWidget::resetCamera()
{
	cameraRotation = QVector2D(36.2972794f, -45.6924248f);
	cameraPosition = QVector3D(-3.30962682f, -3.19945812f, -3.20841980f);
}
