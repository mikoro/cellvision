#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include "ImageLoader.h"

namespace Ui
{
	class MainWindowClass;
}

namespace CellVision
{
	class RenderWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_2_Core
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

		Ui::MainWindowClass* ui = nullptr;

		QOpenGLBuffer defaultVbo;
		QOpenGLVertexArrayObject defaultVao;
		QOpenGLShaderProgram defaultProgram;
		QOpenGLTexture defaultTexture;
	};
}
