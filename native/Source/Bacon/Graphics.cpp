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
#include <GLSLANG/ShaderLang.h>

#include "Bacon.h"
#include "BaconInternal.h"
#include "HandleArray.h"
#include "Rect.h"
#include "MaxRectsAllocator.h"
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

	const int TextureAtlasMargin = 2;
	const int TextureAtlasMinSize = 128;

	enum Bacon_ImageFlags_Internal
	{
		// Image.m_Texture actually refers to another image.  Used for image regions of images
		// that have not had their texture realized yet.
		Bacon_ImageFlags_Internal_TextureIsImage = 1 << 16,
	};

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

	struct UVScaleBias
	{
		UVScaleBias()
		: m_ScaleX(1.f)
		, m_ScaleY(1.f)
		, m_BiasX(0.f)
		, m_BiasY(0.f)
		{ }
		
		UVScaleBias(float scaleX, float scaleY, float biasX, float biasY)
		: m_ScaleX(scaleX)
		, m_ScaleY(scaleY)
		, m_BiasX(biasX)
		, m_BiasY(biasY)
		{ }
		
		float m_ScaleX;
		float m_ScaleY;
		float m_BiasX;
		float m_BiasY;
		
		vec2f Apply(vec2f const& v) const
		{
			return vec2f(m_BiasX + v.x() * m_ScaleX,
						 m_BiasY + v.y() * m_ScaleY);
		}
	};
	
	struct Texture
	{
		Texture()
		: m_RefCount(0)
		{ }

		int m_RefCount;
		int m_Width, m_Height;
		GLuint m_TextureId;
		GLuint m_FrameBuffer;
	};
	
	struct TextureAtlas
	{
		TextureAtlas()
		: m_RefCount(0)
		{ }
		
		int m_RefCount;
		MaxRectsAllocator m_Allocator;
		FIBITMAP* m_Bitmap;
		int m_Texture;
		int m_Flags;
		int m_Width;
		int m_Height;
	};
	
	struct Image
	{
		// In-memory copy of the image; i.e. loaded from disk or rendered by FreeType
		// May be null, representing a possible render target
		FIBITMAP* m_Bitmap;
		
		// Handle to s_Impl->m_Textures, represents GPU resource which may be shared with other images
		// Zero until the image is realized on GPU
		int m_Texture;
		
		// Handle to s_Impl->m_TextureAtlases; zero if the image does not belong to an atlas
		int m_Atlas;
		
		// Image-specific attributes
		int m_Flags;
		int m_Width;
		int m_Height;
		
		// Scale/bias to apply to texture coordinates to map from image to texture
		UVScaleBias m_UVScaleBias;
	};
	
	struct ShaderUniform
	{
		ShaderUniform()
		: m_Id(-1)
		, m_ValueVersion(-1)
		, m_SharedUniformIndex(-1)
		{ }
		
		ShaderUniform(const char* name, ShDataType type, int arrayCount)
		: m_Id(-1)
		, m_Name(name)
		, m_Type(type)
		, m_ArrayCount(arrayCount)
		, m_SharedUniformIndex(-1)
		{ }
		
		// OpenGL-ES name of the uniform, populated when the shader is linked; -1 if the
		// uniform was not linked (e.g., optimized out by the compiler)
		GLint m_Id;
		
		// For sampler uniforms, the texture unit assigned to this uniform
		GLuint m_TextureUnit;
		
		// Name of the uniform, excluding any array suffixes (e.g., name of g_Textures[4] is
		// "g_Textures")
		string m_Name;

		// Type and array count of the uniform; this dictates the size and format of m_Value.
		ShDataType m_Type;
		int m_ArrayCount;
		
		// Shader uniform data can be shared between all shaders (e.g. projection matrix, time, ...)
		// in which case m_SharedUniformIndex is an index into s_Impl->m_SharedUniforms.
		//
		// Other uniforms are specific to the shader (e.g., a variable for a post-effect), and
		// m_SharedUniformIndex is -1 and the value is stored in m_Value.
		int m_SharedUniformIndex;
		
		// For shared uniforms, the version matches if this shader has the shared value up-to-date;
		// for non-shared unifors, m_ValueVersion is 0 if the value is unchanged, non-zero if
		// changed.
		int m_ValueVersion;
		
		// For non-shared uniforms only (or the shared entry in s_Impl->m_SharedUniforms), the next
		// value to set with glUniform.
		vector<char> m_Value;
	};
	
	struct Shader
	{
		string m_VertexSource;
		string m_FragmentSource;

		vector<ShaderUniform> m_Uniforms;
		vector<int> m_TextureUnits;

		GLuint m_Program;

		int m_SharedUniformsVersion;
		bool m_NonSharedValuesDirty;
		bool m_HasError;
	};
		
	struct Impl
	{
        GLuint m_VBO;
        GLuint m_IBO;
		
		int m_FrameBufferWidth;
		int m_FrameBufferHeight;
		
		HandleArray<Shader> m_Shaders;
		int m_DefaultShader;

		HandleArray<Texture> m_Textures;
		HandleArray<TextureAtlas> m_TextureAtlases;
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
		int m_CurrentFrameBuffer;
		int m_CurrentTextureUnits[16];
		
		int m_SharedUniformsVersion;
		vector<ShaderUniform> m_SharedUniforms;
		
		// Built-in shared uniforms
		int m_ProjectionUniform;
		int m_Texture0Uniform;
		
		vector<mat4f> m_TransformStack;
		vector<vec4f> m_ColorStack;
	};
	static Impl* s_Impl = nullptr;
	
    static ShHandle s_VertexCompiler;
    static ShHandle s_FragmentCompiler;

}

#define REQUIRE_GL() \
	if (!s_Impl->m_IsInFrame) \
		return Bacon_Error_NotRendering;

static int CreateSharedUniform(ShaderUniform const& uniform);

void Graphics_Init()
{
	s_Impl = new Impl;
    s_Impl->m_VBO = 0;
    s_Impl->m_IBO = 0;
	s_Impl->m_Images.Reserve(256);
	s_Impl->m_TextureAtlases.Reserve(32);
	s_Impl->m_Textures.Reserve(256);
	s_Impl->m_Shaders.Reserve(16);
	s_Impl->m_Vertices.reserve(8096);
	s_Impl->m_Indices.reserve(32768);
	s_Impl->m_IsInFrame = false;
	s_Impl->m_CurrentZ = 0.f;
	for (int i = 0; i < BACON_ARRAY_COUNT(s_Impl->m_CurrentTextureUnits); ++i)
		s_Impl->m_CurrentTextureUnits[i] = -1;
	s_Impl->m_CurrentFrameBuffer = -1;
	s_Impl->m_CurrentShader = -1;
	s_Impl->m_CurrentMode = GL_TRIANGLES;
	s_Impl->m_ColorStack.push_back(vec4f::ONE);
	s_Impl->m_TransformStack.push_back(mat4f::IDENTITY);
	s_Impl->m_SharedUniformsVersion = 0;
	
	Bacon_CreateImage(&s_Impl->m_BlankImage, 1, 1, Bacon_ImageFlags_DiscardBitmap | Bacon_ImageFlags_Atlas);
	FIBITMAP* blankBitmap = FreeImage_Allocate(1, 1, 32);
	memset(FreeImage_GetBits(blankBitmap), 255, 4);
	Graphics_SetImageBitmap(s_Impl->m_BlankImage, blankBitmap);
	
	FreeImage_Initialise(TRUE);
	
	ShInitialize();
	ShBuiltInResources resources;
	ShInitBuiltInResources(&resources);
	resources.FragmentPrecisionHigh = 1;
#if BACON_PLATFORM_OPENGL
	ShShaderOutput output = SH_GLSL_OUTPUT;
#elif WIN32
	ShShaderOutput output = SH_HLSL9_OUTPUT;
#endif
	s_VertexCompiler = ShConstructCompiler(SH_VERTEX_SHADER, SH_GLES2_SPEC, output, &resources);
	s_FragmentCompiler = ShConstructCompiler(SH_FRAGMENT_SHADER, SH_GLES2_SPEC, output, &resources);
	
	// Built-in shared uniforms
	s_Impl->m_ProjectionUniform = CreateSharedUniform(ShaderUniform("g_Projection", SH_FLOAT_MAT4, 1));
	s_Impl->m_Texture0Uniform = CreateSharedUniform(ShaderUniform("g_Texture0", SH_SAMPLER_2D, 1));
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
    Bacon_Log(Bacon_LogLevel_Info, "GL_VENDOR: %s", glGetString(GL_VENDOR));
    Bacon_Log(Bacon_LogLevel_Info, "GL_RENDERER: %s", glGetString(GL_RENDERER));
    Bacon_Log(Bacon_LogLevel_Info, "GL_SHADING_LANGUAGE_VERSION: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    Bacon_Log(Bacon_LogLevel_Info, "GL_EXTENSIONS: %s", glGetString(GL_EXTENSIONS)); // TODO this is being truncated by logging

	// Constant state
	glDisable(GL_CULL_FACE);

	// Vertex Buffer Object
	glGenBuffers(1, &s_Impl->m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, s_Impl->m_VBO);
	
	// Index Buffer Object
	glGenBuffers(1, &s_Impl->m_IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_Impl->m_IBO);
	
	// Vertex Array Object
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
         "precision highp float;\n"
		 "uniform sampler2D g_Texture0;\n"
		 "varying vec2 v_TexCoord0;\n"
		 "varying vec4 v_Color;\n"
		 
		 "void main()\n"
		 "{"
         "    gl_FragColor = v_Color * texture2D(g_Texture0, v_TexCoord0);\n"
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

static int GetShaderTypeSize(int type)
{
	int size = 0;
	switch (type)
	{
		case SH_NONE:
			size = 0;
			break;
		case SH_BOOL:
		case SH_INT:
		case SH_FLOAT:
			size = 4;
			break;
		case SH_BOOL_VEC2:
		case SH_INT_VEC2:
		case SH_FLOAT_VEC2:
			size = 4 * 2;
			break;
		case SH_BOOL_VEC3:
		case SH_INT_VEC3:
		case SH_FLOAT_VEC3:
			size = 4 * 3;
			break;
		case SH_BOOL_VEC4:
		case SH_INT_VEC4:
		case SH_FLOAT_VEC4:
			size = 4 * 4;
			break;
		case SH_SAMPLER_2D:
		case SH_SAMPLER_2D_RECT_ARB:
		case SH_SAMPLER_CUBE:
		case SH_SAMPLER_EXTERNAL_OES:
			size = 4;
			break;
		case SH_FLOAT_MAT2:
			size = 4 * 2 * 2;
			break;
		case SH_FLOAT_MAT3:
			size = 4 * 3 * 3;
			break;
		case SH_FLOAT_MAT4:
			size = 4 * 4 * 4;
			break;
	}
	return size;
}

static int GetShaderUniformSize(ShaderUniform const& uniform)
{
	return GetShaderTypeSize(uniform.m_Type) * uniform.m_ArrayCount;
}

static int GetSharedUniform(const char* name)
{
	for (int i = 0; i < s_Impl->m_SharedUniforms.size(); ++i)
	{
		if (s_Impl->m_SharedUniforms[i].m_Name == name)
			return i;
	}
	return -1;
}

static int CreateSharedUniform(ShaderUniform const& uniform)
{
	assert(GetSharedUniform(uniform.m_Name.c_str()) == -1);
	s_Impl->m_SharedUniforms.push_back(uniform);
	s_Impl->m_SharedUniforms.back().m_ValueVersion = 0;
	s_Impl->m_SharedUniforms.back().m_Value.resize(GetShaderUniformSize(uniform));
	return (int)s_Impl->m_SharedUniforms.size() - 1;
}

static void SetSharedUniformValue(int sharedUniformIndex, int value)
{
	ShaderUniform& shared = s_Impl->m_SharedUniforms[sharedUniformIndex];
	int& sharedValue = (int&)shared.m_Value[0];
	assert(shared.m_Value.size() == 4);
	
	if (sharedValue == value)
		return;
	
	Bacon_Flush();
	sharedValue = value;
	++shared.m_ValueVersion;
	++s_Impl->m_SharedUniformsVersion;
}

static void SetSharedUniformValue(int sharedUniformIndex, const void* value, size_t size)
{
	ShaderUniform& shared = s_Impl->m_SharedUniforms[sharedUniformIndex];
	assert(shared.m_Value.size() == size);
	if (memcmp(&shared.m_Value[0], value, size) == 0)
		return;
	
	Bacon_Flush();
	memcpy(&shared.m_Value[0], value, size);
	++shared.m_ValueVersion;
	++s_Impl->m_SharedUniformsVersion;
}

int Bacon_CreateSharedShaderUniform(int* outHandle, const char* name, int type, int arrayCount)
{
	if (!outHandle || GetShaderTypeSize(type) == 0 || arrayCount < 1)
		return Bacon_Error_InvalidArgument;
	
	*outHandle = CreateSharedUniform(ShaderUniform(name, (ShDataType)type, arrayCount));
	return Bacon_Error_None;
}

int Bacon_SetSharedShaderUniform(int handle, const void* value, int size)
{
	if (handle < 0 || handle >= s_Impl->m_SharedUniforms.size())
		return Bacon_Error_InvalidHandle;
	
	if (s_Impl->m_SharedUniforms[handle].m_Value.size() != size)
		return Bacon_Error_InvalidArgument;
	
	SetSharedUniformValue(handle, value, size);
	return Bacon_Error_None;
}

static bool TranslateShader(Shader* shader, GLuint type, string& source)
{
	int options = SH_ATTRIBUTES_UNIFORMS;
	
#if BACON_PLATFORM_OPENGL
	options |= SH_OBJECT_CODE;
#endif
	
	ShHandle compiler = (type == GL_VERTEX_SHADER) ? s_VertexCompiler : s_FragmentCompiler;
	const char* sourcePtr = source.c_str();
	int compiled = ShCompile(compiler, &sourcePtr, 1, options);
	
	if (!compiled)
	{
		size_t logLength;
		ShGetInfo(compiler, SH_INFO_LOG_LENGTH, &logLength);
		char* log = new char[logLength];
		ShGetInfoLog(compiler, log);
        Bacon_Log(Bacon_LogLevel_Error, "Shader translate error: %s", log);
		delete[] log;
		
		return false;
	}
	
#if BACON_PLATFORM_OPENGL
    Bacon_Log(Bacon_LogLevel_Trace, "GLES2 Source:\n%s", source.c_str());

	size_t codeLength;
	ShGetInfo(compiler, SH_OBJECT_CODE_LENGTH, &codeLength);
	source.resize(codeLength);
	ShGetObjectCode(compiler, &source[0]);
	
    Bacon_Log(Bacon_LogLevel_Trace, "GLSL Translated Source:\n%s", source.c_str());
#endif
	
	size_t uniformNameSize;
	ShGetInfo(compiler, SH_ACTIVE_UNIFORM_MAX_LENGTH, &uniformNameSize);
	
	size_t activeUniforms;
	ShGetInfo(compiler, SH_ACTIVE_UNIFORMS, &activeUniforms);
	shader->m_Uniforms.reserve(shader->m_Uniforms.size() + activeUniforms);
	for (size_t i = 0; i < activeUniforms; ++i)
	{
		shader->m_Uniforms.push_back(ShaderUniform());
		ShaderUniform& shaderUniform = shader->m_Uniforms.back();
		shaderUniform.m_Name.resize(uniformNameSize);
		size_t nameLength;
		ShGetActiveUniform(compiler, (int)i, &nameLength, &shaderUniform.m_ArrayCount, &shaderUniform.m_Type, &shaderUniform.m_Name[0], nullptr);
		shaderUniform.m_Name.resize(nameLength);
		
		// Uniform arrays are named, e.g. "textures[0]", strip the suffix to make it "textures"
		size_t bracketPos = shaderUniform.m_Name.find('[');
		if (bracketPos != string::npos)
			shaderUniform.m_Name.resize(bracketPos);
		
		if (shaderUniform.m_Name.size() > 2 &&
			shaderUniform.m_Name[0] == 'g' &&
			shaderUniform.m_Name[1] == '_')
		{
			// This is a shared uniform, link it up
			shaderUniform.m_SharedUniformIndex = GetSharedUniform(shaderUniform.m_Name.c_str());
			if (shaderUniform.m_SharedUniformIndex == -1)
			{
				// Create new shared uniform
				shaderUniform.m_SharedUniformIndex = CreateSharedUniform(shaderUniform);
				shaderUniform.m_ValueVersion = -1;
			}
			else
			{
				// Ensure types match
				ShaderUniform& shared = s_Impl->m_SharedUniforms[shaderUniform.m_SharedUniformIndex];
				if (shared.m_ArrayCount != shaderUniform.m_ArrayCount ||
					shared.m_Type != shaderUniform.m_Type)
				{
					Bacon_Log(Bacon_LogLevel_Error, "Shared uniform \"%s\" has the wrong type.  All shared uniforms with the same name must have the same type and array size.", shaderUniform.m_Name.size());
					return false;
				}
			}
		}
		else
		{
			// Non-shared uniform
			shaderUniform.m_SharedUniformIndex = -1;
			shaderUniform.m_ValueVersion = 1;
			shaderUniform.m_Value.resize(GetShaderUniformSize(shaderUniform));
		}
	}
	
	return true;
}

int Bacon_CreateShader(int* outHandle, const char* vertexSource, const char* fragmentSource)
{
	if (!outHandle || !vertexSource || !fragmentSource)
		return Bacon_Error_InvalidArgument;
	
	*outHandle = s_Impl->m_Shaders.Alloc();
	Shader* shader = s_Impl->m_Shaders.Get(*outHandle);
	shader->m_Program = 0;
	shader->m_VertexSource = vertexSource;
	shader->m_FragmentSource = fragmentSource;
	shader->m_SharedUniformsVersion = -1;
	shader->m_NonSharedValuesDirty = true;
	
	if (!TranslateShader(shader, GL_VERTEX_SHADER, shader->m_VertexSource))
	{
		s_Impl->m_Shaders.Free(*outHandle);
		return Bacon_Error_ShaderCompileError;
	}
	
	if (!TranslateShader(shader, GL_FRAGMENT_SHADER, shader->m_FragmentSource))
	{
		s_Impl->m_Shaders.Free(*outHandle);
		return Bacon_Error_ShaderCompileError;
	}

	// Assign texture units to sampler uniforms
	int textureUnit = 0;
	for (auto& uniform : shader->m_Uniforms)
	{
		if (uniform.m_Type == SH_SAMPLER_2D)
		{
			uniform.m_TextureUnit = textureUnit;
			for (int i = 0; i < uniform.m_ArrayCount; ++i)
			{
				textureUnit++;
				shader->m_TextureUnits.push_back(0);
			}
		}
		else
		{
			uniform.m_TextureUnit = -1;
		}
	}

	if (!s_Impl->m_CurrentShader)
		s_Impl->m_CurrentShader = *outHandle;
	
	return Bacon_Error_None;
}

int Bacon_EnumShaderUniforms(int handle, Bacon_EnumShaderUniformsCallback callback, void* arg)
{
	Shader* shader = s_Impl->m_Shaders.Get(handle);
	if (!shader)
		return Bacon_Error_InvalidHandle;
	
	for (int i = 0; i < shader->m_Uniforms.size(); ++i)
	{
		ShaderUniform& uniform = shader->m_Uniforms[i];
		callback(handle, i, uniform.m_Name.c_str(), uniform.m_Type, uniform.m_ArrayCount, arg);
	}
	
	return Bacon_Error_None;
}

int Bacon_SetShaderUniform(int handle, int uniform, const void* value, int size)
{
	if (handle == s_Impl->m_CurrentShader)
		Bacon_Flush();
	
	Shader* shader = s_Impl->m_Shaders.Get(handle);
	if (!shader)
		return Bacon_Error_InvalidHandle;

	if (uniform < 0 || uniform >= shader->m_Uniforms.size())
		return Bacon_Error_InvalidHandle;
	
	ShaderUniform& shaderUniform = shader->m_Uniforms[uniform];
	
	if (shaderUniform.m_SharedUniformIndex != -1)
	{
		// Set shared value
		return Bacon_SetSharedShaderUniform(shaderUniform.m_SharedUniformIndex, value, size);
	}
	else
	{
		// Set non-shared value
		if (shaderUniform.m_Value.size() != size)
			return Bacon_Error_InvalidArgument;

		if (shaderUniform.m_TextureUnit != -1)
		{
			memcpy(&shader->m_TextureUnits[shaderUniform.m_TextureUnit], value, size);
		}
		else
		{
			memcpy(&shaderUniform.m_Value[0], value, size);
			shaderUniform.m_ValueVersion = 1;
			shader->m_NonSharedValuesDirty = true;
		}
	}
	return Bacon_Error_None;
}


static GLuint CompileShader(GLuint type, const char* source)
{
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
        Bacon_Log(Bacon_LogLevel_Error, "Shader compile error:\n%s", log);
        Bacon_Log(Bacon_LogLevel_Error, "Shader source was:\n%s", source);
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
		Bacon_Log(Bacon_LogLevel_Error, "Shader link error:\n%s", log);
		Bacon_Log(Bacon_LogLevel_Error, "Vertex shader source:\n%s", shader->m_VertexSource.c_str());
		Bacon_Log(Bacon_LogLevel_Error, "Fragment shader source:\n%s", shader->m_FragmentSource.c_str());
		delete[] log;
		return Bacon_Error_ShaderLinkError;
	}
	
	shader->m_HasError = false;
	shader->m_Program = program;
	
	glUseProgram(program);
	for (auto& uniform : shader->m_Uniforms)
	{
		uniform.m_Id = glGetUniformLocation(program, uniform.m_Name.c_str());
		if (uniform.m_Id < 0)
			Bacon_Log(Bacon_LogLevel_Warning, "Shader uniform \"%s\" is not used", uniform.m_Name.c_str());
		if (uniform.m_TextureUnit != -1)
		{
			if (uniform.m_ArrayCount == 1)
				glUniform1i(uniform.m_Id, uniform.m_TextureUnit);
			else
			{
				vector<int> textureUnits;
				for (int i = 0; i < uniform.m_ArrayCount; ++i)
					textureUnits.push_back(uniform.m_TextureUnit +i);
				glUniform1iv(uniform.m_Id, uniform.m_ArrayCount, &textureUnits[0]);
			}
		}
	}
	
	return Bacon_Error_None;
}

static int BindShader(int handle)
{
	Shader* shader = s_Impl->m_Shaders.Get(handle);
	if (!shader)
		return Bacon_Error_InvalidHandle;
	
	if (!shader->m_Program)
		CompileShader(shader);
			
	if (shader->m_HasError)
		glUseProgram(0);					// TODO error shader
	else
		glUseProgram(shader->m_Program);
	
	return Bacon_Error_None;
}

static int BindTexture(int handle);

static void BindShaderUniforms()
{
	Shader* shader = s_Impl->m_Shaders.Get(s_Impl->m_CurrentShader);
	
	// Early-out if up-to-date
	if (shader->m_SharedUniformsVersion == s_Impl->m_SharedUniformsVersion &&
		!shader->m_NonSharedValuesDirty)
		return;
	
	shader->m_SharedUniformsVersion = s_Impl->m_SharedUniformsVersion;
	shader->m_NonSharedValuesDirty = false;
	
	for (auto& uniform : shader->m_Uniforms)
	{
		const void* value;
		if (uniform.m_SharedUniformIndex == -1)
		{
			// Non-shared
			if (!uniform.m_ValueVersion)
				continue;
			uniform.m_ValueVersion = 0;
			value = &uniform.m_Value[0];
		}
		else
		{
			// Shared
			ShaderUniform& shared = s_Impl->m_SharedUniforms[uniform.m_SharedUniformIndex];
			if (uniform.m_ValueVersion == shared.m_ValueVersion)
				continue;
			uniform.m_ValueVersion = shared.m_ValueVersion;
			value = &shared.m_Value[0];
			
			// Copy shared sampler value into shader's texture unit array
			if (uniform.m_TextureUnit != -1)
				memcpy(&shader->m_TextureUnits[uniform.m_TextureUnit], &shared.m_Value[0], shared.m_Value.size());
		}
		
		switch (uniform.m_Type)
		{
			case SH_NONE:
				break;
			case SH_BOOL:
			case SH_INT:
				glUniform1iv(uniform.m_Id, uniform.m_ArrayCount, (GLint*)value);
				break;
			case SH_BOOL_VEC2:
			case SH_INT_VEC2:
				glUniform2iv(uniform.m_Id, uniform.m_ArrayCount, (GLint*)value);
				break;
			case SH_BOOL_VEC3:
			case SH_INT_VEC3:
				glUniform3iv(uniform.m_Id, uniform.m_ArrayCount, (GLint*)value);
				break;
			case SH_BOOL_VEC4:
			case SH_INT_VEC4:
				glUniform4iv(uniform.m_Id, uniform.m_ArrayCount, (GLint*)value);
				break;
			case SH_FLOAT:
				glUniform1fv(uniform.m_Id, uniform.m_ArrayCount, (GLfloat*)value);
				break;
			case SH_FLOAT_VEC2:
				glUniform2fv(uniform.m_Id, uniform.m_ArrayCount, (GLfloat*)value);
				break;
			case SH_FLOAT_VEC3:
				glUniform3fv(uniform.m_Id, uniform.m_ArrayCount, (GLfloat*)value);
				break;
			case SH_FLOAT_VEC4:
				glUniform4fv(uniform.m_Id, uniform.m_ArrayCount, (GLfloat*)value);
				break;
			case SH_FLOAT_MAT2:
				glUniformMatrix2fv(uniform.m_Id, uniform.m_ArrayCount, GL_FALSE, (GLfloat*)value);
				break;
			case SH_FLOAT_MAT3:
				glUniformMatrix3fv(uniform.m_Id, uniform.m_ArrayCount, GL_FALSE, (GLfloat*)value);
				break;
			case SH_FLOAT_MAT4:
				glUniformMatrix4fv(uniform.m_Id, uniform.m_ArrayCount, GL_FALSE, (GLfloat*)value);
				break;
			case SH_SAMPLER_2D:
			case SH_SAMPLER_2D_RECT_ARB:
			case SH_SAMPLER_CUBE:
			case SH_SAMPLER_EXTERNAL_OES:
				break;
		}
	}
}

static void BindShaderTextureUnits()
{
	Shader* shader = s_Impl->m_Shaders.Get(s_Impl->m_CurrentShader);

	for (int i = 0; i < shader->m_TextureUnits.size(); ++i)
	{
		if (s_Impl->m_CurrentTextureUnits[i] != shader->m_TextureUnits[i])
		{
			glActiveTexture(GL_TEXTURE0 + i);
			BindTexture(shader->m_TextureUnits[i]);
			s_Impl->m_CurrentTextureUnits[i] = shader->m_TextureUnits[i];
		}
	}
}

// Images

int Bacon_CreateImage(int* outHandle, int width, int height, int flags)
{
	if (!outHandle || width <= 0 || height <= 0)
		return Bacon_Error_InvalidArgument;
	
	*outHandle = s_Impl->m_Images.Alloc();
	Image* image = s_Impl->m_Images.Get(*outHandle);
	image->m_Texture = 0;
	image->m_Atlas = 0;
	image->m_Bitmap = nullptr;
	image->m_Width = width;
	image->m_Height = height;
	image->m_Flags = flags;
	image->m_UVScaleBias = UVScaleBias();
	
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
	image->m_Atlas = 0;
	image->m_Texture = 0;
	image->m_Width = (int)FreeImage_GetWidth(bitmap);
	image->m_Height = (int)FreeImage_GetHeight(bitmap);
	image->m_Flags = flags;
	image->m_UVScaleBias = UVScaleBias();
	
	return Bacon_Error_None;
}

int Bacon_GetImageRegion(int* outImage, int imageHandle, int x1, int y1, int x2, int y2)
{
	if (!outImage)
		return Bacon_Error_InvalidHandle;
	
	Image* image = s_Impl->m_Images.Get(imageHandle);
	if (!image)
		return Bacon_Error_InvalidHandle;

	*outImage = s_Impl->m_Images.Alloc();
	Image* region = s_Impl->m_Images.Get(*outImage);
	region->m_Bitmap = nullptr;
	region->m_Atlas = 0;
	region->m_Flags = image->m_Flags;
	region->m_Width = x2 - x1;
	region->m_Height = y2 - y1;
	
	UVScaleBias const& sb = image->m_UVScaleBias;
	region->m_UVScaleBias = UVScaleBias(sb.m_ScaleX * (float)(x2 - x1) / image->m_Width,
										sb.m_ScaleY * (float)(y2 - y1) / image->m_Height,
										sb.m_BiasX + x1 / (float) image->m_Width * sb.m_ScaleX,
										sb.m_BiasY + y1 / (float) image->m_Height * sb.m_ScaleY);
	
	Texture* texture = s_Impl->m_Textures.Get(image->m_Texture);
	if (texture)
	{
		// Share texture
		region->m_Texture = image->m_Texture;
		++texture->m_RefCount;
	}
	else
	{
		// Image doesn't have a texture yet, so point to the image and set flag to resolve
		// as such.
		region->m_Texture = imageHandle;
		region->m_Flags |= Bacon_ImageFlags_Internal_TextureIsImage;
	}
	
	return Bacon_Error_None;
}

static void ReleaseTexture(int textureHandle)
{
	Texture* texture = s_Impl->m_Textures.Get(textureHandle);
	if (texture)
	{
		if (--texture->m_RefCount == 0)
		{
			if (texture->m_TextureId)
				s_Impl->m_PendingDeleteTextures.push_back(texture->m_TextureId);
			
			if (texture->m_FrameBuffer)
				s_Impl->m_PendingDeleteFrameBuffers.push_back(texture->m_FrameBuffer);
			
			s_Impl->m_Textures.Free(textureHandle);
		}
	}
}

static void ReleaseTextureAtlas(int textureAtlasHandle)
{
	TextureAtlas* atlas = s_Impl->m_TextureAtlases.Get(textureAtlasHandle);
	if (atlas)
	{
		if (--atlas->m_RefCount == 0)
		{
			if (atlas->m_Bitmap)
				FreeImage_Unload(atlas->m_Bitmap);
			
			ReleaseTexture(atlas->m_Texture);
			
			s_Impl->m_TextureAtlases.Free(textureAtlasHandle);
		}
	}
}

int Bacon_UnloadImage(int handle)
{
	Image* image = s_Impl->m_Images.Get(handle);
	if (!image)
		return Bacon_Error_InvalidHandle;

	if (image->m_Bitmap)
		FreeImage_Unload(image->m_Bitmap);
	
	if (!(image->m_Flags & Bacon_ImageFlags_Internal_TextureIsImage))
		ReleaseTexture(image->m_Texture);

	ReleaseTextureAtlas(image->m_Atlas);
	
	s_Impl->m_Images.Free(handle);
	
	return Bacon_Error_None;
}

static void UpdateTexture(Texture* texture, FIBITMAP* bitmap)
{
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
			pitch = texture->m_Width * 4;
			data = (BYTE*)malloc(texture->m_Height * pitch);
			FreeImage_ConvertToRawBits(data, bitmap, pitch, 32, 0, 0, 0, FALSE);
		}
	
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}

#if BACON_PLATFORM_ANGLE
	internalFormat = format;
#endif
	
	glActiveTexture(GL_TEXTURE0);
	s_Impl->m_CurrentTextureUnits[0] = s_Impl->m_Textures.GetHandle(texture);
	
	// TODO optionally use TexSubImage2D
	
	if (!texture->m_TextureId)
		glGenTextures(1, &texture->m_TextureId);
	glBindTexture(GL_TEXTURE_2D, texture->m_TextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, texture->m_Width, texture->m_Height, 0, format, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	if (ownsData)
		free(data);

}

static Texture* CreateTexture(int* outTextureHandle, FIBITMAP* bitmap, int width, int height)
{
	*outTextureHandle = s_Impl->m_Textures.Alloc();
	Texture* texture = s_Impl->m_Textures.Get(*outTextureHandle);
	texture->m_Width = width;
	texture->m_Height = height;
	texture->m_TextureId = 0;

	UpdateTexture(texture, bitmap);
	
	return texture;
}

static void Blit32Line(char* destData, int destPitch, const char* srcData, int srcPitch, int destX, int destY, int srcX, int srcY, int size, int margin)
{
	char* dest = destData + destPitch * destY + destX * 4;
	const char* src = srcData + srcPitch * srcY + srcX * 4;
	
	// Left margin
	for (int x = -margin; x < 0; ++x)
		memcpy(dest + x * 4, src, 4);
		
	// Row
	memcpy(dest, src, size * 4);

	// Right margin
	for (int x = size * 4; x < (size + margin) * 4; x += 4)
		memcpy(dest + x, src + (size - 1) * 4, 4);
}

// Blit entire srcBitmap int destRect of destBitmap.  If destMargin > 0, adds padding pixels around
// destRect (caller's responsibility to ensure dest bitmap is large enough)
static void Blit32(FIBITMAP* destBitmap, FIBITMAP* srcBitmap, Rect const& destRect, int destMargin)
{
	const char* srcData;
	char* ownedSrcData = nullptr;
	if (FreeImage_GetBPP(srcBitmap) == 32)
	{
		srcData = (char*)FreeImage_GetBits(srcBitmap);
	}
	else
	{
		// TODO support more source formats directly, at least 24bpp
		int pitch = FreeImage_GetWidth(srcBitmap) * 4;
		srcData = ownedSrcData = (char*)malloc(pitch * FreeImage_GetHeight(srcBitmap));
		FreeImage_ConvertToRawBits((BYTE*)ownedSrcData, srcBitmap, pitch, 32, 0, 0, 0, FALSE);
	}

	assert(FreeImage_GetBPP(destBitmap) == 32);
	char* destData = (char*)FreeImage_GetBits(destBitmap);
	int destPitch = FreeImage_GetWidth(destBitmap) * 4;
	int srcPitch = FreeImage_GetWidth(srcBitmap) * 4;
	
	// Top margin
	for (int y = destRect.m_Top - destMargin; y < destRect.m_Top; ++y)
		Blit32Line(destData, destPitch, srcData, srcPitch, destRect.m_Left, y, 0, 0, destRect.GetWidth(), destMargin);
	
	// Image
	for (int y = destRect.m_Top; y < destRect.m_Bottom; ++y)
		Blit32Line(destData, destPitch, srcData, srcPitch, destRect.m_Left, y, 0, y - destRect.m_Top, destRect.GetWidth(), destMargin);

	// Bottom margin
	for (int y = destRect.m_Bottom; y < destRect.m_Bottom + destMargin; ++y)
		Blit32Line(destData, destPitch, srcData, srcPitch, destRect.m_Left, y, 0, destRect.GetHeight() - 1, destRect.GetWidth(), destMargin);

	if (ownedSrcData)
		free(ownedSrcData);
}

static bool IsAtlasFlagsCompatible(TextureAtlas* atlas, Image* image)
{
	return true; // TODO filtering
}

static TextureAtlas* AllocInTextureAtlas(Rect& outRect, Image* image)
{
	// Find an existing atlas with room and compatible flags
	for (TextureAtlas& atlas : s_Impl->m_TextureAtlases)
	{
		if (!IsAtlasFlagsCompatible(&atlas, image))
			continue;
		
		if (atlas.m_Allocator.Alloc(outRect, image->m_Width, image->m_Height, TextureAtlasMargin))
			return &atlas;
	}
	return nullptr;
}

static int NextPowerOfTwo(int n)
{
	if ((n & (n - 1)) == 0)
		return n;
	
	int v = 32;
	for (; v < n; v <<= 1)
		;
	return v;
}

static void AddImageToTextureAtlas(Image* image)
{
	Rect rect;
	int atlasHandle;
	TextureAtlas* atlas = AllocInTextureAtlas(rect, image);
	if (atlas)
	{
		// Alloc in existing atlas
		atlasHandle = s_Impl->m_TextureAtlases.GetHandle(atlas);
	}
	else
	{
		// Create a new atlas
		// TODO allow image to run against edges w/out bleeding into margin
		int size = NextPowerOfTwo(std::max(std::max(image->m_Width + TextureAtlasMargin * 2, image->m_Height + TextureAtlasMargin * 2),TextureAtlasMinSize));
		atlasHandle = s_Impl->m_TextureAtlases.Alloc();
		atlas = s_Impl->m_TextureAtlases.Get(atlasHandle);
		atlas->m_Allocator.Init(size, size);
		atlas->m_Texture = 0;
		atlas->m_Width = size;
		atlas->m_Height = size;
		atlas->m_Bitmap = FreeImage_Allocate(atlas->m_Width, atlas->m_Height, 32);
		
		atlas->m_Allocator.Alloc(rect, image->m_Width, image->m_Height, TextureAtlasMargin);
		CreateTexture(&atlas->m_Texture, atlas->m_Bitmap, atlas->m_Width, atlas->m_Height);
	}

	assert(rect.GetWidth() == image->m_Width &&
		   rect.GetHeight() == image->m_Height);
	if (image->m_Bitmap)
		Blit32(atlas->m_Bitmap, image->m_Bitmap, rect, TextureAtlasMargin);
	image->m_Atlas = atlasHandle;
	image->m_UVScaleBias = UVScaleBias(rect.GetWidth() / (float)atlas->m_Width,
									   rect.GetHeight() / (float)atlas->m_Height,
									   rect.m_Left / (float)atlas->m_Width,
									   rect.m_Top / (float)atlas->m_Height);
	++atlas->m_RefCount;
	
	image->m_Texture = atlas->m_Texture;
}

static void FillTextureAtlases()
{
	// Collect images that need atlasing
	vector<Image*> images;
	images.reserve(s_Impl->m_Images.GetCount());
	for (Image& image : s_Impl->m_Images)
	{
		if (image.m_Texture == 0 &&
			image.m_Flags & Bacon_ImageFlags_Atlas)
		{
			images.push_back(&image);
		}
	}
	
	// Sort by longest edge
	std::sort(images.begin(), images.end(), [](Image* a, Image* b) {
		int sizeA = std::max(a->m_Width, a->m_Height);
		int sizeB = std::max(b->m_Width, b->m_Height);
		return sizeB < sizeA;
	});
	
	// Insert each image
	for (Image* image : images)
		AddImageToTextureAtlas(image);

	// Update all textures in texture atlases that have been invalidated
	for (TextureAtlas& atlas : s_Impl->m_TextureAtlases)
	{
		// TODO track invalid region
		Texture* texture = s_Impl->m_Textures.Get(atlas.m_Texture);
		UpdateTexture(texture, atlas.m_Bitmap);
	}
}

static Texture* RealizeTexture(Image* image)
{
	if (image->m_Flags & Bacon_ImageFlags_Internal_TextureIsImage)
	{
		// Image::m_Texture is actually an image reference.  Resolve that parent
		// image first, then update our texture reference for next time
		Image* parent = s_Impl->m_Images.Get(image->m_Texture);
		assert(parent);
		RealizeTexture(parent);
		image->m_Texture = parent->m_Texture;
		image->m_Flags &= ~Bacon_ImageFlags_Internal_TextureIsImage;
	}
	
	Texture* texture = s_Impl->m_Textures.Get(image->m_Texture);
	if (!texture)
	{
		Bacon_Flush();
		
		if (image->m_Flags & Bacon_ImageFlags_Atlas)
		{
			FillTextureAtlases();
			texture = s_Impl->m_Textures.Get(image->m_Texture);
		}
		else
		{
			texture = CreateTexture(&image->m_Texture, image->m_Bitmap, image->m_Width, image->m_Height);
		}
		
		if (image->m_Flags & Bacon_ImageFlags_DiscardBitmap)
		{
			FreeImage_Unload(image->m_Bitmap);
			image->m_Bitmap = nullptr;
		}
	}
	
	return texture;
}

static int BindTexture(int textureHandle)
{
	Texture* texture = s_Impl->m_Textures.Get(textureHandle);
	if (!texture)
		return Bacon_Error_InvalidHandle;
	
	glBindTexture(GL_TEXTURE_2D, texture->m_TextureId);
	return Bacon_Error_None;
}

static bool CreateTextureFrameBuffer(Texture* texture)
{
	glGenFramebuffers(1, &texture->m_FrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, texture->m_FrameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->m_TextureId, 0);
	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

static int BindFrameBuffer(int imageHandle)
{
	if (!imageHandle)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		Bacon_SetViewport(0, 0, s_Impl->m_FrameBufferWidth, s_Impl->m_FrameBufferHeight);
		return Bacon_Error_None;
	}
	
	Image* image = s_Impl->m_Images.Get(imageHandle);
	if (!image)
		return Bacon_Error_InvalidHandle;

	Texture* texture = RealizeTexture(image);
	
	if (!CreateTextureFrameBuffer(texture))
		return Bacon_Error_Unknown;
	
	glBindFramebuffer(GL_FRAMEBUFFER, texture->m_FrameBuffer);
	vec2f origin = image->m_UVScaleBias.Apply(vec2f(0.f, 0.f)) * vec2f(texture->m_Width, texture->m_Height);
	Bacon_SetViewport((int)origin.x(), (int)origin.y(), image->m_Width, image->m_Height);
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
	{
		Image* frameBufferImage = s_Impl->m_Images.Get(s_Impl->m_CurrentFrameBuffer);
		Texture* frameBufferTexture = s_Impl->m_Textures.Get(frameBufferImage->m_Texture);
		frameBufferHeight = frameBufferTexture->m_Height;
	}
	
	glViewport(x, frameBufferHeight - (y + height), width, height);
	vmml::mat4f projection = frustumf(0.f, (float)width, (float)height, 0.f, -1.f, 1.f).compute_ortho_matrix();
	SetSharedUniformValue(s_Impl->m_ProjectionUniform, projection, sizeof(mat4f));
	return Bacon_Error_None;
}

int Bacon_SetShader(int shader)
{
	REQUIRE_GL();
	
	if (!shader)
		shader = s_Impl->m_DefaultShader;

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

static float DefaultQuadTexCoords[] = {
	0, 1,
	0, 0,
	1, 0,
	1, 1
};

static float DefaultQuadColors[] = {
	1.f, 1.f, 1.f, 1.f,
	1.f, 1.f, 1.f, 1.f,
	1.f, 1.f, 1.f, 1.f,
	1.f, 1.f, 1.f, 1.f,
};

int Bacon_DrawImage(int image, float x1, float y1, float x2, float y2)
{
	float z = s_Impl->m_CurrentZ;
	float positions[] = {
		x1, y1, z,
		x1, y2, z,
		x2, y2, z,
		x2, y1, z
	};
	
	return Bacon_DrawImageQuad(image, positions, DefaultQuadTexCoords, DefaultQuadColors);
}

int Bacon_DrawImageRegion(int imageHandle, float x1, float y1, float x2, float y2,
						 float u1, float v1, float u2, float v2)
{
	Image* image = s_Impl->m_Images.Get(imageHandle);
	if (!image)
		return Bacon_Error_InvalidHandle;

	float w = (float)image->m_Width;
	float h = (float)image->m_Height;
	u1 = u1 / w;
	v1 = 1.f - v1 / h;
	u2 = u2 / h;
	v2 = 1.f - v2 / h;

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
	
	return Bacon_DrawImageQuad(imageHandle, positions, texCoords, colors);
}

inline void SetCurrentMode(GLuint mode)
{
	if (mode != s_Impl->m_CurrentMode)
	{
		Bacon_Flush();
		s_Impl->m_CurrentMode = mode;
	}
}

inline void SetCurrentTexture(int textureHandle)
{
	SetSharedUniformValue(s_Impl->m_Texture0Uniform, textureHandle);
}

inline void SetCurrentImage(Image* image)
{
	RealizeTexture(image);
	SetCurrentTexture(image->m_Texture);
}

void Graphics_DrawQuad(float* positions, float* texCoords, float* colors, UVScaleBias const& uvScaleBias)
{
	SetCurrentMode(GL_TRIANGLES);
	
	mat4f const& transform = s_Impl->m_TransformStack.back();
	vec4f const& color = s_Impl->m_ColorStack.back();
	vector<Vertex>& vertices = s_Impl->m_Vertices;
	unsigned short index = vertices.size();
	for (int i = 0; i < 4; ++i)
		vertices.push_back(Vertex(
								  transform * vec3f(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]),
								  uvScaleBias.Apply(vec2f(texCoords[i * 2], texCoords[i * 2 + 1])),
								  color * vec4f(colors[i * 4], colors[i * 4 + 1], colors[i * 4 + 2], colors[i * 4 + 3])
								  ));
	
	vector<unsigned short>& indices = s_Impl->m_Indices;
	indices.push_back(index + 0);
	indices.push_back(index + 1);
	indices.push_back(index + 2);
	indices.push_back(index + 0);
	indices.push_back(index + 2);
	indices.push_back(index + 3);
}

int Bacon_DrawImageQuad(int imageHandle, float* positions, float* texCoords, float* colors)
{
	REQUIRE_GL();

	Image* image = s_Impl->m_Images.Get(imageHandle);
	if (!image)
		return Bacon_Error_InvalidHandle;

	SetCurrentImage(image);
	Graphics_DrawQuad(positions, texCoords, colors, image->m_UVScaleBias);
	
	return Bacon_Error_None;
}


void Graphics_DrawTexture(int texture, float x1, float y1, float x2, float y2)
{
	float z = s_Impl->m_CurrentZ;
	float positions[] = {
		x1, y1, z,
		x1, y2, z,
		x2, y2, z,
		x2, y1, z
	};
	
	SetCurrentTexture(texture);
	Graphics_DrawQuad(positions, DefaultQuadTexCoords, DefaultQuadColors, UVScaleBias());
}

int Bacon_DrawLine(float x1, float y1, float x2, float y2)
{
	REQUIRE_GL();
	
	Image* image = s_Impl->m_Images.Get(s_Impl->m_BlankImage);
	if (!image)
		return Bacon_Error_InvalidHandle;
	
	SetCurrentImage(image);
	SetCurrentMode(GL_LINES);

	float z = s_Impl->m_CurrentZ;
	mat4f const& transform = s_Impl->m_TransformStack.back();
	vec4f const& color = s_Impl->m_ColorStack.back();
	vector<Vertex>& vertices = s_Impl->m_Vertices;
	unsigned short index = vertices.size();
	vertices.push_back(Vertex(transform * vec3f(x1, y1, z), image->m_UVScaleBias.Apply(vec2f(0, 0)), color));
	vertices.push_back(Vertex(transform * vec3f(x2, y2, z), image->m_UVScaleBias.Apply(vec2f(1, 1)), color));

	vector<unsigned short>& indices = s_Impl->m_Indices;
	indices.push_back(index + 0);
	indices.push_back(index + 1);
	
	return Bacon_Error_None;
}

static unsigned short DrawRectIndices[] = {
	0, 1,
	1, 2,
	0, 3,
	3, 2
};

int Bacon_DrawRect(float x1, float y1, float x2, float y2)
{
	float z = s_Impl->m_CurrentZ;

	// Magic coordinate jiggle to get all pixel corners filled
	x1 += 0.375f;
	y2 -= 0.375f;
	
	Image* image = s_Impl->m_Images.Get(s_Impl->m_BlankImage);
	if (!image)
		return Bacon_Error_InvalidHandle;
	
	SetCurrentImage(image);
	SetCurrentMode(GL_LINES);
	mat4f const& transform = s_Impl->m_TransformStack.back();
	vec4f const& color = s_Impl->m_ColorStack.back();
	vector<Vertex>& vertices = s_Impl->m_Vertices;
	unsigned short index = vertices.size();
	vertices.push_back(Vertex(transform * vec3f(x1, y1, z), image->m_UVScaleBias.Apply(vec2f(0, 1)), color));
	vertices.push_back(Vertex(transform * vec3f(x1, y2, z), image->m_UVScaleBias.Apply(vec2f(0, 0)), color));
	vertices.push_back(Vertex(transform * vec3f(x2, y2, z), image->m_UVScaleBias.Apply(vec2f(1, 0)), color));
	vertices.push_back(Vertex(transform * vec3f(x2, y1, z), image->m_UVScaleBias.Apply(vec2f(1, 1)), color));
	
	vector<unsigned short>& indices = s_Impl->m_Indices;
	for (int i = 0; i < BACON_ARRAY_COUNT(DrawRectIndices); ++i)
		indices.push_back(DrawRectIndices[i] + index);
	
	return Bacon_Error_None;
}

int Bacon_FillRect(float x1, float y1, float x2, float y2)
{
	return Bacon_DrawImage(s_Impl->m_BlankImage, x1, y1, x2, y2);
}

int Bacon_Flush()
{
	REQUIRE_GL();

	if (s_Impl->m_Indices.empty())
		return Bacon_Error_None;
	
	BindShaderUniforms();
	BindShaderTextureUnits();
	
	glBindBuffer(GL_ARRAY_BUFFER, s_Impl->m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * s_Impl->m_Vertices.size(), &s_Impl->m_Vertices[0], GL_DYNAMIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_Impl->m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * s_Impl->m_Indices.size(), &s_Impl->m_Indices[0], GL_DYNAMIC_DRAW);
	
	glDrawElements(s_Impl->m_CurrentMode, (int)s_Impl->m_Indices.size(), GL_UNSIGNED_SHORT, 0);
	
	s_Impl->m_Indices.clear();
	s_Impl->m_Vertices.clear();
	
	return Bacon_Error_None;
}

void Graphics_DrawDebugOverlay()
{
	int drawTextureAtlas = 1;
	for (TextureAtlas& atlas : s_Impl->m_TextureAtlases)
	{
		if (drawTextureAtlas-- != 0)
			continue;
	
		Bacon_SetColor(0, 0, 0, 1);
		Bacon_DrawImage(s_Impl->m_BlankImage, 0, 0, atlas.m_Width, atlas.m_Height);
		Bacon_SetColor(1, 1, 1, 1);
		Graphics_DrawTexture(atlas.m_Texture, 0, atlas.m_Height, atlas.m_Width, 0);
		
		Bacon_SetColor(0.f, 1.f, 0.f, 1.f);
		for (Rect r : atlas.m_Allocator.GetFreeRects())
		{
			r = r.Inset(2);
			Bacon_DrawRect(r.m_Left, r.m_Top, r.m_Right, r.m_Bottom);
		}
	}
}
