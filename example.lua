-- Example config for macrodevice-lua

-- The backend needs to be specified by setting the backend variable
backend = "libusb"

-- The settings table specifies the device, and other settings.
-- The available setting keys depend on the backend, look at documentation/backends.md 
settings = { vid = "046a", pid = "0011", use_bus_device = false, bus = 1, device = 18 }

-- These settings are for the libevdev backend
-- settings = { eventfile = "/dev/input/by-id/u.sb-046a_0011-event-kbd", grab = true, numbers = false }


-- Whenever an input event occurs, the function input_handler gets called.
-- As an argument this function takes a table which describes the event.
-- Important: this table always contains strings.
function input_handler( event )
	
	-- Print the event
	if #event == 2 then -- for the libusb and hidapi backends the table size is 2
		print( event[1].."\t"..event[2] )
	end
	if #event == 3 then -- for the libevdev backend the table size is 3
		print( event[1].."\t"..event[2].."\t"..event[3] )
	end
	
	-- execute commands
	if event[2] == 58 then -- this won't work
		os.execute( "mpc toggle" )
	end
	
	if event[2] == "58" then -- this does
		os.execute( "mpc toggle" ) -- F1
	end
	if event[2] == "59" then
		os.execute( "mpc prev" ) -- F2
	end
	if event[2] == "60" then
		os.execute( "mpc next" ) -- F3
	end
	
	-- when this function returns "quit", the program exits
	if event[1] == "2" and event[2] == "41" then
		return "quit" -- shift+esc : quit
	end
	
end
