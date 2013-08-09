#ifdef __APPLE__
	#include <OpenGL/gl3.h>

	#define GL_BGRA_EXT GL_BGRA
	#define BACON_PLATFORM_OPENGL 1
#else
	#define GL_GLEXT_PROTOTYPES
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>

	#define BACON_PLATFORM_ANGLE 1
#endif

#include <FreeImage/FreeImage.h>
#include <vmmlib/vmmlib.hpp>

#if BACON_PLATFORM_OPENGL
#include <GLSLANG/ShaderLang.h>
static ShHandle s_VertexCompiler;
static ShHandle s_FragmentCompiler;
#endif

#include "Bacon.h"
#include "BaconInternal.h"
#include "HandleArray.h"
using namespace Bacon;

#include <cassert>
#include <vector>
#include <string>
using namespace std;

using vmml::vec2f;
using vmml::vec3f;
using vmml::vec4f;
using vmml::mat4f;
using vmml::frustumf;

namespace {
	
    const char* const VertexAttribPosition = "a_Position";
    const char* const VertexAttribTexCoord0 = "a_TexCoord0";
    const char* const VertexAttribColor = "a_Color";

    const char* const UniformProjection = "g_Projection";
    const char* const UniformTexture0 = "g_Texture0";

	const int BoundVertexAttribPosition = 0;
	const int BoundVertexAttribTexCoord0 = 1;
	const int BoundVertexAttribColor = 2;

	struct Vertex
	{
        Vertex(vec3f const& position, vec2f const& texCoord, vec4f const& color)
            : m_Position(position)
            , m_TexCoord0(texCoord)
            , m_Color(color)
        { }

		vec3f m_Position;
		vec2f m_TexCoord0;
		vec4f m_Color;
	};

	struct Image
	{
		FIBITMAP* m_Bitmap;
		int m_Flags;
		int m_Width;
		int m_Height;
		GLuint m_Texture;
		GLuint m_FrameBuffer;
	};
	
	struct Shader
	{
		string m_VertexSource;
		string m_FragmentSource;
		
		bool m_HasError;
		GLuint m_Program;
		GLuint m_UniformProjection;
		GLuint m_UniformTexture0;
	};
		
	struct Impl
	{
        GLuint m_VBO;
        GLuint m_IBO;
        GLuint m_VAO;
		
		int m_FrameBufferWidth;
		int m_FrameBufferHeight;
		
		HandleArray<Shader> m_Shaders;
		int m_DefaultShader;

		HandleArray<Image> m_Images;
		int m_BlankImage;
		vector<GLuint> m_PendingDeleteTextures;
		vector<GLuint> m_PendingDeleteFrameBuffers;
		
		vector<Vertex> m_Vertices;
		vector<unsigned short> m_Indices;
				
		bool m_IsInFrame;
		GLuint m_CurrentMode;
		float m_CurrentZ;
		int m_CurrentShader;
		int m_CurrentImage;
		int m_CurrentFrameBuffer;
		
		mat4f m_Projection;
		
		vector<mat4f> m_TransformStack;
		vector<vec4f> m_ColorStack;
	};
	static Impl* s_Impl = nullptr;
	
}

#define REQUIRE_GL() \
	if (!s_Impl->m_IsInFrame) \
		return Bacon_Error_NotRendering;

void Graphics_Init()
{
	s_Impl = new Impl;
    s_Impl->m_VBO = 0;
    s_Impl->m_IBO = 0;
    s_Impl->m_VAO = 0;
	s_Impl->m_Images.Reserve(256);
	s_Impl->m_Shaders.Reserve(16);
	s_Impl->m_Vertices.reserve(8096);
	s_Impl->m_Indices.reserve(32768);
	s_Impl->m_IsInFrame = false;
	s_Impl->m_CurrentZ = 0.f;
	s_Impl->m_CurrentImage = -1;
	s_Impl->m_CurrentFrameBuffer = -1;
	s_Impl->m_CurrentShader = -1;
	s_Impl->m_CurrentMode = GL_TRIANGLES;
	s_Impl->m_ColorStack.push_back(vec4f::ONE);
	s_Impl->m_TransformStack.push_back(mat4f::IDENTITY);
	
	Bacon_CreateImage(&s_Impl->m_BlankImage, 1, 1);
	
	FreeImage_Initialise(TRUE);
	
#if BACON_PLATFORM_OPENGL
	ShInitialize();
	ShBuiltInResources resources;
	ShInitBuiltInResources(&resources);
	s_VertexCompiler = ShConstructCompiler(SH_VERTEX_SHADER, SH_GLES2_SPEC, SH_GLSL_OUTPUT, &resources);
	s_FragmentCompiler = ShConstructCompiler(SH_FRAGMENT_SHADER, SH_GLES2_SPEC, SH_GLSL_OUTPUT, &resources);
#endif
}

void Graphics_Shutdown()
{
#if BACON_PLATFORM_OPENGL
	ShDestruct(s_VertexCompiler);
	ShDestruct(s_FragmentCompiler);
	ShFinalize();
#endif
	
	FreeImage_DeInitialise();
	delete s_Impl;
}

void Graphics_InitGL()
{
	// Constant state
	glDisable(GL_CULL_FACE);

	// Vertex Buffer Object
	glGenBuffers(1, &s_Impl->m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, s_Impl->m_VBO);
	
	// Index Buffer Object
	glGenBuffers(1, &s_Impl->m_IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_Impl->m_IBO);
	
	// Vertex Array Object
#if __APPLE__
	glGenVertexArrays(1, &s_Impl->m_VAO);
	glBindVertexArray(s_Impl->m_VAO);
#endif
    glEnableVertexAttribArray(BoundVertexAttribPosition);
	glVertexAttribPointer(BoundVertexAttribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(BoundVertexAttribTexCoord0);
	glVertexAttribPointer(BoundVertexAttribTexCoord0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_TexCoord0));
	glEnableVertexAttribArray(BoundVertexAttribColor);
	glVertexAttribPointer(BoundVertexAttribColor, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Color));
	
	// Default shader
	Bacon_CreateShader(&s_Impl->m_DefaultShader,
		 
		 // Vertex shader
		 "precision highp float;\n"
         "attribute vec3 a_Position;\n"
		 "attribute vec2 a_TexCoord0;\n"
         "attribute vec4 a_Color;\n"
		 
		 "varying vec2 v_TexCoord0;\n"
		 "varying vec4 v_Color;\n"
		 
		 "uniform mat4 g_Projection;\n"
		 
		 "void main()\n"
		 "{\n"
		 "    gl_Position = g_Projection * vec4(a_Position, 1.0);\n"
		 "    v_TexCoord0 = a_TexCoord0;\n"
		 "    v_Color = a_Color;\n"
		 "}\n",
		 
		 // Fragment shader
         "precision mediump float;\n"
		 "uniform sampler2D g_Texture0;\n"
		 "varying vec2 v_TexCoord0;\n"
		 "varying vec4 v_Color;\n"
		 
		 "void main()\n"
		 "{"
         "    gl_FragColor = texture2D(g_Texture0, v_TexCoord0);\n"
		 "}\n");
}

void Graphics_ShutdownGL()
{
}

void Graphics_BeginFrame(int width, int height)
{
	if (!s_Impl->m_PendingDeleteTextures.empty())
	{
		glDeleteTextures((GLsizei)s_Impl->m_PendingDeleteTextures.size(), &s_Impl->m_PendingDeleteTextures[0]);
		s_Impl->m_PendingDeleteTextures.clear();
	}

	if (!s_Impl->m_PendingDeleteFrameBuffers.empty())
	{
		glDeleteFramebuffers((GLsizei)s_Impl->m_PendingDeleteFrameBuffers.size(), &s_Impl->m_PendingDeleteFrameBuffers[0]);
		s_Impl->m_PendingDeleteFrameBuffers.clear();
	}

	s_Impl->m_IsInFrame = true;
	if (s_Impl->m_FrameBufferWidth != width ||
		s_Impl->m_FrameBufferHeight != height)
	{
		s_Impl->m_FrameBufferWidth = width;
		s_Impl->m_FrameBufferHeight = height;
		s_Impl->m_CurrentFrameBuffer = -1;
	}
	
	// Default state, reset per frame
	s_Impl->m_ColorStack.clear();
	s_Impl->m_ColorStack.push_back(vec4f::ONE);

	s_Impl->m_TransformStack.clear();
	s_Impl->m_TransformStack.push_back(mat4f::IDENTITY);
	
	Bacon_SetFrameBuffer(0);
	Bacon_SetShader(0);
	Bacon_SetBlending(Bacon_Blend_One, Bacon_Blend_OneMinusSrcAlpha);
}

void Graphics_EndFrame()
{
	Bacon_Flush();
	s_Impl->m_IsInFrame = false;
}

// Shaders

int Bacon_CreateShader(int* outHandle, const char* vertexSource, const char* fragmentSource)
{
	if (!outHandle || !vertexSource || !fragmentSource)
		return Bacon_Error_InvalidArgument;
	
	*outHandle = s_Impl->m_Shaders.Alloc();
	Shader* shader = s_Impl->m_Shaders.Get(*outHandle);
	shader->m_Program = 0;
	shader->m_UniformProjection = 0;
	shader->m_VertexSource = vertexSource;
	shader->m_FragmentSource = fragmentSource;
	
	if (!s_Impl->m_CurrentShader)
		s_Impl->m_CurrentShader = *outHandle;
	
	return Bacon_Error_None;
}

#if BACON_PLATFORM_OPENGL
static bool TranslateShader(GLuint type, const char* source, string& result)
{
	int options = SH_OBJECT_CODE;
	ShHandle compiler = (type == GL_VERTEX_SHADER) ? s_VertexCompiler : s_FragmentCompiler;
	int compiled = ShCompile(compiler, &source, 1, options);
	
	if (!compiled)
	{
		size_t logLength;
		ShGetInfo(compiler, SH_INFO_LOG_LENGTH, &logLength);
		char* log = new char[logLength];
		ShGetInfoLog(compiler, log);
		printf("%s\n", log);
		delete[] log;
		
		return false;
	}

	size_t codeLength;
	ShGetInfo(compiler, SH_OBJECT_CODE_LENGTH, &codeLength);
	result.resize(codeLength);
	ShGetObjectCode(compiler, &result[0]);
	
	printf("GLES2 Source:\n%s\n", source);
	printf("GLSL Translated Source:\n%s\n", result.c_str());
	
	return true;
}
#endif

static GLuint CompileShader(GLuint type, const char* source)
{
#if BACON_PLATFORM_OPENGL
	string translatedSource;
	if (!TranslateShader(type, source, translatedSource))
		return 0;
	source = translatedSource.c_str();
#endif
	
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);
	
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		GLint logLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
		char* log = new char[logLength];
		glGetShaderInfoLog(shader, logLength, &logLength, &log[0]);
		printf("Shader compile error:\n");
		printf("%s\n", log);
		printf("Source:\n");
		printf("%s\n", source);
		delete[] log;
		return 0;
	}
	return shader;
}

static int CompileShader(Shader* shader)
{
	shader->m_HasError = true;
	
	GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, shader->m_VertexSource.c_str());
	GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, shader->m_FragmentSource.c_str());
	if (!vertexShader || !fragmentShader)
		return Bacon_Error_ShaderCompileError;
	
	GLuint program = glCreateProgram();
	glBindAttribLocation(program, BoundVertexAttribPosition, VertexAttribPosition);
    glBindAttribLocation(program, BoundVertexAttribTexCoord0, VertexAttribTexCoord0);
    glBindAttribLocation(program, BoundVertexAttribColor, VertexAttribColor);
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	
	glLinkProgram(program);
	
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status)
	{
		GLint logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		char* log = new char[logLength];
		glGetProgramInfoLog(program, logLength, &logLength, &log[0]);
		printf("Shader link error:\n");
		printf("%s\n", log);
		printf("Vertex shader source:\n");
		printf("%s\n", shader->m_VertexSource.c_str());
		printf("Fragment shader source:\n");
		printf("%s\n", shader->m_FragmentSource.c_str());
		delete[] log;
		return Bacon_Error_ShaderLinkError;
	}
	
	shader->m_HasError = false;
	shader->m_Program = program;
	shader->m_UniformProjection = glGetUniformLocation(program, UniformProjection);
	shader->m_UniformTexture0 = glGetUniformLocation(program, UniformTexture0);
	
	return Bacon_Error_None;
}

static int BindShader(int handle)
{
	if (!handle)
		handle = s_Impl->m_DefaultShader;
	
	Shader* shader = s_Impl->m_Shaders.Get(handle);
	if (!shader)
		return Bacon_Error_InvalidHandle;
	
	if (!shader->m_Program)
		CompileShader(shader);
			
	if (shader->m_HasError)
		glUseProgram(0);					// TODO error shader
	else
		glUseProgram(shader->m_Program);
	
	glUniformMatrix4fv(shader->m_UniformProjection, 1, GL_FALSE, s_Impl->m_Projection);
	glUniform1i(shader->m_UniformTexture0, 0);
	return Bacon_Error_None;
}

// Images

int Bacon_CreateImage(int* outHandle, int width, int height)
{
	if (!outHandle || width <= 0 || height <= 0)
		return Bacon_Error_InvalidArgument;
	
	*outHandle = s_Impl->m_Images.Alloc();
	Image* image = s_Impl->m_Images.Get(*outHandle);
	image->m_Bitmap = nullptr;
	image->m_Width = width;
	image->m_Height = height;
	image->m_Texture = 0;
	image->m_FrameBuffer = 0;
	
	return Bacon_Error_None;
}

int Bacon_LoadImage(int* outHandle, const char* path, int flags)
{
	if (!outHandle || !path)
		return Bacon_Error_InvalidArgument;
	
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	fif = FreeImage_GetFileType(path, 0);
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(path);
	
	if (fif == FIF_UNKNOWN)
		return Bacon_Error_UnsupportedFormat;
	
	if (!FreeImage_FIFSupportsReading(fif))
		return Bacon_Error_UnsupportedFormat;
	
	FIBITMAP *bitmap = FreeImage_Load(fif, path, 0);
	if (!bitmap)
		return Bacon_Error_Unknown;
	
	if (flags & Bacon_ImageFlags_PremultiplyAlpha)
		FreeImage_PreMultiplyWithAlpha(bitmap);
	
	*outHandle = s_Impl->m_Images.Alloc();
	Image* image = s_Impl->m_Images.Get(*outHandle);
	image->m_Bitmap = bitmap;
	image->m_Width = (int)FreeImage_GetWidth(bitmap);
	image->m_Height = (int)FreeImage_GetHeight(bitmap);
	image->m_Texture = 0;
	image->m_FrameBuffer = 0;
	image->m_Flags = flags;
	
	return Bacon_Error_None;
}

int Bacon_UnloadImage(int handle)
{
	Image* image = s_Impl->m_Images.Get(handle);
	if (!image)
		return Bacon_Error_InvalidHandle;
	
	if (image->m_Bitmap)
		FreeImage_Unload(image->m_Bitmap);
	
	if (image->m_Texture)
		s_Impl->m_PendingDeleteTextures.push_back(image->m_Texture);
	
	if (image->m_FrameBuffer)
		s_Impl->m_PendingDeleteFrameBuffers.push_back(image->m_FrameBuffer);
	
	s_Impl->m_Images.Free(handle);
	
	return Bacon_Error_None;
}

static bool CreateImageTexture(Image* image)
{
	FIBITMAP* bitmap = image->m_Bitmap;

	bool ownsData = false;
	BYTE* data = nullptr;
	GLuint format = GL_BGRA_EXT;
	GLuint internalFormat = GL_RGBA;
	if (bitmap)
	{
		data = FreeImage_GetBits(bitmap);
		int pitch = FreeImage_GetPitch(bitmap);
		int bpp = FreeImage_GetBPP(bitmap);
#if !BACON_PLATFORM_ANGLE
        if (bpp == 24)
		{
			format = GL_BGR;
			internalFormat = GL_RGB;
		}
		else
#endif
        if (bpp == 32)
		{
			format = GL_BGRA_EXT;
			internalFormat = GL_RGBA;
		}
		else
		{
			format = GL_BGRA_EXT;
			internalFormat = GL_RGBA;

			ownsData = true;
			pitch = image->m_Width * 4;
			data = (BYTE*)malloc(image->m_Height * pitch);
			FreeImage_ConvertToRawBits(data, bitmap, pitch, 32, 0, 0, 0, FALSE);
		}
	
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}

#if BACON_PLATFORM_ANGLE
	internalFormat = format;
#endif
	
	glGenTextures(1, &image->m_Texture);
	glBindTexture(GL_TEXTURE_2D, image->m_Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image->m_Width, image->m_Height, 0, format, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	if (ownsData)
		free(data);
	
	if (image->m_Flags & Bacon_ImageFlags_DiscardBitmap)
	{
		FreeImage_Unload(image->m_Bitmap);
		image->m_Bitmap = nullptr;
	}
	
	return true;
}

static void CreateBlankImageTexture(Image* image)
{
	unsigned char data[] = { 0xff, 0xff, 0xff, 0xff };
	glGenTextures(1, &image->m_Texture);
	glBindTexture(GL_TEXTURE_2D, image->m_Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

static int BindImage(int handle)
{
	if (!handle)
		handle = s_Impl->m_BlankImage;
	
	Image* image = s_Impl->m_Images.Get(handle);
	if (!image)
		return Bacon_Error_InvalidHandle;

	if (!image->m_Texture)
	{
		if (handle == s_Impl->m_BlankImage)
			CreateBlankImageTexture(image);
		else
			CreateImageTexture(image);
	}
	
	glBindTexture(GL_TEXTURE_2D, image->m_Texture);
	assert(glGetError() == GL_NO_ERROR);
	return Bacon_Error_None;
}

static bool CreateImageFrameBuffer(Image* image)
{
	if (!image->m_Texture)
	{
		if (!CreateImageTexture(image))
			return false;
	}
	
	glGenFramebuffers(1, &image->m_FrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, image->m_FrameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, image->m_Texture, 0);
	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

static int BindFrameBuffer(int handle)
{
	if (!handle)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		Bacon_SetViewport(0, 0, s_Impl->m_FrameBufferWidth, s_Impl->m_FrameBufferHeight);
		return Bacon_Error_None;
	}
	
	Image* image = s_Impl->m_Images.Get(handle);
	if (!image)
		return Bacon_Error_InvalidHandle;
	
	if (!image->m_FrameBuffer)
	{
		if (!CreateImageFrameBuffer(image))
			return Bacon_Error_Unknown;
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, image->m_FrameBuffer);
	Bacon_SetViewport(0, 0, image->m_Width, image->m_Height);
	return Bacon_Error_None;
}

int Bacon_GetImageSize(int handle, int* width, int* height)
{
	if (!width || !height)
		return Bacon_Error_InvalidArgument;
	
	Image* image = s_Impl->m_Images.Get(handle);
	if (!image)
		return Bacon_Error_InvalidHandle;
	
	*width = image->m_Width;
	*height = image->m_Height;
	
	return Bacon_Error_None;
}

int Bacon_GetImageBitmap(int handle, FIBITMAP** bitmap)
{
	if (!bitmap)
		return Bacon_Error_InvalidArgument;

	Image* image = s_Impl->m_Images.Get(handle);
	if (!image)
		return Bacon_Error_InvalidHandle;

	*bitmap = image->m_Bitmap;
	return Bacon_Error_None;
}

int Graphics_SetImageBitmap(int handle, FIBITMAP* bitmap)
{
	Image* image = s_Impl->m_Images.Get(handle);
	if (!image)
		return Bacon_Error_InvalidHandle;

	if (image->m_Bitmap)
		FreeImage_Unload(image->m_Bitmap);
	image->m_Bitmap = bitmap;
	
	return Bacon_Error_None;
}


// 2D Transform
int Bacon_PushTransform()
{
	mat4f t = s_Impl->m_TransformStack.back();
	s_Impl->m_TransformStack.push_back(t);
	return Bacon_Error_None;
}

int Bacon_PopTransform()
{
	if (s_Impl->m_TransformStack.size() <= 1)
		return Bacon_Error_StackUnderflow;
	
	s_Impl->m_TransformStack.pop_back();
	return Bacon_Error_None;
}

int Bacon_Translate(float x, float y)
{
	mat4f translate;
	translate.translation(vec3f(x, y, 0.f));
	s_Impl->m_TransformStack.back() *= translate;
	return Bacon_Error_None;
}

int Bacon_Scale(float sx, float sy)
{
	mat4f scale;
	scale.scaling(vec3f(sx, sy, 0.f));
	s_Impl->m_TransformStack.back() *= scale;
	return Bacon_Error_None;
}

int Bacon_Rotate(float radians)
{
	mat4f rotate;
	rotate.rotation_z(radians);
	s_Impl->m_TransformStack.back() *= rotate;
	return Bacon_Error_None;
}

int Bacon_SetTransform(float* matrix)
{
	s_Impl->m_TransformStack.back().set(matrix, &matrix[16]);
	return Bacon_Error_None;
}

// Color

int Bacon_PushColor()
{
	vec4f c = s_Impl->m_ColorStack.back();
	s_Impl->m_ColorStack.push_back(c);
	return Bacon_Error_None;
}

int Bacon_PopColor()
{
	if (s_Impl->m_ColorStack.size() <= 1)
		return Bacon_Error_StackUnderflow;
	
	s_Impl->m_ColorStack.pop_back();
	return Bacon_Error_None;
}

int Bacon_SetColor(float r, float g, float b, float a)
{
	s_Impl->m_ColorStack.back() = vec4f(r, g, b, a);
	return Bacon_Error_None;
}

int Bacon_MultiplyColor(float r, float g, float b, float a)
{
	s_Impl->m_ColorStack.back() *= vec4f(r, g, b, a);
	return Bacon_Error_None;
}


// Drawing

int Bacon_SetFrameBuffer(int image)
{
	REQUIRE_GL();
	if (s_Impl->m_CurrentFrameBuffer == image)
		return Bacon_Error_None;

	Bacon_Flush();
	s_Impl->m_CurrentFrameBuffer = image;
	return BindFrameBuffer(image);
}

int Bacon_SetViewport(int x, int y, int width, int height)
{
	REQUIRE_GL();

	Bacon_Flush();

	int frameBufferHeight = s_Impl->m_FrameBufferHeight;
	if (s_Impl->m_CurrentFrameBuffer != 0)
		frameBufferHeight = s_Impl->m_Images.Get(s_Impl->m_CurrentFrameBuffer)->m_Height;
	
	glViewport(x, frameBufferHeight - (y + height), width, height);
	s_Impl->m_Projection = frustumf(0.f, (float)width, (float)height, 0.f, -1.f, 1.f).compute_ortho_matrix();
	s_Impl->m_CurrentShader = -1; // Invalidate uniform
	return Bacon_Error_None;
}

int Bacon_SetShader(int shader)
{
	REQUIRE_GL();
	if (s_Impl->m_CurrentShader == shader)
		return Bacon_Error_None;
	
	Bacon_Flush();
	s_Impl->m_CurrentShader = shader;
	return BindShader(shader);
}

static GLuint GetGLBlending(int blend)
{
	switch (blend)
	{
		case Bacon_Blend_Zero: return GL_ZERO;
		case Bacon_Blend_One: return GL_ONE;
		case Bacon_Blend_SrcColor: return GL_SRC_COLOR;
		case Bacon_Blend_OneMinusSrcColor: return GL_ONE_MINUS_SRC_COLOR;
		case Bacon_Blend_DstColor: return GL_DST_COLOR;
		case Bacon_Blend_OneMinusDstColor: return GL_ONE_MINUS_DST_COLOR;
		case Bacon_Blend_SrcAlpha: return GL_SRC_ALPHA;
		case Bacon_Blend_OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
		case Bacon_Blend_DstAlpha: return GL_DST_ALPHA;
		case Bacon_Blend_OneMinusDstAlpha: return GL_ONE_MINUS_DST_ALPHA;
		default: return 0;
	}
}

int Bacon_SetBlending(int src, int dest)
{
	REQUIRE_GL();
	Bacon_Flush();
	
	GLuint blendSrc = GetGLBlending(src);
	GLuint blendDest = GetGLBlending(dest);
	if (!blendSrc || !blendDest)
		return Bacon_Error_InvalidArgument;
	
	if (blendSrc == GL_ONE && blendDest == GL_ZERO)
		glDisable(GL_BLEND);
	else
		glEnable(GL_BLEND);
	glBlendFunc(blendSrc, blendDest);
	return Bacon_Error_None;
}

int Bacon_Clear(float r, float g, float b, float a)
{
	REQUIRE_GL();

	Bacon_Flush();
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);
#ifdef __APPLE__
	glGetError(); // Consume known error
#endif
	return Bacon_Error_None;
}

int Bacon_DrawImage(int handle, float x1, float y1, float x2, float y2)
{
	float z = s_Impl->m_CurrentZ;
	float positions[] = {
		x1, y1, z,
		x1, y2, z,
		x2, y2, z,
		x2, y1, z
	};
	float texCoords[] = {
		0.f, 1.f,
		0.f, 0.f,
		1.f, 0.f,
		1.f, 1.f
	};
	float colors[] = {
		1.f, 1.f, 1.f, 1.f,
		1.f, 1.f, 1.f, 1.f,
		1.f, 1.f, 1.f, 1.f,
		1.f, 1.f, 1.f, 1.f,
	};
	
	return Bacon_DrawImageQuad(handle, positions, texCoords, colors);
}

int Bacon_DrawImageRegion(int handle, float x1, float y1, float x2, float y2,
						 float u1, float v1, float u2, float v2)
{
	if (handle)
	{
		Image* image = s_Impl->m_Images.Get(handle);
		if (!image)
			return Bacon_Error_InvalidHandle;
		float w = (float)image->m_Width;
		float h = (float)image->m_Height;
		u1 = u1 / w;
		v1 = 1.f - v1 / h;
		u2 = u2 / h;
		v2 = 1.f - v2 / h;
	}
	
	float z = s_Impl->m_CurrentZ;
	float positions[] = {
		x1, y1, z,
		x1, y2, z,
		x2, y2, z,
		x2, y1, z
	};
	float texCoords[] = {
		u1, v1,
		u1, v2,
		u2, v2,
		u2, v1
	};
	float colors[] = {
		1.f, 1.f, 1.f, 1.f,
		1.f, 1.f, 1.f, 1.f,
		1.f, 1.f, 1.f, 1.f,
		1.f, 1.f, 1.f, 1.f,
	};
	
	return Bacon_DrawImageQuad(handle, positions, texCoords, colors);
}

inline void SetCurrentImage(int image)
{
	if (image != s_Impl->m_CurrentImage)
	{
		Bacon_Flush();
		BindImage(image);
		s_Impl->m_CurrentImage = image;
	}
}

inline void SetCurrentMode(GLuint mode)
{
	if (mode != s_Impl->m_CurrentMode)
	{
		Bacon_Flush();
		s_Impl->m_CurrentMode = mode;
	}
}

int Bacon_DrawImageQuad(int image, float* positions, float* texCoords, float* colors)
{
	REQUIRE_GL();

	SetCurrentImage(image);
	SetCurrentMode(GL_TRIANGLES);
	
	mat4f const& transform = s_Impl->m_TransformStack.back();
	vec4f const& color = s_Impl->m_ColorStack.back();
	vector<Vertex>& vertices = s_Impl->m_Vertices;
	unsigned short index = vertices.size();
	for (int i = 0; i < 4; ++i)
		vertices.push_back(Vertex(
			transform * vec3f(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]),
			vec2f(texCoords[i * 2], texCoords[i * 2 + 1]),
			color * vec4f(colors[i * 4], colors[i * 4 + 1], colors[i * 4 + 2], colors[i * 4 + 3])
		));
	
	vector<unsigned short>& indices = s_Impl->m_Indices;
	indices.push_back(index + 0);
	indices.push_back(index + 1);
	indices.push_back(index + 2);
	indices.push_back(index + 0);
	indices.push_back(index + 2);
	indices.push_back(index + 3);
	
	return Bacon_Error_None;
}

int Bacon_DrawLine(float x1, float y1, float x2, float y2)
{
	REQUIRE_GL();
	
	SetCurrentImage(0);
	SetCurrentMode(GL_LINES);

	float z = s_Impl->m_CurrentZ;
	mat4f const& transform = s_Impl->m_TransformStack.back();
	vec4f const& color = s_Impl->m_ColorStack.back();
	vector<Vertex>& vertices = s_Impl->m_Vertices;
	unsigned short index = vertices.size();
	vertices.push_back(Vertex(transform * vec3f(x1, y1, z), vec2f(0, 0), color));
	vertices.push_back(Vertex(transform * vec3f(x2, y2, z), vec2f(1, 0), color));

	vector<unsigned short>& indices = s_Impl->m_Indices;
	indices.push_back(index + 0);
	indices.push_back(index + 1);
	
	return Bacon_Error_None;
}

int Bacon_Flush()
{
	REQUIRE_GL();

	if (s_Impl->m_Indices.empty())
		return Bacon_Error_None;
	
	glBindBuffer(GL_ARRAY_BUFFER, s_Impl->m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * s_Impl->m_Vertices.size(), &s_Impl->m_Vertices[0], GL_DYNAMIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_Impl->m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * s_Impl->m_Indices.size(), &s_Impl->m_Indices[0], GL_DYNAMIC_DRAW);
	
	glDrawElements(s_Impl->m_CurrentMode, (int)s_Impl->m_Indices.size(), GL_UNSIGNED_SHORT, 0);
	
	s_Impl->m_Indices.clear();
	s_Impl->m_Vertices.clear();
	
	return Bacon_Error_None;
}
