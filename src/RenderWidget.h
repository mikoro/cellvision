#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_2_Core>

namespace CellVision
{
	class RenderWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_2_Core
	{
		Q_OBJECT

	public:

		explicit RenderWidget(QWidget* parent = nullptr);

	protected:

		void initializeGL() override;
		void resizeGL(int width, int height) override;
		void paintGL() override;

	private:

		QOpenGLVertexArrayObject vao;
		QOpenGLBuffer vbo;
		QOpenGLShaderProgram* program = nullptr;
		QOpenGLShader* shader = nullptr;
		QOpenGLTexture* texture = nullptr;
	};
}
