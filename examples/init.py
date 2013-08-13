import bacon

class Game(bacon.Game):
    def on_init(self):
        print('on_init')

    def on_tick(self):
        bacon.clear(0, 0, 0, 1)
        
bacon.run(Game())
