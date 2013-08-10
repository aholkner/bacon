import bacon

font = bacon.Font('res/DejaVuSans.ttf', 48)

class Game(bacon.Game):
    def on_tick(self):
        bacon.clear(0, 0, 0, 1)
        bacon.set_blending(bacon.BlendFlags.one, bacon.BlendFlags.one)

        bacon.set_color(1, 0, 0, 1)
        bacon.draw_string(font, 'Bacon', 0, 50)

        bacon.set_color(0, 1, 0, 1)
        bacon.draw_string(font, 'Bacon', 10, 50)
        
        bacon.set_color(0, 0, 1, 1)
        bacon.draw_string(font, 'Bacon', 20, 50)

bacon.run(Game())