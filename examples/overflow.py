import logging
logging.basicConfig()

import bacon
bacon.window.resizable = True
bacon.window.width = 480

font = bacon.Font(None, 12)

runs = [
    bacon.GlyphRun(bacon.Style(font), '''Deques are a generalization of stacks and queues (the name is pronounced ``deck'' and is short for ``double-ended queue''). Deques support thread-safe, memory efficient appends and pops from either side of the deque with approximately the same O(1) performance in either direction.''')
]
glyph_layout = bacon.GlyphLayout(runs, 
    0, 
    bacon.window.height / 2, 
    width=bacon.window.width,
    align=bacon.Alignment.left, 
    vertical_align=bacon.VerticalAlignment.center,
    overflow=bacon.Overflow.wrap)

class Game(bacon.Game):
    def on_tick(self):
        bacon.clear(0, 0, 0, 1)
        glyph_layout.y = bacon.window.height / 2
        glyph_layout.width = bacon.window.width
        bacon.draw_glyph_layout(glyph_layout)
bacon.run(Game())