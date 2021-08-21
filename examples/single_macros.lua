-- This example shows how individual keys can be mapped to macros when using the libevdev backend.
-- Run: macrodevice-lua -c single_macros.lua

-- Edit the eventfile to match the correct device.
settings = {
    backend = "libevdev",
    eventfile = "/dev/input/by-id/<replace this>",
    grab = true,
    numbers = false,
    timeout = -1
}

-- Edit this table to configure the macros.
-- Functions get called, strings passed to os.execute().
keymap = {
    ["down:KEY_ESC"] = macrodevice.close, -- request closing all opened devices
    ["down:KEY_A"] = "echo 'pressed a'",
    ["up:KEY_A"] = function() print("released a") end,
}

-- These are the libevdev specific possible actions.
key_actions = {
    ["0"] = "up",
    ["1"] = "down",
    ["2"] = "repeat"
}

-- This function gets called for each event.
function handle_input( event )
	
    if event[1] == "EV_KEY" then
        -- Parse the event.
        action = key_actions[event[3]] or ""
        action_key = action..':'..event[2]

        -- Print the event in a format that can be copied into the keymap table.
        print('    ["'..action_key..'"] = "…",')

        -- Run the associated function.
        if type(keymap[action_key]) == "function" then
            keymap[action_key]()
        elseif type(keymap[action_key]) == "string" then
            os.execute(keymap[action_key])
        end
    end
	
end

-- Open the device.
macrodevice.open( settings, handle_input )

-- After the device has been opened, you can drop the root privileges if they were required.
macrodevice.drop_root( "<your username>" )
