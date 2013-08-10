import bacon

kitten = bacon.Image('res/kitten.png')

class Game(bacon.Game):
    def on_tick(self):
    	bacon.clear(0, 0, 0, 1)
    	bacon.set_color(1, 0, 0, 1)
        bacon.draw_image(kitten, 0, 0)
bacon.run(Game())