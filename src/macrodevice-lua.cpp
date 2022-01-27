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
#include <thread>
#include <mutex>
#include <cstdio>

#include <getopt.h> // getopt_long
#include <sys/types.h> // for fork
#include <unistd.h> // for fork
#include <pwd.h> // for getpwnam

#ifdef __linux__
#include <grp.h> // for setgroups()
#endif

// lua libraries
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "backends/helpers.h"

// version defined in makefile
#ifndef VERSION_STRING
#define VERSION_STRING "undefined"
#endif

// the default path for fennel.lua, formatted for lua package.searchpath
#define FENNEL_PATH "/usr/share/macrodevice/?.lua;"

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

#ifdef USE_BACKEND_XINDICATOR
#include "backends/macrodevice-xindicator.h"
#endif

// help message
//**********************************************************************
const std::string help_message = R"(macrodevice-lua options:

-h --help     print this message
-c --config   lua or fennel file to be loaded (required)
-l --language set the language of the config file ('lua'|'fennel'|'auto')
-f --fork     fork into the background
-a --arg      pass the next argument to Lua

Licensed under the GNU GPL v3 or later
)";


// global variables
//**********************************************************************
/// Threads for event handling, one thread per device
std::vector<std::jthread> device_threads;

/// Used as an identifier for each thread
int thread_number = 0;

/// Mutex for interactions with the Lua state
std::mutex mutex_lua;

/// This mutex gets lock when opening a new device
std::mutex mutex_open_device;

/// Drop root permissions to the given user and group id
int drop_root( uid_t uid, gid_t gid )
{
	// important: set gid before uid
	if( getuid() == 0 ) // check if root
	{
		#ifdef __linux__
		if( setgroups(0, NULL) != 0 ) // drop supplementary groups
		{
			return 2;
		}
		#endif

		if( setgid( gid ) != 0 ) // set gid
		{
			return 1;
		}
		if( setuid( uid ) != 0 ) // set uid
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

// functions
//**********************************************************************
/// Thread function to open a specified device and pass the incoming events to the callback function
template< class T > int run_macros( std::stop_token st, T device, lua_State *L, std::map<std::string, std::string> settings, std::string callback_registry_key )
{
	// pass settings to the device object
	//******************************************************************
	if( device.load_settings( settings ) != 0 ){
		std::cerr << "Error: Invalid settings specified\n";
		return 1;
	}
	
	// open the device
	//******************************************************************
	if( device.open_device() != 0 ){
		std::cerr << "Error: Could not open the device\n";
		return 1;
	}
	
	// wait for input
	//******************************************************************
	std::vector< std::string > event;
	std::string lua_return;

	while( !st.stop_requested() )
	{
		int status = device.wait_for_event( event );

		if( status == MACRODEVICE_FAILURE )
		{
			std::cerr << "Warning : could not get input event\n";
			continue;
		}
		else if( status == MACRODEVICE_TIMEOUT )
		{
			continue;
		}
		else if( status == MACRODEVICE_SUCCESS )
		{
			// process input event
			//******************************************************************
			
			// lock lua mutex
			const std::lock_guard<std::mutex> lock( mutex_lua );
			
			// load callback function onto the stack
			lua_pushstring( L, callback_registry_key.c_str() ); // push key onto the stack
			lua_gettable( L, LUA_REGISTRYINDEX ); // push registry["callback_registry_key"] onto the stack
			
			lua_newtable( L ); // create new table at the top of the stack
			for( unsigned int i = 0; i < event.size(); i++ ){
				lua_pushnumber( L, i+1 ); // push table index
				lua_pushstring( L, event.at(i).c_str() ); // push table value
				lua_settable( L, -3 );
			}
			
			// call lua callback function
			if( lua_pcall( L, 1, 1, 0 ) != 0 ){
				std::cerr << "An error occured: " << lua_tostring( L, -1 ) << "\n";
				lua_remove( L, -1 );  // remove top value from stack
				device.close_device();
				return 1;
			}
			
			// get return value from lua callback function
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
		
	}
	
	// close the device
	//******************************************************************
	device.close_device();
	
	return 0;
}

/// Lua wrapper for drop_root()
int lua_drop_root( lua_State *L )
{
	
	if( lua_gettop( L ) == 2 ) // if two arguments passed: drop_root( uid, gid )
	{
		uid_t uid = luaL_checkinteger( L, 1 );
		gid_t gid = luaL_checkinteger( L, 2 );
		lua_pushinteger( L, drop_root( uid, gid ) );
	}
	else // drop_root( "username" )
	{
		std::string username = luaL_checkstring( L, 1 );
		struct passwd *pw = getpwnam( username.c_str() );

		if( pw == NULL )
			lua_pushinteger( L, 1 );
		else
			lua_pushinteger( L, drop_root( pw->pw_uid, pw->pw_gid ) );
	}
	
	return 1;
}

/// Lua function to open a new device, creates a new thread running run_macros()
int lua_open_device( lua_State *L )
{
	// lock mutex, mutex_lua should be locked every time this function gets called
	const std::lock_guard<std::mutex> lock( mutex_open_device );
	
	std::string backend, registry_key;
	std::map< std::string, std::string > settings;
	bool backend_from_settings = false;
	
	// check arguments
	//******************************************************************
	if( lua_gettop( L ) == 3 ) // open( "backend", {settings}, event_handler() )
	{
		backend = luaL_checkstring( L, 1 );
		luaL_checktype( L, 2, LUA_TTABLE );
		// position 3 in the stack could be a function or a callable table, therefore it doesn't get checked
	}
	else if( lua_gettop( L ) == 2 ) // open( {settings}, event_handler() )
	{
		backend_from_settings = true;
		luaL_checktype( L, 1, LUA_TTABLE );
		// position 2 in the stack could be a function or a callable table, therefore it doesn't get checked
	}
	else
	{
		std::cerr << "Error: Invalid number of arguments to macrodevice.open()\n";
		lua_pushnil( L );
		return 1;
	}

	// store callback function in Lua registry
	//******************************************************************
	registry_key = "macrodevice_callback_" + std::to_string( thread_number );
	thread_number++;
	
	lua_pushstring( L, registry_key.c_str() ); // push key onto the stack
	lua_pushvalue( L, -2 ); // push copy of the callback function onto the stack
	lua_settable( L, LUA_REGISTRYINDEX ); // set registry[key] = callback function
	lua_pop( L, 1 ); // pop callback function from the stack
	
	// parse settings table
	//******************************************************************
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
	lua_remove( L, -1 ); // remove top value from stack (settings table)
	
	// determine backend and create thread
	//******************************************************************
	if( backend_from_settings )
		backend = settings.at("backend");

	lua_pop( L, 1 ); // pop backend from stack
	if( backend == "hidapi" )
	{
		#ifdef USE_BACKEND_HIDAPI
		device_threads.push_back( std::jthread( run_macros<macrodevice::device_hidapi>, macrodevice::device_hidapi(), L, settings, registry_key ) );
		lua_pushinteger( L, device_threads.size()-1 );
		#else
		std::cerr << "Error: Backend " << backend << " is not enabled\n";
		#endif
	}
	else if( backend == "libevdev" )
	{
		#ifdef USE_BACKEND_LIBEVDEV
		device_threads.push_back( std::jthread( run_macros<macrodevice::device_libevdev>, macrodevice::device_libevdev(), L, settings, registry_key ) );
		lua_pushinteger( L, device_threads.size()-1 );
		#else
		std::cerr << "Error: Backend " << backend << " is not enabled\n";
		#endif
	}
	else if( backend == "libusb" )
	{
		#ifdef USE_BACKEND_LIBUSB
		device_threads.push_back( std::jthread( run_macros<macrodevice::device_libusb>, macrodevice::device_libusb(), L, settings, registry_key ) );
		lua_pushinteger( L, device_threads.size()-1 );
		#else
		std::cerr << "Error: Backend " << backend << " is not enabled\n";
		#endif
	}
	else if( backend == "serial" )
	{
		#ifdef USE_BACKEND_SERIAL
		device_threads.push_back( std::jthread( run_macros<macrodevice::device_serial>, macrodevice::device_serial(), L, settings, registry_key ) );
		lua_pushinteger( L, device_threads.size()-1 );
		#else
		std::cerr << "Error: Backend " << backend << " is not enabled\n";
		#endif
	}
	else if( backend == "xindicator" )
	{
		#ifdef USE_BACKEND_XINDICATOR
		device_threads.push_back( std::jthread( run_macros<macrodevice::device_xindicator>, macrodevice::device_xindicator(), L, settings, registry_key ) );
		lua_pushinteger( L, device_threads.size()-1 );
		#else
		std::cerr << "Error: Backend " << backend << " is not enabled\n";
		#endif
	}
	else
	{
		std::cerr << "Error: Invalid backend\n";
		lua_pushnil( L );
	}
	
	return 1;
}

/// Lua function to request closing a single or all device(s)
int lua_close_device( lua_State *L )
{
	// close a single device
	if( lua_gettop( L ) == 1 ){

		// get argument
		size_t id = luaL_checkinteger( L, -1 );
		lua_remove( L, -1 );

		if( id < device_threads.size() )
			device_threads.at(id).request_stop();
	}

	// close all devices
	else if( lua_gettop( L ) == 0 )
	{
		for( auto &t : device_threads )
			t.request_stop();
	}

	else
	{
		std::cerr << "Error: Invalid number of arguments to macrodevice.close()\n";
	}

	return 0;
}

/// Makes the macrodevice table available to the Lua state
inline void lua_register_macrodevice( lua_State *L, std::vector< std::string > &arg )
{
    lua_newtable( L ); // create new table

    lua_pushstring( L, "open" ); // index
    lua_pushcfunction( L, lua_open_device ); // value
    lua_settable( L, -3 ); // table[index] = value, pops index and value

	lua_pushstring( L, "close" ); // index
    lua_pushcfunction( L, lua_close_device ); // value
    lua_settable( L, -3 ); // table[index] = value, pops index and value

    lua_pushstring( L, "drop_root" ); // index
    lua_pushcfunction( L, lua_drop_root ); // value
    lua_settable( L, -3 ); // table[index] = value, pops index and value

    lua_pushstring( L, "version" ); // index
    lua_pushstring( L, VERSION_STRING ); // value
    lua_settable( L, -3 ); // table[index] = value, pops index and value

	lua_pushstring( L, "arg" ); // index
	lua_newtable( L ); // create new table
	for( size_t i = 0; i < arg.size(); i++ )
	{
		lua_pushinteger( L, i+1 ); // index
		lua_pushstring( L, arg.at(i).c_str() ); // value
		lua_settable( L, -3 ); // table[index] = value, pops index and value
	}
	lua_settable( L, -3 ); // name table arg, pops table from stack


    lua_setglobal( L, "macrodevice" ); // name table, pops table from stack
}

// main function
//**********************************************************************
int main( int argc, char *argv[] )
{
	try
	{
		
		// commandline arguments
		//**************************************************************
		static struct option long_options[] = 
		{
			{"help", no_argument, 0, 'h'},
			{"config", required_argument, 0, 'c'},
			{"fork", no_argument, 0, 'f'},
			{"arg", required_argument, 0, 'a'},
			{"language", optional_argument, 0, 'l'},
			{0, 0, 0, 0}
		};
		
		// parse commandline options
		int c, option_index = 0;
		bool flag_fork = false, flag_config = false;
		std::string string_config, string_language = "lua";
		std::vector< std::string > lua_args;
			
		while( (c = getopt_long( argc, argv, "hc:fa:l:", long_options, &option_index ) ) != -1 )
		{
			switch( c )
			{
				case 'h':
					std::cout << help_message;
					return 0;
					break;
				case 'c':
					flag_config = true;
					string_config = optarg;
					break;
				case 'f':
					flag_fork = true;
					break;
				case 'a':
					lua_args.push_back( optarg );
					break;
				case 'l':
					string_language = optarg;
					break;
				case '?':
					return 1;
					break;
				default:
					break;
			}
		}
		
		// is a config file specified ?
		if( !flag_config )
		{
			std::cerr << "Missing argument -c, run " << argv[0] << " -h for help\n";
			return 1;
		}

		// determine the language of the config file
		if( string_language == "auto" )
		{
			if( string_config.ends_with(".lua") )
			{
				string_language = "lua";
			}
			else if( string_config.ends_with(".fnl") )
			{
				string_language = "fennel";
			}
			else
			{
				std::cerr << "Could not identify the config file language, set a language with the -l option.\n";
				return 1;
			}
		}

		if( string_language != "lua" and string_language != "fennel" ){
			std::cerr << "Unknown language: " << string_language << ", supported are 'lua' and 'fennel'.\n";
			return 1;
		}
		
		// fork ?
		//**************************************************************
		if( flag_fork )
		{
			// create child process
			pid_t process_id = fork();
			
			// close file descriptors
			close( fileno(stdin) ); // cin
			close( fileno(stdout) ); // cout
			close( fileno(stdout) ); // cerr
			
			// quit if not child process
			if( process_id != 0 )
				return 0;
		}
		
		// lua initialisation
		//**************************************************************
		lua_State *L = luaL_newstate(); // open lua
		luaL_openlibs( L ); // open lua libraries
		
		
		// make macrodevice functions available to Lua
		lua_register_macrodevice( L, lua_args );
		
		{
			// lock lua mutex
			const std::lock_guard<std::mutex> lock( mutex_lua );
			
			// load and run the config file
			if( string_language == "lua" ){
				if( luaL_loadfile( L, string_config.c_str() ) || lua_pcall( L, 0, 0, 0 ) )
				{
					std::cerr << "Error in Lua: " << lua_tostring( L, -1 ) << "\n";
					lua_remove( L, -1 ); // remove top value from stack
					lua_close( L );
					return 1;
				}
			}
			else if( string_language == "fennel" )
			{
				if( luaL_dostring( L, std::string(
					"package.path = \"" FENNEL_PATH "\"..package.path\n"
					"local fennel = require(\"fennel\")\n"
					"fennel.dofile(\"" +  string_config + "\")\n"
				).c_str()) )
				{
					std::cerr << "Error in Lua: " << lua_tostring( L, -1 ) << "\n";
					lua_remove( L, -1 ); // remove top value from stack
					lua_close( L );
					return 1;
				}
			}
		}

		// wait for all threads to join
		//**************************************************************
		for( auto &t : device_threads )
			t.join();
		
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
