import collections

import bacon
from bacon import native

class Style(object):
    def __init__(self, font, color=None, background_color=None):
        self.font = font
        self.color = color
        self.background_color = background_color

class GlyphRun(object):
    def __init__(self, style, text, glyphs=None):
        if glyphs is None:
            glyphs = style.font.get_glyphs(text)
        self.glyphs = glyphs
        self.style = style
        self.advance = sum(g.advance for g in self.glyphs)

    def __repr__(self):
        return 'GlyphRun("%s")' % (''.join(g.char for g in self.glyphs))

class GlyphLine(object):
    def __init__(self, runs, content_width = None):
        self.runs = runs
        if content_width is None:
            content_width = sum(run.advance for run in runs)
        self.content_width = content_width
        self.ascent = min(run.style.font.ascent for run in runs)
        self.descent = max(run.style.font.descent for run in runs)
        self.x = 0
        self.y = 0

    def __repr__(self):
        return 'GlyphLine(runs=%r)' % self.runs


@native.enum
class Alignment(object):
    left = 0
    center = 1
    right = 2

@native.enum
class VerticalAlignment(object):
    baseline = 0
    top = 1
    center = 2
    bottom = 3

@native.enum
class Overflow(object):
    none = 0
    wrap = 1
    wrap_characters = 2

class GlyphLayout(object):
    '''Caches a layout of glyphs rendering a given string with bounding rectangle, layout metrics.
    '''
    def __init__(self, runs, x, y, width=None, height=None, align=Alignment.left, vertical_align=VerticalAlignment.baseline, overflow=Overflow.wrap):
        self._runs = runs
        self._x = x
        self._y = y
        self._width = width
        self._height = height
        self._align = align
        self._vertical_align = vertical_align
        self._overflow = overflow
        self._dirty = True

        self._content_width = None
        self._content_height = None
        self._lines = None

    def _get_runs(self):
        return self._runs
    def _set_runs(self, runs):
        self._runs = runs
        self._dirty = True
    runs = property(_get_runs, _set_runs)

    def _get_x(self):
        return self._x
    def _set_x(self, x):
        if x != self._x:
            self._x = x
            self._dirty = True
    x = property(_get_x, _set_x)

    def _get_y(self):
        return self._y
    def _set_y(self, y):
        if y != self._y:
            self._y = y
            self._dirty = True
    y = property(_get_y, _set_y)
    
    def _get_width(self):
        return self._width
    def _set_width(self, width):
        if width != self._width:
            self._width = width
            self._dirty = True
    width = property(_get_width, _set_width)

    def _get_height(self):
        return self._height
    def _set_height(self, height):
        if height != self._height:
            self._height = height
            self._dirty = True
    height = property(_get_height, _set_height)

    def _get_align(self):
        return self._align
    def _set_align(self, align):
        if align != self._align:
            self._align = align
            self._dirty = True
    align = property(_get_align, _set_align)

    def _get_vertical_align(self):
        return self._vertical_align
    def _set_vertical_align(self, vertical_align):
        if vertical_align != self._vertical_align:
            self._vertical_align = vertical_align
            self._dirty = True
    vertical_align = property(_get_vertical_align, _set_vertical_align)

    def _get_overflow(self):
        return self._overflow
    def _set_overflow(self, overflow):
        if overflow != self._overflow:
            self._overflow = overflow
            self._dirty = True
    overflow = property(_get_overflow, _set_height)

    @property
    def lines(self):
        if self._dirty:
            self._update()
        return self._lines

    @property
    def content_width(self):
        if self._dirty:
            self._update()
        return self._content_width

    @property
    def content_height(self):
        if self._dirty:
            self._update()
        return self._content_height

    def _update(self):
        self._dirty = False

        content_width = sum(run.advance for run in self._runs)
        if (self._width is None or 
            self._overflow == Overflow.none or
            content_width <= self._width):
            self._lines = [GlyphLine(self._runs, 
                                     content_width=content_width)]
        else:
            self._update_overflow()

        self._content_width = sum(line.content_width for line in self.lines)
        self._content_height = sum(line.descent - line.ascent for line in self.lines)

        self._update_line_position()

    def _update_overflow(self):
        remaining_runs = collections.deque(self._runs)
        line_runs = []
        lines = []
        x = 0
        while remaining_runs:
            line_runs.append(remaining_runs.popleft())
            x += line_runs[-1].advance
            if x > self._width:
                if self._overflow == Overflow.wrap:
                    self._break_runs_word(x, line_runs, remaining_runs)
                elif self._overflow == Overflow.wrap_characters:
                    self._break_runs_character(x, line_runs, remaining_runs)
                if line_runs:
                    lines.append(GlyphLine(line_runs))
                    line_runs = []
                    x = 0

        if line_runs:
            lines.append(GlyphLine(line_runs))
        self._lines = lines

    def _break_runs_word(self, x, line_runs, remaining_runs):
        # Scan backwards through each glyph in line_runs until suitable break
        # is found.  Push remaining glyphs into remaining_runs
        start_x = x
        width = self._width
        for run_i in range(len(line_runs) - 1, -1, -1):
            run = line_runs[run_i]
            glyphs = run.glyphs
            for i in range(len(glyphs) - 1, -1, -1):
                x -= glyphs[i].advance
                if x >= width:
                    continue
                
                if glyphs[i]._char in u' \u200B':
                    if run_i != 0 or i != 0:
                        return self._break(line_runs, remaining_runs, run_i, i, i + 1)

        # No breaks found.  Repeat scan, break on character
        self._break_runs_character(start_x, line_runs, remaining_runs)

    def _break_runs_character(self, x, line_runs, remaining_runs):
        # Scan backwards through each glyph in line_runs until suitable break
        # is found.  Push remaining glyphs into remaining_runs
        width = self._width
        for run_i in range(len(line_runs) - 1, -1, -1):
            run = line_runs[run_i]
            glyphs = run.glyphs
            for i in range(len(glyphs) - 1, -1, -1):
                x -= glyphs[i].advance
                if x >= width:
                    continue

                return self._break(line_runs, remaining_runs, run_i, i, i)

    def _break(self, line_runs, remaining_runs, run_i, end_glyph_i, start_glyph_i):
        split_run = line_runs[run_i]
        
        # Move entire runs to the right of run_i into remaining_runs
        for i in range(len(line_runs) - 1, run_i, -1):
            remaining_runs.appendleft(line_runs[i])
        del line_runs[run_i + 1:]

        # line_runs gets glyphs up to end_glyph_i of split_run
        line_runs[-1] = GlyphRun(split_run.style, '', split_run.glyphs[:end_glyph_i])

        # remaining_runs gets the right side of start_glyph_i
        remaining_runs.appendleft(GlyphRun(split_run.style, '', split_run.glyphs[start_glyph_i:]))

    def _update_line_position(self):
        if not self._lines:
            return

        x = self._x
        y = self._y
        width = self._width
        height = self._height
        align = self._align
        vertical_align = self._vertical_align

        if self._width is not None:
            # Align relative to box, not pivot
            if align == Alignment.center:
                x += width / 2
            elif align == Alignment.right:
                x += width

        if height is not None:
            # Align relative to box, not pivot
            if vertical_align == VerticalAlignment.center:
                y += height / 2
            elif vertical_align == VerticalAlignment.bottom:
                y += height
            elif vertical_align == VerticalAlignment.baseline:
                vertical_align = VerticalAlignment.top

        # Align first baseline vertically against pivot
        line = self.lines[0]
        if vertical_align == VerticalAlignment.center:
            y -= self._content_height / 2
        elif vertical_align == VerticalAlignment.bottom:
            y -= self._content_height + line.descent
        elif vertical_align == VerticalAlignment.baseline:
            y += line.ascent

        # Layout lines
        start_x = x
        for line in self._lines:
            x = start_x
            if align == Alignment.center:
                x -= line.content_width / 2
            elif align == Alignment.right:
                x -= line.content_width

            y -= self._lines[0].ascent

            line.x = x
            line.y = y

            y += self._lines[0].descent



def draw_string(font, text, x, y, width=None, height=None, align=Alignment.left, vertical_align=VerticalAlignment.baseline):
    '''Draw a string with the given font.

    :note: Text alignment and word-wrapping is not yet implemented.  The text is rendered with the left edge and
        baseline at ``(x, y)``.

    :param font: the :class:`Font` to render text with
    :param text: a string of text to render.
    '''
    style = Style(font)
    run = GlyphRun(style, text)
    glyph_layout = GlyphLayout([run], x, y, width, height, align, vertical_align)
    draw_glyph_layout(glyph_layout)


def draw_glyph_layout(glyph_layout):
    '''Draw a prepared :class:`GlyphLayout`
    '''
    pushed_color = False

    # Draw lines
    for line in glyph_layout.lines:
        x = line.x
        y = line.y
        
        for run in line.runs:
            style = run.style
            if style.color is not None:
                if not pushed_color:
                    bacon.push_color()
                    pushed_color = True
                bacon.set_color(*style.color)
            elif pushed_color:
                bacon.pop_color()
                pushed_color = False

            for glyph in run.glyphs:
                if glyph.image:
                    bacon.draw_image(glyph.image, x + glyph.offset_x, y - glyph.offset_y)
                x += glyph.advance

    if pushed_color:
        bacon.pop_color()