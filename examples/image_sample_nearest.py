import bacon
import math

ball_linear = bacon.Image('res/ball.png')
ball_nearest = bacon.Image('res/ball.png', sample_nearest=True)

bacon.window.width = 512
bacon.window.height = 512

class Game(bacon.Game):
    def on_tick(self):
        bacon.clear(0, 0, 0, 1)
        bacon.draw_image(ball_linear, 0, 0, 256, 256)
        bacon.draw_image(ball_nearest, 256, 0, 512, 256)
bacon.run(Game())