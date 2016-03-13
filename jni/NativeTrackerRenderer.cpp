#include <_VisageTracker.h>
#include <GLES2/gl2.h>
#include <NativeTrackerRenderer.h>

void NativeTrackerRenderer::onSurfaceCreated()
{
	glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
}

void NativeTrackerRenderer::onSurfaceChanged(int w, int h)
{

}

void NativeTrackerRenderer::onDrawFrame()
{
	glClear(GL_COLOR_BUFFER_BIT);
}



