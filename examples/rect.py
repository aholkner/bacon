import bacon

class Game(bacon.Game):
    def on_tick(self):
    	bacon.clear(0, 0, 0, 1)
    	bacon.set_color(1, 0, 0, 1)
        bacon.fill_rect(50, 50, 150, 150)
        bacon.set_color(0, 1, 0, 1)
        bacon.draw_rect(50, 50, 150, 150)
bacon.run(Game())