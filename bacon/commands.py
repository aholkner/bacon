from bacon import native
from bacon.core import lib
from ctypes import *
    
_commands = []
_data = []

_enabled = False

def PushTransform():
    _commands.append(native.Commands.push_transform)

def PopTransform():
    _commands.append(native.Commands.pop_transform)

def Translate(x, y):
    _commands.append(native.Commands.translate)
    _data.extend((x, y))

def Scale(sx, sy):
    _commands.append(native.Commands.scale)
    _data.extend((sx, sy))

def Rotate(angle):
    _commands.append(native.Commands.rotate)
    _data.append(angle)

def SetTransform(matrix):
    _commands.append(native.Commands.set_transform)
    _data.extend(matrix)

def PushColor():
    _commands.append(native.Commands.push_color)

def PopColor():
    _commands.append(native.Commands.pop_color)

def SetColor(r, g, b, a):
    _commands.append(native.Commands.set_color)
    _data.extend((r, g, b, a))

def MultiplyColor(r, g, b, a):
    _commands.append(native.Commands.multiply_color)
    _data.extend((r, g, b, a))

def SetBlending(src, dest):
    _commands.extend((native.Commands.set_blending, src, dest))

def DrawImage(image, x1, y1, x2, y2):
    _commands.extend((native.Commands.draw_image, image))
    _data.extend((x1, y1, x2, y2))

def DrawImageRegion(image, x1, y1, x2, y2, ix1, iy1, ix2, iy2):
    _commands.extend((native.Commands.draw_image_region, image))
    _data.extend((x1, y1, x2, y2, ix1, iy1, ix2, iy2))

def DrawLine(x1, y1, x2, y2):
    _commands.append(native.Commands.draw_line)
    _data.extend((x1, y1, x2, y2, ix1, iy1, ix2, iy2))

def DrawRect(x1, y1, x2, y2):
    _commands.append(native.Commands.draw_rect)
    _data.extend((x1, y1, x2, y2, ix1, iy1, ix2, iy2))

def FillRect(x1, y1, x2, y2):
    _commands.append(native.Commands.fill_rect)
    _data.extend((x1, y1, x2, y2, ix1, iy1, ix2, iy2))

def SetShaderUniformFloats(handle, uniform, values):
    _commands.extend((native.Commands.set_shader_uniform_floats, handle, uniform, len(values)))
    _data.extend(values)

def SetShaderUniformInts(handle, uniform, values):
    _commands.extend((native.Commands.set_shader_uniform_ints, handle, uniform, len(values)))
    _commands.extend(values)
    
def SetSharedShaderUniformFloats(handle, values):
    _commands.extend((native.Commands.set_shared_shader_uniform_floats, handle, len(values)))
    _data.extend(values)

def SetSharedShaderUniformInts(handle, values):
    _commands.extend((native.Commands.set_shared_shader_uniform_ints, handle, len(values)))
    _commands.extend(values)
    
def SetShader(handle):
    _commands.extend((native.Commands.set_shader, handle))

def Clear(r, g, b, a):
    _commands.append(native.Commands.clear)
    _data.extend((r, g, b, a))

def SetFrameBuffer(handle, content_scale):
    _commands.extend((native.Commands.set_frame_buffer, handle))
    _data.append(content_scale)

def SetViewport(x, y, width, height, content_scale):
    _commands.append(native.Commands.set_viewport)
    _data.extend((x, y, width, height, content_scale))

def flush():
    if _commands:
        lib.ExecuteCommands((c_int * len(_commands))(*_commands), len(_commands),
                               (c_float * len(_data))(*_data), len(_data))
        del _commands[:]
        del _data[:]

def init():
    global _enabled
    _enabled = True
    lib.PushTransform = PushTransform
    lib.PopTransform = PopTransform
    lib.Translate = Translate
    lib.Scale = Scale
    lib.Rotate = Rotate
    lib.SetTransform = SetTransform
    lib.PushColor = PushColor
    lib.PopColor = PopColor
    lib.SetColor = SetColor
    lib.MultiplyColor = MultiplyColor
    lib.SetBlending = SetBlending
    lib.DrawImage = DrawImage
    lib.DrawImageRegion = DrawImageRegion
    lib.DrawLine = DrawLine
    lib.DrawRect = DrawRect
    lib.FillRect = FillRect
    lib.SetShaderUniformFloats = SetShaderUniformFloats
    lib.SetShaderUniformInts = SetShaderUniformInts
    lib.SetSharedShaderUniformFloats = SetSharedShaderUniformFloats
    lib.SetSharedShaderUniformInts = SetSharedShaderUniformInts
    lib.SetShader = SetShader
    lib.Clear = Clear
    lib.SetFrameBuffer = SetFrameBuffer
    lib.SetViewport = SetViewport

