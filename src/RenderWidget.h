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

		void initializeGL() override;
		void resizeGL(int width, int height) override;
		void paintGL() override;

	private:

		void updateLogic();

		Ui::MainWindowClass* ui = nullptr;

		QElapsedTimer timer;

		QOpenGLBuffer defaultVbo;
		QOpenGLVertexArrayObject defaultVao;
		QOpenGLShaderProgram defaultProgram;
		QOpenGLTexture defaultTexture;
	};
}
