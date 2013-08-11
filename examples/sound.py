import bacon

sound = bacon.Sound('res/ball.wav')

class Game(bacon.Game):
    def on_tick(self):
        bacon.clear(0, 0, 0, 1)

    def on_key(self, key, pressed):
        if pressed:
            sound.play()

bacon.run(Game())