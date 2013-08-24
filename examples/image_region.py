import bacon

kitten = bacon.Image('res/kitten.png')

class Game(bacon.Game):
    def on_tick(self):
        bacon.clear(0, 0, 0, 1)
        x, y = bacon.mouse.x, bacon.mouse.y
        bacon.draw_image_region(kitten, x - 50, y - 50, x + 50, y + 50, 
                                x - 50, y - 50, x + 50, y + 50)
bacon.run(Game())