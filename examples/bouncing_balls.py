import os
import random
import sys

import logging
logging.basicConfig(level=logging.INFO)

import bacon
import random

res = 'res'
try:
    if sys.frozen:
        res = os.path.join(os.path.dirname(sys.executable), 'res')
except AttributeError:
    pass

font = bacon.Font(res + '/DejaVuSans.ttf', 64)
ball_image = bacon.Image(res + '/ball.png')
ball_sound = bacon.Sound(res + '/ball.wav')

music = bacon.Voice(bacon.Sound(res + '/PowerChorus2.ogg', stream=True), loop=True)
music.play()

bacon.window.resizable = True
bacon.window.title = 'Bouncing Balls'

class Ball(object):
    width = ball_image.width
    height = ball_image.height

    def __init__(self):
        self.x = random.random() * (bacon.window.width - self.width)
        self.y = random.random() * (bacon.window.height - self.height)

        self.dx = (random.random() - 0.5) * 1000
        self.dy = (random.random() - 0.5) * 1000

    def update(self, dt):
        if self.x <= 0 or self.x + self.width >= bacon.window.width:
            self.dx *= -1
            self.on_bounce()
        if self.y <= 0 or self.y + self.height >= bacon.window.height:
            self.dy *= -1
            self.on_bounce()
        self.x += self.dx * dt
        self.y += self.dy * dt

        self.x = min(max(self.x, 0), bacon.window.width - self.width)
        self.y = min(max(self.y, 0), bacon.window.height - self.height)

    def on_bounce(self):
        pan = self.x / float(bacon.window.width - self.width) * 2 - 1
        pitch = 0.9 + random.random() * 0.2
        #ball_sound.play(gain=0.1, pan=pan, pitch=pitch)

player_controller = None

class Game(bacon.Game):

    def on_tick(self):
        bacon.clear(0, 0, 0, 0)
        bacon.set_color(1, 1, 1, 1)
        if player_controller and player_controller.get_button_state(1 << 17):
            bacon.draw_string(font, player_controller.manufacturer, (player_controller.left_thumb_x + 1) * bacon.window.width / 2, (player_controller.left_thumb_y + 1) * bacon.window.height / 2)
        for ball in balls:
            ball.update(bacon.timestep)
            bacon.draw_image(ball_image, ball.x, ball.y)

    def on_key(self, key, value):
        if value:
            if key == bacon.Keys.f:
                bacon.window.fullscreen = not bacon.window.fullscreen
            if key == bacon.Keys.right:
                bacon.window.width += 50

    def on_controller_connected(self, controller):
        global player_controller
        player_controller = controller
        print('profile: %s' % controller.profile)
        print('product: %s' % controller.name)
        print('product_id: %d' % controller.product_id)
        print('vendor_id: %d' % controller.vendor_id)

    def on_controller_disconnected(self, controller):
        global player_controller
        player_controller = None

balls = []
for i in range(100):
    balls.append(Ball())

bacon.run(Game())
