import bacon
from bacon import native
from bacon import mouse_input

TouchStates = native.TouchStates

class Touch(object):
    def __init__(self, index):
        self.index = index
        self.pressed = False
        self.x = 0
        self.y = 0
touches = [Touch(i) for i in range(11)]

def _touch_event_handler(touch, state, x, y):
    try:
        touch_object = touches[touch]
    except IndexError:
        return

    was_pressed = touch_object.pressed
    touch_object.pressed = state == native.TouchStates.pressed
    touch_object.x = x
    touch_object.y = y

    game = bacon._current_game
    game.on_touch(touch_object, state)

    print('_touch_event_handler %r %r' % (was_pressed, touch_object.pressed))

    if touch == 0 and game.emulate_mouse:
        mouse_input.mouse.button_mask = (1 << 0) if touch_object.pressed else 0
        mouse_input.mouse.x = x
        mouse_input.mouse.y = y
        if was_pressed != touch_object.pressed:
            game.on_mouse_button(native.MouseButtons.left, touch_object.pressed)