import bacon

class Game(bacon.Game):
    def on_tick(self):
        bacon.clear(0, 0, 0, 1)

        bacon.set_color(0, 1, 0, 1)
        bacon.fill_triangle(0, 0, 0, 100, 100, 0)
        bacon.set_color(1, 0, 0, 1)
        bacon.draw_triangle(0, 0, 0, 100, 100, 0)

        bacon.set_color(0, 1, 0, 1)
        bacon.fill_triangle(100, 100, 150, 100, 200, 300)
        bacon.set_color(1, 0, 0, 1)
        bacon.draw_triangle(100, 100, 150, 100, 200, 300)


bacon.run(Game())