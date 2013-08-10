import bacon
import math

kitten = bacon.Image('res/kitten.png')

class Game(bacon.Game):
    def on_tick(self):
    	bacon.clear(0, 0, 0, 1)
    	bacon.translate(kitten.width / 2, kitten.height / 2)
    	bacon.rotate(math.pi / 4)
        bacon.draw_image(kitten, -kitten.width / 2, -kitten.height / 2)
bacon.run(Game())