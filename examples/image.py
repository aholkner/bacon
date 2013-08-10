import bacon

kitten = bacon.Image('res/kitten.png')
print('The kitten image has dimensions %dx%d' % (kitten.width, kitten.height))

class Game(bacon.Game):
    def on_tick(self):
    	bacon.clear(0, 0, 0, 1)
        bacon.draw_image(kitten, 0, 0)
bacon.run(Game())