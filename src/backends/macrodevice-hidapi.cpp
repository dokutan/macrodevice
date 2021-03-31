/*
 * macrodevice-hidapi.cpp
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

#include "macrodevice-hidapi.h"

/**
 * @copydoc macrodevice::device_hidapi::load_settings
 */
int macrodevice::device_hidapi::load_settings( const std::map< std::string, std::string > &settings )
{
	
	// set device vid and pid
	try
	{
		m_vid = std::stoi( settings.at("vid"), 0, 16);
		m_pid = std::stoi( settings.at("pid"), 0, 16);
	}
	catch( std::exception &e )
	{
		return 1;
	}
	
	return 0;
}

/**
 * @copydoc macrodevice::device_hidapi::open_device
 */
int macrodevice::device_hidapi::open_device()
{
	
	int res = 0;
	
	// initialize the hidapi library
	res = hid_init();
	
	if( res != 0 )
	{
		return res;
	}
	
	// open the device
	m_device = hid_open( m_vid, m_pid, NULL );
	
	if( !m_device )
	{
		return 1;
	}
	
	
	return 0;
}

/**
 * @copydoc macrodevice::device_hidapi::close_device
 */
int macrodevice::device_hidapi::close_device()
{
	
	// close the hidapi device
	hid_close( m_device );
	
	// close the hidapi library
	hid_exit();
	
	return 0;
}

/**
 * @copydoc macrodevice::device_hidapi::wait_for_event
 */
int macrodevice::device_hidapi::wait_for_event( std::vector< std::string > &event )
{
	
	unsigned char buffer[65]; // for reading and writing to the device
	unsigned char key_old = 0, key_new = 0;
	int res = 0; // return value of this function
	int func_res = 0; // return value from called functions
	
	// run until a keypress occurs
	while( 1 )
	{
		
		
		// request device state
		buffer[1] = 0x81;
		func_res = hid_write( m_device, buffer, 65 );
		
		if( func_res < 0 )
		{
			res = func_res;
			break;
		}
		
		
		// read requested state
		func_res = hid_read( m_device, buffer, 65 );
		
		if( func_res < 0 )
		{
			res = func_res;
			break;
		}
		
		key_old = key_new;
		key_new = buffer[2];
		
		// if key is pressed
		if( key_old == 0 && key_new != 0 )
		{
			// clear event vector
			event.clear();
			
			// add modifier value to event
			event.push_back( std::to_string(buffer[0]) );
			// add key value to event
			event.push_back( std::to_string(key_new) );
			
			break;
		}
		
	}
	
	return res;
}
