import bacon

# Xbox 360 controller via http://tattiebogle.net OS X driver
bacon.ControllerMapping.register(0x45e, 0x28e, bacon.ControllerMapping(
    buttons=dict(
            button3 = 'action_down',
            button4 = 'action_right',
            button5 = 'action_left',
            button6 = 'left_shoulder',
            button7 = 'right_shoulder',
            button8 = 'left_thumb',
            button9 = 'right_thumb',
            button10 = 'start',
            button11 = 'back',
            button13 = 'dpad_up',
            button14 = 'dpad_down',
            button15 = 'dpad_left',
            button16 = 'dpad_right',
            button17 = 'action_up',
        ),
    axes=dict(
            axis1 = 'right_thumb_x',
            axis2 = 'right_thumb_y',
            right_thumb_y = 'right_trigger',
            right_thumb_x = 'left_trigger',
        ),
    dead_zones=dict(
            left_thumb_x = 7849.0 / 65535.0,
            right_thumb_x = 7849.0 / 65535.0,
            left_thumb_y = 8689.0 / 65535.0,
            right_thumb_y = 8689.0 / 65535.0,
            left_trigger = 30.0 / 255.0,
            right_trigger = 30.0 / 255.0
        ),
    profile='extended'
))