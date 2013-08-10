import bacon

kitten = bacon.Image('res/kitten.png')

bacon.window.width = 512
bacon.window.height = 512

class Game(bacon.Game):
    def on_tick(self):
        bacon.clear(0, 0, 0, 1)
        bacon.draw_image(kitten, 0, 0)
        
bacon.run(Game())
