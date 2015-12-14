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
	defaultTexture.setWrapMode(QOpenGLTexture::ClampToEdge);
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

	defaultProgram.setUniformValue("texcoordZ", float(ui->doubleSpinBoxZDepth->value()));
	defaultProgram.setUniformValue("texture0", 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	if (defaultTexture.isCreated())
		defaultTexture.release();

	defaultVao.release();
	defaultProgram.release();
}

void RenderWidget::updateLogic()
{
	double timeStep = timer.nsecsElapsed() / 1000000000.0;
	timer.restart();
}
