#pragma once

@interface View : UIView
{
	GLuint renderBuffer;
	GLuint frameBuffer;
	EAGLContext* context;
}

@end