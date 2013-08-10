import bacon

class Game(bacon.Game):
    def on_tick(self):
        if bacon.Keys.space in bacon.keys:
            bacon.clear(0, 1, 0, 1)
        else:
            bacon.clear(0, 0, 0, 1)

bacon.run(Game())