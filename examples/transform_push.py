import bacon
import math

kitten = bacon.Image('res/kitten.png')
font = bacon.Font('res/DejaVuSans.ttf', 24)

class Game(bacon.Game):
    def on_tick(self):
    	bacon.clear(0, 0, 0, 1)
    	
    	bacon.push_transform()
    	bacon.translate(kitten.width / 2, kitten.height / 2)
    	bacon.rotate(math.pi / 4)
        bacon.draw_image(kitten, -kitten.width / 2, -kitten.height / 2)
        bacon.pop_transform()

        bacon.draw_string(font, 'Hello', 20, 30)
bacon.run(Game())