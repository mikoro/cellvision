// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QElapsedTimer>

#include "ImageLoader.h"

namespace Ui
{
	class MainWindowClass;
}

namespace CellVision
{
	class RenderWidget : public QOpenGLWidget, protected QOpenGLFunctions
	{
		Q_OBJECT

	public:

		explicit RenderWidget(QWidget* parent = nullptr);
		
		void uploadImageData(const ImageLoaderResult& result);

	protected:

		void mousePressEvent(QMouseEvent* me) override;
		void mouseMoveEvent(QMouseEvent* me) override;
		void wheelEvent(QWheelEvent* we) override;

		void initializeGL() override;
		void resizeGL(int width, int height) override;
		void paintGL() override;

	private:

		void updateLogic();
		void resetCamera();

		QPoint previousMousePosition;

		QElapsedTimer timeStepTimer;

		QOpenGLTexture volumeTexture;

		QOpenGLBuffer cubeVbo;
		QOpenGLVertexArrayObject cubeVao;
		QOpenGLShaderProgram cubeProgram;
		QMatrix4x4 cubeModelMatrix;
		QMatrix4x4 cubeMvp;

		QOpenGLBuffer cubeLinesVbo;
		QOpenGLVertexArrayObject cubeLinesVao;
		QOpenGLShaderProgram cubeLinesProgram;

		QOpenGLBuffer planeVbo;
		QOpenGLVertexArrayObject planeVao;
		QOpenGLShaderProgram planeProgram;
		QMatrix4x4 planeModelMatrix;
		QMatrix4x4 planeMvp;

		QOpenGLBuffer planeLinesVbo;
		QOpenGLVertexArrayObject planeLinesVao;
		QOpenGLShaderProgram planeLinesProgram;

		QVector3D cameraPosition;
		QMatrix4x4 viewMatrix;

		float moveSpeedModifier = 1.0f;
	};
}
