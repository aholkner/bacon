import bacon

kitten = bacon.Image('res/kitten.png')
kitten2 = kitten.get_region(50, 50, kitten.width - 50, kitten.height - 50)
kitten3 = kitten2.get_region(50, 50, kitten2.width - 50, kitten2.height - 50)

bacon.window.width = kitten.width
bacon.window.height = kitten.height

class Game(bacon.Game):
    def on_tick(self):
        bacon.clear(0, 0, 0, 1)
        bacon.set_color(1, 0.5, 0.5, 1)
        bacon.draw_image(kitten, 0, 0)
        bacon.set_color(0.5, 1, 0.5, 1)
        bacon.draw_image(kitten2, 50, 50)
        bacon.set_color(0.5, 0.5, 1, 1)
        bacon.draw_image(kitten3, 100, 100)
bacon.run(Game())