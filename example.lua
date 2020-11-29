-- Example config for macrodevice-lua

print( "macrodevice version "..macrodevice.version )

-- The backend needs to be specified by setting the backend variable
backend = "libevdev"

-- The settings table specifies the device, and other settings.
-- The available setting keys depend on the backend, look at documentation/backends.md
settings = { eventfile = "/dev/input/by-id/u.sb-046a_0011-event-kbd", grab = true, numbers = false }

-- Whenever an input event occurs, this function gets called.
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

-- Open the device(s)
-- multiple devices (with the same or different backend) can be opened simultaneously.
macrodevice.open( backend, settings, input_handler )

macrodevice.open(
	"libusb",
	{ vid = "046a", pid = "0011", use_bus_device = false, bus = 1, device = 18 },
	function( event )
		print( event[1].."\t"..event[2] )
	end
)


-- After the devices have been opened, you can drop root privileges if required
-- Option 1:
macrodevice.drop_root( "username" )

-- Option 2:
user_id = 1000
group_id = 1000
if macrodevice.drop_root( user_id, group_id ) == 0 then
	print( "success" )
else
	print( "failure" )
end

