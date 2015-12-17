#include "stdafx.h"

#include "RenderWidget.h"
#include "MainWindow.h"

using namespace CellVision;

RenderWidget::RenderWidget(QWidget* parent) : QOpenGLWidget(parent), defaultTexture(QOpenGLTexture::Target3D)
{
	connect(this, SIGNAL(frameSwapped()), this, SLOT(update()));
}

void RenderWidget::setUI(Ui::MainWindowClass* ui_)
{
	ui = ui_;
}

void RenderWidget::uploadImageData(const ImageLoaderResult& result)
{
	defaultTexture.destroy();
	defaultTexture.create();
	defaultTexture.bind();
	defaultTexture.setFormat(QOpenGLTexture::RGBA8_UNorm);
	defaultTexture.setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
	defaultTexture.setWrapMode(QOpenGLTexture::ClampToBorder);
	defaultTexture.setBorderColor(0.0f, 0.0f, 0.0f, 1.0f);
	defaultTexture.setMipLevels(1);
	defaultTexture.setSize(result.width, result.height, result.depth);
	defaultTexture.allocateStorage(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8);
	defaultTexture.setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, &result.data[0]);
	defaultTexture.release();
}

void RenderWidget::initializeGL()
{
	initializeOpenGLFunctions();

	const GLfloat vertexData[] =
	{
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 0.0f, 1.0f,
	};

	defaultProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/default.vert");
	defaultProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/default.frag");
	defaultProgram.link();
	defaultProgram.bind();

	defaultVbo.create();
	defaultVbo.bind();
	defaultVbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
	defaultVbo.allocate(vertexData, sizeof(vertexData));

	defaultVao.create();
	defaultVao.bind();

	defaultProgram.enableAttributeArray("position");
	defaultProgram.enableAttributeArray("texcoord");
	defaultProgram.setAttributeBuffer("position", GL_FLOAT, 0, 2, 4 * sizeof(GLfloat));
	defaultProgram.setAttributeBuffer("texcoord", GL_FLOAT, 2 * sizeof(GLfloat), 2, 4 * sizeof(GLfloat));
	
	defaultVao.release();
	defaultVbo.release();
	defaultProgram.release();

	timer.start();
}

void RenderWidget::resizeGL(int width, int height)
{
}

void RenderWidget::paintGL()
{
	updateLogic();

	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	defaultProgram.bind();
	defaultVao.bind();
	
	if (defaultTexture.isCreated())
		defaultTexture.bind();

	defaultProgram.setUniformValue("texture0", 0);
	defaultProgram.setUniformValue("objectToWorld", objectToWorld);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	if (defaultTexture.isCreated())
		defaultTexture.release();

	defaultVao.release();
	defaultProgram.release();
}

void RenderWidget::updateLogic()
{
	float timeStep = timer.nsecsElapsed() / 1000000000.0f;
	timer.restart();

	if (MainWindow::keyIsDown(Qt::Key_R))
	{
		cameraPosition = QVector3D();
		cameraRotation = QVector2D();
	}

	cameraRotation[0] += MainWindow::getMouseDelta().y() * 0.1f * timeStep;
	cameraRotation[1] += MainWindow::getMouseDelta().x() * 0.1f * timeStep;

	//tfm::printfln("x: %f y: %f", cameraRotation[0], cameraRotation[1]);

	QVector3D cameraForward;

	// is [0 0 -1] when angles are zero
	cameraForward[0] = -sin(cameraRotation[1]) * cos(cameraRotation[0]);
	cameraForward[1] = sin(cameraRotation[0]);
	cameraForward[2] = -cos(cameraRotation[1]) * cos(cameraRotation[0]);
	cameraForward = -cameraForward;
	cameraForward.normalize();

	QVector3D cameraRight = QVector3D::crossProduct(QVector3D(0.0001f, 1.0f, 0.0001f), cameraForward).normalized();
	QVector3D cameraUp = QVector3D::crossProduct(cameraForward, cameraRight).normalized();

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

	objectToWorld.setToIdentity();
	objectToWorld.setColumn(0, QVector4D(cameraRight[0], cameraRight[1], cameraRight[2], 0.0f));
	objectToWorld.setColumn(1, QVector4D(cameraUp[0], cameraUp[1], cameraUp[2], 0.0f));
	objectToWorld.setColumn(2, QVector4D(cameraForward[0], cameraForward[1], cameraForward[2], 0.0f));
	objectToWorld.setColumn(3, QVector4D(cameraPosition[0], cameraPosition[1], cameraPosition[2], 1.0f));
}
