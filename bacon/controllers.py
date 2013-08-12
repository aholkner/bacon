import bacon

# Xbox 360 controller via http://tattiebogle.net OS X driver
bacon.ControllerMapping.register(0x45e, 0x28e, bacon.ControllerMapping(
    buttons=dict(
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
            button17 = 'action_down',
            select = 'action_up',
            back = 'action_left',
            start = 'action_right',
        ),
    axes=dict(
            axis1 = 'right_thumb_x',
            axis2 = 'right_thumb_y',
            right_thumb_y = 'right_trigger',
            right_thumb_x = 'left_trigger',
        ),
    profile='extended'
))