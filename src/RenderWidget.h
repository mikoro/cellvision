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
#include "KeyboardHelper.h"

namespace Ui
{
	class MainWindowClass;
}

namespace CellVision
{
	struct OpenGLData
	{
		QOpenGLBuffer vbo;
		QOpenGLVertexArrayObject vao;
		QOpenGLShaderProgram program;
		QMatrix4x4 modelMatrix;
		QMatrix4x4 mvp;
	};

	class RenderWidget : public QOpenGLWidget, protected QOpenGLFunctions
	{
		Q_OBJECT

	public:

		explicit RenderWidget(QWidget* parent = nullptr);
		
		void uploadImageData(const ImageLoaderResult& result);

	protected:

		bool event(QEvent* e) override;

		void mousePressEvent(QMouseEvent* me) override;
		void mouseMoveEvent(QMouseEvent* me) override;
		void wheelEvent(QWheelEvent* we) override;

		void initializeGL() override;
		void resizeGL(int width, int height) override;
		void paintGL() override;

	private:

		void updateLogic();
		void resetCamera();

		KeyboardHelper keyboardHelper;
		QPoint previousMousePosition;
		QElapsedTimer timeStepTimer;
		QVector3D cameraPosition;
		QVector2D cameraRotation;
		QMatrix4x4 cameraMatrix;
		float moveSpeedModifier = 1.0f;
		float mouseSpeedModifier = 0.25f;
		bool renderCoordinates = true;

		QOpenGLTexture volumeTexture;

		OpenGLData cube;
		OpenGLData cubeLines;
		OpenGLData plane;
		OpenGLData planeLines;
		OpenGLData coordinateLines;
	};
}
