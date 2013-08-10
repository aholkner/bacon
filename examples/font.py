import bacon

font = bacon.Font('res/DejaVuSans.ttf', 24)
print('The font has ascent %d and descent %d' % (font.ascent, font.descent))

class Game(bacon.Game):
    def on_tick(self):
        bacon.clear(0, 0, 0, 1)
        bacon.draw_string(font, 'Hello, Bacon!', 50, 50)
bacon.run(Game())