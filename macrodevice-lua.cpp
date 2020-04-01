/*
 * macroKeyboard-lua.cpp
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */


// standard libraries
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <exception>

#include <getopt.h> // getopt_long
#include <sys/types.h> // for fork
#include <unistd.h> // for fork

// lua libraries
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}



// backends
//**********************************************************************
#ifdef USE_BACKEND_HIDAPI
#include "backends/macrodevice-hidapi.h"
#endif

#ifdef USE_BACKEND_LIBEVDEV
#include "backends/macrodevice-libevdev.h"
#endif

#ifdef USE_BACKEND_LIBUSB
#include "backends/macrodevice-libusb.h"
#endif

#ifdef USE_BACKEND_SERIAL
#include "backends/macrodevice-serial.h"
#endif

// help message
//**********************************************************************
void print_help()
{
	std::cout << "macrodevice-lua options:\n\n";
	std::cout << "-h --help\tprint this message\n";
	std::cout << "-c --config\tlua file to be loaded (required)\n";
	std::cout << "-f --fork\tfork into the background\n";
	std::cout << "-u --user\tuser id to drop privileges to (requires -g)\n";
	std::cout << "-g --group\tgroup id to drop privileges to (requires -u)\n";
}


// lua stack dump for debugging purposes
//**********************************************************************
void stack_dump( lua_State *l )
{
	std::cout << "------\n";
	for( int i = lua_gettop( l ); i > 0; i-- ){
		
		std::cout << i << "\t" << lua_typename( l, lua_type( l, i ) );
		
		if( lua_isnumber( l, i ) ){
			std::cout << "\t" << lua_tonumber( l, i ) << "\n";
		} else if( lua_isstring( l, i ) ){
			std::cout << "\t" << lua_tostring( l, i ) << "\n";
		} else{
			std::cout << "\n";
		}
		
	}
	std::cout << "------\n";
}


// drop root privileges to a specified user and group id
//**********************************************************************
int drop_root( int uid, int gid )
{
	// important: set gid before uid
	if( getuid() == 0 ) // check if root
	{
		if( setregid( gid, gid ) != 0 ) // set real and effective gid
		{
			return 1;
		}
		if( setreuid( uid, uid ) != 0 ) // set real and effective uid
		{
			return 1;
		}
	}
	else
	{
		// if not root
		return 1;
	}
	
	return 0;
}


// template function to handle all the device management and macro execution 
//**********************************************************************
template< class T > int run_macros( T device, lua_State *L, bool drop_privs, int uid, int gid )
{
	
	// get and convert settings table from lua 
	//******************************************************************
	lua_getglobal( L, "settings" ); // put table on top of the stack
	if( !lua_istable( L, -1 ) )
	{
		std::cerr << "Error in Lua: variable settings must be a table\n";
		lua_remove( L, -1 ); // remove top value from stack
		return 1;
	}
	
	// traverse the table
	std::map< std::string, std::string > settings;
	lua_pushnil( L ); // first key
	
	while( lua_next( L, -2 ) != 0 ) // key at -2, value at -1
	{
		// check if key is string
		if( lua_type( L, -2 ) == LUA_TSTRING )
		{
			if( lua_type( L, -1 ) == LUA_TSTRING ) // is value a string ?
			{
				// add key-value-pair to std::map
				settings.emplace( lua_tostring( L, -2 ), lua_tostring( L, -1 ) );
			}
			else if( lua_type( L, -1 ) == LUA_TBOOLEAN ) // is value a boolean ?
			{
				// add key-value-pair to std::map
				if( lua_toboolean( L, -1 ) == 1 )
				{
					settings.emplace( lua_tostring( L, -2 ), "true" );
				}
				else if( lua_toboolean( L, -1 ) == 0 )
				{
					settings.emplace( lua_tostring( L, -2 ), "false" );
				}
			}
			else if( lua_type( L, -1 ) == LUA_TNUMBER ) // is value a number ?
			{
				// add key-value-pair to std::map
				settings.emplace( lua_tostring( L, -2 ), lua_tostring( L, -1 ) );
			}
		}
		
		lua_remove( L, -1 ); // remove value from stack
	}
	
	lua_remove( L, -1 ); // remove top value from stack ( settings table )
	
	
	
	// pass settings to the device object
	if( device.load_settings( settings ) != 0 ){
		std::cerr << "Error: Invalid settings specified\n";
		return 1;
	}
	
	
	
	// open the device
	if( device.open_device() != 0 ){
		std::cerr << "Error: Could not open the device\n";
		return 1;
	}
	
	// if root: drop root privileges if requested
	if( drop_privs )
	{
		if( drop_root( uid, gid ) != 0 )
		{
			std::cerr << "Error: could not drop privileges\n";
			device.close_device();
			return 1;
		}
	}
	
	// wait for input
	std::vector< std::string > event;
	std::string lua_return;
	while( 1 )
	{
		event.clear();
		if( device.wait_for_event( event ) != 0 )
		{
			std::cerr << "Warning : could not get input event\n";
			continue;
			
			// When suspending, the current read fails, however the program should not quit
			/*device.close_device();
			return 1;*/
		}
		
		
		lua_getglobal( L, "input_handler" ); // load function onto stack
		
		lua_newtable( L ); // create new table at the top of the stack
		for( unsigned int i = 0; i < event.size(); i++ ){
			lua_pushnumber( L, i+1 ); // push table index
			lua_pushstring( L, event.at(i).c_str() ); // push table value
			lua_settable( L, -3 );
		}
		
		// call lua function input_handler
		if( lua_pcall( L, 1, 1, 0 ) != 0 ){
			std::cerr << "An error occured: " << lua_tostring( L, -1 ) << "\n";
			lua_remove( L, -1 );  // remove top value from stack
			device.close_device();
			return 1;
		}
		
		// get return value from lua function inut_hander
		if( lua_isstring( L, -1 ) )
		{
			lua_return = lua_tostring( L, -1 );
			
			//quit if requested by lua
			if( lua_return == "quit" )
			{
				break;
			}
			
		}
		
		// remove function return value from stack
		lua_remove( L, -1 );
		
	}
	
	// close the device
	device.close_device();
	
	return 0;
}


// main function
//**********************************************************************
int main( int argc, char *argv[] )
{
	try
	{
		
		// commandline arguments
		//******************************************************************
		
		//command line options
		static struct option long_options[] = 
		{
			{"help", no_argument, 0, 'h'},
			{"config", required_argument, 0, 'c'},
			{"fork", no_argument, 0, 'f'},
			{"user", no_argument, 0, 'u'},
			{"group", no_argument, 0, 'g'},
			{0, 0, 0, 0}
		};
		
		// parse commandline options
		int c, option_index = 0;
		bool flag_fork = false, flag_config = false, flag_user = false, flag_group = false;
		std::string string_config, string_user, string_group;
		
		int target_user = 0, target_group = 0; // uid and gid for privilege dropping
		bool drop_privileges = false;
		
		while( (c = getopt_long( argc, argv, "hc:fu:g:", long_options, &option_index ) ) != -1 )
		{
			switch( c )
			{
				case 'h':
					print_help();
					return 0;
					break;
				case 'c':
					flag_config = true;
					string_config = optarg;
					break;
				case 'f':
					flag_fork = true;
					break;
				case 'u':
					flag_user = true;
					string_user = optarg;
					break;
				case 'g':
					flag_group = true;
					string_group = optarg;
					break;
				case '?':
					return 1;
					break;
				default:
					break;
			}
		}
		
		// is a lua file specified ?
		if( !flag_config )
		{
			std::cerr << "Missing arguments: run " << argv[0] << " -h for help\n";
			return 1;
		}
		
		// are user and group id specified ? → enable privilege dropping
		if( !flag_user != !flag_group )
		{
			std::cerr << "Missing arguments: -u and -g need to be used together\n";
			return 1;
		}
		else if( flag_user && flag_group )
		{
			drop_privileges = true;
			try
			{
				target_user = std::stoi( string_user, 0, 10 );
				target_group = std::stoi( string_group, 0, 10 );
		}
			catch( std::exception &e )
			{
				std::cerr << "Invalid argument: -u and -g require a number\n";
				return 1;
			}
		}
		
		// lua initialisation
		//******************************************************************
		
		lua_State *L = luaL_newstate(); // open lua
		luaL_openlibs( L ); // open lua libraries
		
		// load and run lua file
		if( luaL_loadfile( L, string_config.c_str() ) || lua_pcall( L, 0, 0, 0 ) )
		{
			std::cerr << "Error in Lua: " << lua_tostring( L, -1 ) << "\n";
			lua_remove( L, -1 ); // remove top value from stack
			lua_close( L );
			return 1;
		}
		
		// get backend variable from lua
		lua_getglobal( L, "backend" );
		if( !lua_isstring( L, -1 ) )
		{
			std::cerr << "Error in lua: backend must be a string\n";
			lua_remove( L, -1 ); // remove top value from stack
			lua_close( L ); 
			return 1;
		}
		std::string backend = lua_tostring( L, -1 );
		lua_remove( L, -1 ); // remove top value from stack
		
		
		
		// fork ?
		//******************************************************************
		
		if( flag_fork )
		{
			// create child process
			pid_t process_id = fork();
			
			// close file descriptors
			close(0); // cin
			close(1); // cout
			close(2); // cerr
			
			// quit if not child process
			if( process_id != 0 )
				return 0;
		}
		
		
		
		// determine backend and call run_macros
		//******************************************************************
		
		if( backend == "hidapi" )
		{
			#ifdef USE_BACKEND_HIDAPI
			run_macros<macrodevice::device_hidapi>( macrodevice::device_hidapi(), L, drop_privileges, target_user, target_group );
			#else
			std::cerr << "Error: Backend " << backend << " is not enabled\n";
			#endif
		}
		else if( backend == "libevdev" )
		{
			#ifdef USE_BACKEND_LIBEVDEV
			run_macros<macrodevice::device_libevdev>( macrodevice::device_libevdev(), L, drop_privileges, target_user, target_group );
			#else
			std::cerr << "Error: Backend " << backend << " is not enabled\n";
			#endif
		}
		else if( backend == "libusb" )
		{
			#ifdef USE_BACKEND_LIBUSB
			run_macros<macrodevice::device_libusb>( macrodevice::device_libusb(), L, drop_privileges, target_user, target_group );
			#else
			std::cerr << "Error: Backend " << backend << " is not enabled\n";
			#endif
		}
		else if( backend == "serial" )
		{
			#ifdef USE_BACKEND_SERIAL
			run_macros<macrodevice::device_serial>( macrodevice::device_serial(), L, drop_privileges, target_user, target_group );
			#else
			std::cerr << "Error: Backend " << backend << " is not enabled\n";
			#endif
		}
		else
		{
			std::cerr << "Error: Invalid backend\n";
		}
		
		
		// cleanup
		//**************************************************************
		
		lua_close( L );
		
	}
	catch( std::exception &e ) // excepetion handler
	{
		std::cerr << "Exception caught: " << e.what() << "\n";
		
		// perform cleanup
		
		return 1;
	}
	
	return 0;
}
