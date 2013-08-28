import bacon
import math

kitten = bacon.Image('res/kitten.png', wrap=True)

class Game(bacon.Game):
    def on_tick(self):
        bacon.clear(0, 0, 0, 1)
        bacon.draw_image_region(kitten, 0, 0, bacon.window.width, bacon.window.height,
                                0, 0, bacon.window.width, bacon.window.height)
bacon.run(Game())