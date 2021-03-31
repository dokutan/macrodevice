-- Example config for macrodevice-lua
-- This script can be used to generate the main section for the
-- input_handler function

-- The backend needs to be specified by setting the backend variable
backend = "libusb"

-- The settings table specifies the device, and other settings.
-- The available setting keys depend on the backend, look at documentation/backends.md 
settings = { vid = "046a", pid = "0011", use_bus_device = false, bus = 1, device = 18 }

-- These settings are for the libevdev backend
-- settings = { eventfile = "/dev/input/by-id/u.sb-046a_0011-event-kbd", grab = true, numbers = false }

print( "-- Copy the following output into the input handler function" )
print( "-- Press any keys (on the macrodevice) you want to use" )
print( "-- Press Control+C (on your normal keyboard) to exit\n" )

function print_event( event )
	
	-- Only print key down events for libevdev
	-- Remove if you want all events
	if( backend == "libevdev" and event[3] ~= "1" ) then
		return
	end
	
	io.write( "if " )
	
	for i=1,#event,1 do
	
		io.write( "event["..i.."] == \""..event[i].."\" " )
		
		if i ~= #event then
			io.write( "and " )
		end
		
	end
	
	io.write( "then\n" )
	io.write( "\tos.execute(\"place command here\")\n" )
	io.write( "end\n" )
	
	io.flush()
	
end

macrodevice.open( backend, settings, print_event )
