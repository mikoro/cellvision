// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#pragma once

#include <array>

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

	struct RenderWidgetSettings
	{
		std::string imageFileName;
		QColor backgroundColor = QColor(100, 100, 100, 255);
		QColor lineColor = QColor(255, 255, 255, 128);
		float imageWidth = 1.0f;
		float imageHeight = 1.0f;
		float imageDepth = 1.0f;
	};

	class RenderWidget : public QOpenGLWidget, protected QOpenGLFunctions
	{
		Q_OBJECT

	public:

		explicit RenderWidget(QWidget* parent = nullptr);
		
		void initialize(const RenderWidgetSettings& settings);
		
	protected:

		bool event(QEvent* e) override;

		void mousePressEvent(QMouseEvent* me) override;
		void mouseMoveEvent(QMouseEvent* me) override;
		void wheelEvent(QWheelEvent* we) override;

		void initializeGL() override;
		void resizeGL(int width, int height) override;
		void paintGL() override;

	private:

		void generateCubeVertices(std::array<QVector3D, 72>& cubeVertexData, std::array<QVector3D, 24>& cubeLinesVertexData, float width, float height, float depth);

		void updateLogic();
		void resetCamera();

		RenderWidgetSettings settings;

		KeyboardHelper keyboardHelper;
		QPoint previousMousePosition;
		QElapsedTimer timeStepTimer;
		QVector3D cameraPosition;
		QVector2D cameraRotation;
		QMatrix4x4 cameraMatrix;
		float moveSpeedModifier = 1.0f;
		float mouseSpeedModifier = 0.25f;
		float planeDistance = 1.0f;
		bool renderCoordinates = true;

		QOpenGLTexture volumeTexture;

		OpenGLData cube;
		OpenGLData cubeLines;
		OpenGLData plane;
		OpenGLData coordinateLines;
	};
}
