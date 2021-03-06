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

#include "KeyboardHelper.h"
#include "ImageLoader.h"

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
		ImageLoaderInfo imageLoaderInfo;
		QColor backgroundColor = QColor(100, 100, 100, 255);
		QColor lineColor = QColor(255, 255, 255, 128);
		float imageWidth = 1.0f;
		float imageHeight = 1.0f;
		float imageDepth = 1.0f;
	};

	enum class MouseMode { NONE, ROTATE, ORBIT, PAN, ZOOM, MEASURE };

	class RenderWidget : public QOpenGLWidget, protected QOpenGLFunctions
	{
		Q_OBJECT

	public:

		explicit RenderWidget(QWidget* parent = nullptr);
		~RenderWidget();

		void initialize(const RenderWidgetSettings& settings);
		
	protected:

		bool event(QEvent* e) override;

		void mousePressEvent(QMouseEvent* me) override;
		void mouseReleaseEvent(QMouseEvent* me) override;
		void mouseMoveEvent(QMouseEvent* me) override;
		void wheelEvent(QWheelEvent* we) override;

		void initializeGL() override;
		void resizeGL(int width, int height) override;
		void paintGL() override;

	private:

		void updateLogic();
		void updateCamera();
		void resetCameraPosition();
		void resetCameraSpeeds();
		void loadCameraSpeeds();
		void setMouseMode();
		QVector3D getPlaneIntersection(const QPointF& mousePosition);
		void generateCubeVertices(std::array<QVector3D, 72>& cubeVertexData, std::array<QVector3D, 24>& cubeLinesVertexData, float width, float height, float depth);
		void generateBackgroundVertices(std::array<float, 30>& backgroundVertexData, QColor color);

		RenderWidgetSettings settings;

		KeyboardHelper keyboardHelper;
		Qt::MouseButtons mouseButtons;
		QPoint previousMousePosition;
		QElapsedTimer timeStepTimer;
		QVector3D cameraPosition;
		QMatrix4x4 cameraOrientationMatrix;
		QMatrix4x4 cameraOrientationInvMatrix;
		QMatrix4x4 cameraMatrix;
		QMatrix4x4 viewMatrix;
		QMatrix4x4 projectionMatrix;
		QVector3D cameraRight;
		QVector3D cameraUp;
		QVector3D cameraForward;
		QVector3D planePosition;
		QVector3D planeNormal;
		QVector3D measureStartPoint;
		QVector3D measureEndPoint;
		QVector3D orbitPointWorld;
		QVector3D orbitPointCamera;
		MouseMode mouseMode = MouseMode::NONE;
		float moveSpeedModifier = 0.0f;
		float mouseMoveSpeedModifier = 0.0f;
		float mouseRotateSpeedModifier = 0.0f;
		float mouseWheelStepSizeModifier = 0.0f;
		float planeDistance = 1.0f;
		float measureDistance = 0.0f;
		bool renderBackground = true;
		bool renderCoordinates = true;
		bool renderMiniCoordinates = true;
		bool renderText = true;

		QOpenGLTexture volumeTexture;
		QOpenGLTexture textTexture;
		QImage textImage;

		OpenGLData cube;
		OpenGLData plane;
		OpenGLData coordinates;
		OpenGLData miniCoordinates;
		OpenGLData background;
		OpenGLData measurement;
		OpenGLData text;
	};
}
