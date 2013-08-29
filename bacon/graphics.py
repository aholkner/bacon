from ctypes import *
from bacon.core import lib
from bacon import native
import bacon

BlendFlags = native.BlendFlags

if native._mock_native:
    def push_transform():
        pass
else:
    push_transform = lib.PushTransform
push_transform.__doc__ = '''Save the current graphics transform by pushing it on the transform stack.  It can be restored by
calling :func:`pop_transform`.
'''

if native._mock_native:
    def pop_transform():
        pass
else:
    pop_transform = lib.PopTransform
pop_transform.__doc__ = '''Restore a previously saved transform by popping it off the transform stack.
'''

if native._mock_native:
    def translate(x, y):
        pass
else:
    translate = lib.Translate
translate.__doc__ = '''Translate the current graphics transform by ``(x, y)`` units.
'''

if native._mock_native:
    def scale(sx, sy):
        pass
else:
    scale = lib.Scale
scale.__doc__ = '''Scale the current graphics transform by multiplying through ``(sx, sy)``.
'''

if native._mock_native:
    def rotate(radians):
        pass
else:
    rotate = lib.Rotate
rotate.__doc__ = '''Rotate the current graphics transform by ``radians`` counter-clockwise.
'''

def set_transform(matrix):
    '''Replace the current graphics transform with the given 4x4 matrix.  For example, to replace
    the transform with a translation by ``(x, y)``::

        bacon.set_transform([1, 0, 0, x,
                             0, 1, 0, y,
                             0, 0, 1, 0,
                             0, 0, 0, 1])

    :param matrix: a 4x4 matrix in column major order, represented as a flat 16 element sequence.
    '''
    lib.SetTransform((c_float * 16)(*matrix))

if native._mock_native:
    def push_color():
        pass
else:
    push_color = lib.PushColor
push_color.__doc__ = '''Save the current graphics color by pushing it on the color stack.  It can be restored with :func:`pop_color`.

The color stack is cleared at the beginning of each frame, and the default color reset to white.
'''

if native._mock_native:
    def pop_color():
        pass
else:
    pop_color = lib.PopColor
pop_color.__doc__ = '''Restore a previously saved graphics color by popping it off the color stack.
'''

if native._mock_native:
    def set_color(r, g, b, a):
        pass
else:
    set_color = lib.SetColor
set_color.__doc__ = '''Set the current graphics color to the given RGBA values.  Typically each component has a value between
0.0 and 1.0, however out-of-range values are permitted and may be used for special effects with an 
appropriate shader.

The color is reset to white at the beginning of each frame.
'''

if native._mock_native:
    def multiply_color(r, g, b, a):
        pass
else:
    multiply_color = lib.MultiplyColor
multiply_color.__doc__ = '''multiply_color(r, g, b, a)

Multiplies the current graphics color component-wise by the given RGBA values.
'''

if native._mock_native:
    def clear(r, g, b, a):
        pass
else:
    clear = lib.Clear
clear.__doc__ = '''Clear the current framebuffer to the given RGBA color.  Each color component must be in the range 0.0 to 1.0.
You should clear the default frame buffer at the beginning of each frame.  Failure to do so may cause visual
artifacts and/or poor performance on some platforms.
'''

def set_frame_buffer(image):
    '''Set the current frame buffer to the given image.  All subsequent drawing commands will be applied to the
    given image instead of the default frame buffer.  To resume rendering to the main frame buffer (the window),
    pass ``None`` as the image.

    The framebuffer is automatically reset to the default framebuffer at the beginning of each frame.

    :param image: the :class:`Image` to render to
    ''' 
    if image:
        lib.SetFrameBuffer(image._handle, image._content_scale)
    else:
        lib.SetFrameBuffer(0, bacon.window._content_scale)

if native._mock_native:
    def set_viewport(x, y, width, height):
        pass
else:
    set_viewport = lib.SetViewport
set_viewport.__doc__ = '''Set the current viewport in screen-space.  This affects both the GPU viewport, which provides a screen-space clip,
and the projection matrix, which is constructed from the viewport coordinates automatically.

Viewport coordinates are specified in pixel-space with (0, 0) at the upper-left corner.

The viewport is reset to the framebuffer dimensions at the beginning of each frame, and when switching framebuffers.
'''

def set_shader(shader):
    '''Set the current graphics shader.  All subsequent drawing commands will be rendered with this shader.
    To reset to the default shader, pass ``None`` as the argument.

    The shader is automatically reset to the default shader at the beginning of each frame.

    :param shader: a :class:`Shader` to render with
    '''
    lib.SetShader(shader._handle if shader else 0)

if native._mock_native:
    def set_blending(src_blend, dest_blend):
        pass
else:
    set_blending = lib.SetBlending
set_blending.__doc__ = '''Set the current graphics blend mode.  All subsequent drawing commands will be rendered with this blend mode.

The default blend mode is ``(BlendFlags.one, BlendFlags.one_minus_src_alpha)``, which is appropriate for
premultiplied alpha.

:param src_blend: a value from the :class:`BlendFlags` enumeration, specifying the blend contribution from the source fragment
:param dest_blend: a value from the :class:`BlendFlags` enumeration, specifying the blend contribution from the destination fragment
'''

def draw_image(image, x1, y1, x2 = None, y2 = None):
    '''Draw an image.

    The image's top-left corner is drawn at ``(x1, y1)``, and its lower-left at ``(x2, y2)``.  If ``x2`` and ``y2`` are omitted, they
    are calculated to render the image at its native resoultion.

    Note that images can be flipped and scaled by providing alternative values for ``x2`` and ``y2``.

    :param image: an :class:`Image` to draw
    '''
    if x2 is None:
        x2 = x1 + image.width
    if y2 is None:
        y2 = y1 + image.height
    lib.DrawImage(image._handle, x1, y1, x2, y2)

def draw_image_region(image, x1, y1, x2, y2,
                      ix1, iy1, ix2, iy2):
    '''Draw a rectangular region of an image.

    The part of the image contained by the rectangle in texel-space by the coordinates ``(ix1, iy1)`` to ``(ix2, iy2)`` is
    drawn at coordinates ``(x1, y1)`` to ``(x2, y2)``.  All coordinates have the origin ``(0, 0)`` at the upper-left corner.

    For example, to draw the left half of a ``100x100`` image at coordinates ``(x, y)``::

        bacon.draw_image_region(image, x, y, x + 50, y + 100,
                                0, 0, 50, 100)

    :param image: an :class:`Image` to draw
    '''
    lib.DrawImageRegion(image._handle, x1, y1, x2, y2, ix1, iy1, ix2, iy2)

if native._mock_native:
    def draw_line(x1, y1, x2, y2):
        pass
else:
    draw_line = lib.DrawLine
draw_line.__doc__ = '''Draw a line from coordinates ``(x1, y1)`` to ``(x2, y2)``.

No texture is applied.
'''

if native._mock_native:
    def draw_rect(x1, y1, x2, y2):
        pass
else:
    draw_rect = lib.DrawRect
draw_rect.__doc__ = '''Draw a rectangle bounding coordinates ``(x1, y1)`` to ``(x2, y2)``.

No texture is applied.
'''

if native._mock_native:
    def fill_rect(x1, y1, x2, y2):
        pass
else:
    fill_rect = lib.FillRect
fill_rect.__doc__ = '''Fill a rectangle bounding coordinates ``(x1, y1)`` to ``(x2, y2)``.

No texture is applied.
'''
