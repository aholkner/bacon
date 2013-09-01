from ctypes import *

from bacon.core import lib
from bacon import native
from bacon import resource
import bacon
import bacon.core

class Image(object):
    '''An image that can be passed to :func:`draw_image` and other rendering functions.

    There are two forms to the `Image` constructor.  The first loads an image from a file::

        Image(file, premultiply_alpha=True, discard_bitmap=True)

    The other creates an empty image which can then be rendered to using :func:`set_frame_buffer`::

        Image(width, height)

    There may be GPU limits on the maximum size of an image that can be uploaded to the GPU (typically 2048x2048 is
    a safe upper bound for current generation mobile devices; desktops may support up to 8192x8192).  There is no
    diagnostic if the device limit is exceeded, the image will not render.

    Images are retained by the renderer until :func:`unload` is explicitly called.

    :param file: path to an image file to load.  Supported formats include PNG, JPEG, BMP, TIF, etc.
    :param premultiply_alpha: if ``True`` (the default), the color channels are multiplied by the alpha channel when 
        image is loaded.  This allows the image to be alpha blended with bilinear interpolation between texels correctly.
        This paramater should be set to ``False`` if the original image data is required and won't be blended (for example, if 
        it will be used as a mask).
    :param discard_bitmap: if ``True`` (the default), the bitmap data backing the image will be discarded after
        a GPU texture has been created (which happens automatically the first time the image is rendered).  This saves
        memory.  The parameter should be set to ``False`` if the source data will be required for reasons besides rendering
        (there is currently no API for using an image this way).
    :param bool sample_nearest: if ``True``, the image is sampled with nearest (point) filtering.  This gives a "pixellated"
        effect when images are scaled or rotated.
    :param bool wrap: if ``True``, the image is sampled with wraparound texture coordinates, and the image is packed into its
        own texture.  The image must have power of two dimensions (i.e., 32x32, 64x64, etc.).  Use :func:`draw_image_region` with
        image coordinates that extend past the bounds of the image to render it with a repeating pattern.
    :param atlas: group index of the atlas to pack this image into.  If zero, the image is not packed into an atlas; otherwise,
        the image is permitted to be packed into other images with the same atlas number (although this is not guaranteed).
    :param width: width of the image to create, in texels
    :param height: height of the image to create, in texels
    :param float content_scale: optional scale factor for backing texture.  Defaults to 1.0 for images loaded from a file,
        defaults to :attr:`Window.content_scale` for empty images.  The effect of setting content_scale to 2.0 is that
        the ``width`` and ``height`` properties will be half the actual pixel values.
    '''

    def __init__(self, file=None, premultiply_alpha=True, discard_bitmap=True, sample_nearest=False, wrap=False, atlas=1, width=None, height=None, content_scale=None, handle=None):
        flags = 0
        if premultiply_alpha:
            flags |= native.ImageFlags.premultiply_alpha
        if discard_bitmap:
            flags |= native.ImageFlags.discard_bitmap
        if sample_nearest:
            flags |= native.ImageFlags.sample_nearest
        if wrap:
            flags |= native.ImageFlags.wrap
            atlas = 0
        if atlas:
            flags |= atlas << native.ImageFlags_atlas_shift
            
        if file:
            # Load image from file
            if handle:
                raise ValueError('`handle` is not a not valid argument if `file` is given')

            handle = c_int()
            lib.LoadImage(byref(handle), resource.get_resource_path(file).encode('utf-8'), flags)
            handle = handle.value
            
            if not content_scale:
                content_scale = 1.0

            if not width or not height:
                width = c_int()
                height = c_int()
                lib.GetImageSize(handle, byref(width), byref(height))
                width = width.value
                height = height.value

            width = int(width / content_scale)
            height = int(height / content_scale)

        elif width and height and not handle:
            # Create empty image of given dimensions
            if not content_scale:
                content_scale = bacon.window.content_scale

            handle = c_int()
            lib.CreateImage(byref(handle), int(width * content_scale), int(height * content_scale), flags)
            handle = handle.value

        if not handle:
            raise ValueError('invalid arguments to Image, must specify either `file` or `width` and `height`')

        self._handle = handle
        self._width = width
        self._height = height
        self._content_scale = content_scale

    def __del__(self):
        self.unload()

    def unload(self):
        '''Releases renderer resources associated with this image.'''
        lib.UnloadImage(self._handle)
        self._handle = -1

    @property
    def width(self):
        '''The width of the image, in texels (read-only).'''
        return self._width

    @property
    def height(self):
        '''The height of the image, in texels (read-only).'''
        return self._height

    @property
    def content_scale(self):
        '''The content scale applied to the image (read-only).'''
        return self._content_scale

    def get_region(self, x1, y1, x2, y2):
        '''Get an image that refers to the given rectangle within this image.  The image data is not actually
        copied; if the image region is rendered into, it will affect this image.

        :param int x1: left edge of the image region to return
        :param int y1: top edge of the image region to return
        :param int x2: right edge of the image region to return
        :param int y2: bottom edge of the image region to return
        :return: :class:`Image`
        '''
        handle = c_int()
        lib.GetImageRegion(byref(handle), self._handle, x1, y1, x2, y2)
        return Image(width = x2 - x1, height = y2 - y1, content_scale = self._content_scale, handle = handle)