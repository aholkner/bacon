from ctypes import *

from bacon.core import lib
from bacon.readonly_collections import ReadOnlyDict
from bacon import native
from bacon import commands

ShaderUniformType = native.ShaderUniformType

class Shader(object):
    '''A GPU shader object that can be passed to :func:`set_shader`.

    The default shader is as follows, and demonstrates the available vertex attributes and
    uniforms::

        default_shader = Shader(vertex_source=
                                """
                                precision highp float;
                                attribute vec3 a_Position;
                                attribute vec2 a_TexCoord0;
                                attribute vec4 a_Color;
                                
                                varying vec2 v_TexCoord0;
                                varying vec4 v_Color;
                                
                                uniform mat4 g_Projection;
                                
                                void main()
                                {
                                    gl_Position = g_Projection * vec4(a_Position, 1.0);
                                    v_TexCoord0 = a_TexCoord0;
                                    v_Color = a_Color;
                                }
                                """,
                                fragment_source=
                                """
                                precision highp float;
                                uniform sampler2D g_Texture0;
                                varying vec2 v_TexCoord0;
                                varying vec4 v_Color;
                                
                                void main()
                                {
                                    gl_FragColor = v_Color * texture2D(g_Texture0, v_TexCoord0);
                                }
                                """)

    The shading language is OpenGL-ES SL 2.  The shader will be translated automatically into
    HLSL on Windows, and into GLSL on other desktop platforms.

    :param vertex_source: string of source code for the vertex shader
    :param fragment_source: string of source code for the fragment shader
    '''
    def __init__(self, vertex_source, fragment_source):
        self._vertex_source = vertex_source
        self._fragment_source = fragment_source

        handle = c_int()
        lib.CreateShader(byref(handle), vertex_source.encode('utf-8'), fragment_source.encode('utf-8'))
        self._handle = handle.value

        self._uniforms = {}

        enum_uniform_callback = lib.EnumShaderUniformsCallback(self._on_enum_uniform)
        lib.EnumShaderUniforms(handle, enum_uniform_callback, None)

        self._uniforms = ReadOnlyDict(self.uniforms)

    _shared_uniforms = {}

    def _on_enum_uniform(self, shader, uniform, name, type, array_count, arg):
        name = name.decode('utf-8')
        if name.startswith('g_'):
            shared_uniforms = self.__class__._shared_uniforms
            if name not in shared_uniforms:
                self._uniforms[name] = shared_uniforms[name] = ShaderUniform(name, type, array_count, _shader=self, _uniform=uniform)
            else:
                self._uniforms[name] = shared_uniforms[name]
        else:
            self._uniforms[name] = ShaderUniform(name, type, array_count, _shader=self, _uniform=uniform)

    @property
    def uniforms(self):
        '''Map of shader uniforms available on this shader.

        :type: read-only dictionary of string to :class:`ShaderUniform`
        '''
        return self._uniforms

    @property
    def vertex_source(self):
        '''Get the vertex shader source

        :type: ``str``
        '''
        return self._vertex_source

    @property
    def fragment_source(self):
        '''Get the fragment shader source

        :type: ``str``
        '''
        return self._fragment_source

class _ShaderUniformNativeType(object):
    def __init__(self, ctype, converter=None):
        self.ctype = ctype
        if converter:
            self.converter = converter
        elif hasattr(ctype, '_length_'):
            self.converter = lambda x: ctype(*x)
        else:
            self.converter = None

if commands._enabled:
    _shader_uniform_native_types = {
        native.ShaderUniformType.float_:    _ShaderUniformNativeType(c_float, lambda value : (value,)),
        native.ShaderUniformType.vec2:      _ShaderUniformNativeType(c_float),
        native.ShaderUniformType.vec3:      _ShaderUniformNativeType(c_float),
        native.ShaderUniformType.vec4:      _ShaderUniformNativeType(c_float),
        native.ShaderUniformType.int_:      _ShaderUniformNativeType(c_int, lambda value : (value,)),
        native.ShaderUniformType.ivec2:     _ShaderUniformNativeType(c_int),
        native.ShaderUniformType.ivec3:     _ShaderUniformNativeType(c_int),
        native.ShaderUniformType.ivec4:     _ShaderUniformNativeType(c_int),
        native.ShaderUniformType.bool_:     _ShaderUniformNativeType(c_int, lambda value : (value,)),
        native.ShaderUniformType.bvec2:     _ShaderUniformNativeType(c_int),
        native.ShaderUniformType.bvec3:     _ShaderUniformNativeType(c_int),
        native.ShaderUniformType.bvec4:     _ShaderUniformNativeType(c_int),
        native.ShaderUniformType.mat2:      _ShaderUniformNativeType(c_float),
        native.ShaderUniformType.mat3:      _ShaderUniformNativeType(c_float),
        native.ShaderUniformType.mat4:      _ShaderUniformNativeType(c_float),
        native.ShaderUniformType.sampler2D: _ShaderUniformNativeType(c_int, lambda image : (c_int(image._handle),))
    }
else:
    _shader_uniform_native_types = {
        native.ShaderUniformType.float_:    _ShaderUniformNativeType(c_float),
        native.ShaderUniformType.vec2:      _ShaderUniformNativeType(c_float * 2),
        native.ShaderUniformType.vec3:      _ShaderUniformNativeType(c_float * 3),
        native.ShaderUniformType.vec4:      _ShaderUniformNativeType(c_float * 4),
        native.ShaderUniformType.int_:      _ShaderUniformNativeType(c_int),
        native.ShaderUniformType.ivec2:     _ShaderUniformNativeType(c_int * 2),
        native.ShaderUniformType.ivec3:     _ShaderUniformNativeType(c_int * 3),
        native.ShaderUniformType.ivec4:     _ShaderUniformNativeType(c_int * 4),
        native.ShaderUniformType.bool_:     _ShaderUniformNativeType(c_int),
        native.ShaderUniformType.bvec2:     _ShaderUniformNativeType(c_int * 2),
        native.ShaderUniformType.bvec3:     _ShaderUniformNativeType(c_int * 3),
        native.ShaderUniformType.bvec4:     _ShaderUniformNativeType(c_int * 4),
        native.ShaderUniformType.mat2:      _ShaderUniformNativeType(c_float * 4),
        native.ShaderUniformType.mat3:      _ShaderUniformNativeType(c_float * 9),
        native.ShaderUniformType.mat4:      _ShaderUniformNativeType(c_float * 16),
        native.ShaderUniformType.sampler2D: _ShaderUniformNativeType(c_int, lambda image : c_int(image._handle))
    }
    

class ShaderUniform(object):
    '''A uniform variable, either shared between all shaders (if its name begins with ``g_``),
    specific to a particular shader.

    :see: :attr:`Shader.uniforms`

    :param name: name of the shared shader uniform to create, must begin with ``g_``
    :param type: type of the uniform variable, a member of :class:`ShaderUniformType`
    :param array_count: number of elements in the uniform array, or 1 if the uniform is not an array
    '''
    def __init__(self, name, type, array_count=1, _shader=None, _uniform=None):
        if _shader:
            # Create a non-shared uniform already bound to a shader
            self._shader_handle = _shader._handle
            self._uniform_handle = _uniform
        else:
            # Create a shared uniform not yet bound to any shader
            if not name.startswith('g_'):
                raise ValueError('Shader uniforms constructed manually must be shared, so the name must begin with "g_"')
            if name in Shader._shared_uniforms:
                raise ValueError('Shared shader uniform already exists')
            self._shader_handle = None
            _uniform = c_int()
            lib.CreateSharedShaderUniform(byref(_uniform), name.encode('utf-8'), type, array_count)
            self._uniform_handle = _uniform.value
            Shader._shared_uniforms[name] = self

        self._name = name
        self._type = type
        self._array_count = array_count
        self._value = None

        try:
            native_type = _shader_uniform_native_types[type]
        except KeyError:
            raise ValueError('Unsupported shader uniform type %s' % native.ShaderUniformType.tostring(type))

        if commands._enabled:
            if _shader:
                if native_type.ctype == c_float:
                    self._set_func = lib.SetShaderUniformFloats
                else:
                    self._set_func = lib.SetShaderUniformInts
            else:
                if native_type.ctype == c_float:
                    self._set_func = lib.SetSharedShaderUniformFloats
                else:
                    self._set_func = lib.SetSharedShaderUniformInts
            self._converter = native_type.converter
            if array_count > 1:
                def flatten(v):
                    return tuple(item for sublist in v for item in sublist)
                if self._converter:
                    self._converter = lambda v: flatten(converter(x) for x in v)
                else:
                    self._converter = flatten
            else:
                self._converter = native_type.converter
        else:
            if not native_type.converter:
                native_type.converter = native_type.ctype
            if array_count > 1:
                ctype = native_type.ctype
                converter = native_type.converter
                self._converter = lambda v: (ctype * array_count)(*(converter(x) for x in v))
            else:
                self._converter = native_type.converter
            
    def __repr__(self):
        return 'ShaderUniform(%d, %s, %s, %d)' % (self._shader_handle, self.name, native.ShaderUniformType.tostring(self.type), self.array_count)

    @property
    def shared(self):
        '''Boolean indicating if this shader uniform's value is shared between all shaders

        :type: ``bool``
        '''
        return self._name.startswith('g_')

    def _get_value(self):
        return self._value
    def _set_value(self, value):
        self._value = value
        if commands._enabled:
            if self._converter:
                value = self._converter(value)
            if self._shader_handle is not None:
                self._set_func(self._shader_handle, self._uniform_handle, value)
            else:
                self._set_func(self._uniform_handle, value)
        else:
            native_value = self._converter(value)
            if self._shader_handle is not None:
                lib.SetShaderUniform(self._shader_handle, self._uniform_handle, byref(native_value), sizeof(native_value))
            else:
                lib.SetSharedShaderUniform(self._uniform_handle, byref(native_value), sizeof(native_value))
    value = property(_get_value, _set_value, doc='''Current value of the uniform as seen by the shader.

        Uniforms with names beginning with ``g_`` (e.g., ``g_Projection``) share their value across all shaders.  Otherwise,
        the uniform value is unique to this shader.

        The type of the value depends on the type of the uniform:

        * ``float``, ``int``, ``bool``: their equivalent Python types
        * ``vec[2-4]``, ``ivec[2-4]``, ``bvec[2-4]``: a sequence of equivalent Python types (e.g., a sequence of 3 floats for ``vec3``)
        * ``mat2``, ``mat3``, ``mat4``: a sequence of 4, 9 or 16 floats, respectively
        * ``sampler2D``: an :class:`Image`

        For uniform arrays, the value is a sequence of the above types.  For example, a uniform of type ``vec2[3]`` can be assigned::

            value = ((0, 1), (2, 3), (4, 5))

        ''')

    @property
    def name(self):
        '''Name of the uniform, as it appears in the shader.

        :type: ``str``
        '''
        return self._name

    @property
    def type(self):
        '''Type of the uniform, or, if the uniform is an array, the element type of the array.

        :type: an enumeration of :class:`ShaderUniformType`
        '''
        return self._type

    @property
    def array_count(self):
        '''The size of the array, or 1 if this uniform is not an array.

        :type: ``int``
        '''
        return self._array_count
