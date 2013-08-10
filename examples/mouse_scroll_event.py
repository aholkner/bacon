import bacon

kitten = bacon.Image('res/kitten.png')

class Game(bacon.Game):
    offset_x = 0
    offset_y = 0

    def on_tick(self):
        bacon.clear(0, 0, 0, 1)
        bacon.draw_image(kitten, self.offset_x, self.offset_y)
        pass

    def on_mouse_scroll(self, dx, dy):
        self.offset_x += dx
        self.offset_y += dy

bacon.run(Game())