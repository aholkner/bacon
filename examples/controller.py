import bacon

kitten = bacon.Image('res/kitten.png')

class Game(bacon.Game):
    controller = None
    def on_tick(self):
        bacon.clear(0, 0, 0, 1)
        if self.controller:
            bacon.translate(bacon.window.width / 2, bacon.window.height / 2)
            bacon.translate(self.controller.left_thumb_x * 100, self.controller.left_thumb_y * 100)
            bacon.rotate(self.controller.right_thumb_x)
            bacon.scale(self.controller.right_thumb_y + 1, self.controller.right_thumb_y + 1)
            bacon.draw_image(kitten, -kitten.width / 2, -kitten.height / 2)

    def on_controller_button(self, controller, button, pressed):
        print('bacon.ControllerButtons.%s on controller %d was %s' % (bacon.ControllerButtons.tostring(button), controller.controller_index, 'pressed' if pressed else 'released'))

    def on_controller_axis(self, controller, axis, value):
        print('bacon.ControllerAxes.%s on controller %d value is now %f' % (bacon.ControllerAxes.tostring(axis), controller.controller_index, value))

    def on_controller_connected(self, controller):
        self.controller = controller
        print('Controller %d connected: %s' % (controller.controller_index, controller.name))

    def on_controller_disconnected(self, controller):
        print('Controller %d disconnected' % (controller.controller_index))
        if self.controller is controller:
            self.controller = None

bacon.run(Game())