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
		
		void setUI(Ui::MainWindowClass* ui);
		void uploadImageData(const ImageLoaderResult& result);

	protected:

		bool event(QEvent* event) override;
		void initializeGL() override;
		void resizeGL(int width, int height) override;
		void paintGL() override;

	private:

		void updateLogic();
		void resetCamera();

		Ui::MainWindowClass* ui = nullptr;

		QElapsedTimer timer;

		QOpenGLTexture volumeTexture;

		QOpenGLBuffer cubeVbo;
		QOpenGLVertexArrayObject cubeVao;
		QOpenGLShaderProgram cubeProgram;
		QMatrix4x4 cubeModel;
		QMatrix4x4 cubeMvp;

		QOpenGLBuffer cubeLinesVbo;
		QOpenGLVertexArrayObject cubeLinesVao;
		QOpenGLShaderProgram cubeLinesProgram;

		QOpenGLBuffer planeVbo;
		QOpenGLVertexArrayObject planeVao;
		QOpenGLShaderProgram planeProgram;
		QMatrix4x4 planeModel;
		QMatrix4x4 planeMvp;

		QOpenGLBuffer planeLinesVbo;
		QOpenGLVertexArrayObject planeLinesVao;
		QOpenGLShaderProgram planeLinesProgram;

		QVector3D cameraPosition;
		QVector2D cameraRotation;
	};
}
