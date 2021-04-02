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
		return MACRODEVICE_FAILURE;
	}
	
	return MACRODEVICE_SUCCESS;
}

/**
 * @copydoc macrodevice::device_hidapi::open_device
 */
int macrodevice::device_hidapi::open_device()
{
	
	// initialize the hidapi library
	if( hid_init() != 0 )
	{
		return MACRODEVICE_FAILURE;
	}
	
	// open the device
	m_device = hid_open( m_vid, m_pid, NULL );
	
	if( !m_device )
	{
		return MACRODEVICE_FAILURE;
	}
	
	
	return MACRODEVICE_SUCCESS;
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
	
	return MACRODEVICE_SUCCESS;
}

/**
 * @copydoc macrodevice::device_hidapi::wait_for_event
 */
int macrodevice::device_hidapi::wait_for_event( std::vector< std::string > &event )
{
	
	unsigned char buffer[65]; // for reading and writing to the device
	unsigned char key_old = 0, key_new = 0;
	int res = MACRODEVICE_SUCCESS; // return value of this function
	
	// run until a keypress occurs
	while( 1 )
	{
		
		
		// request device state
		buffer[1] = 0x81;
		if( hid_write( m_device, buffer, 65 ) < 0 )
		{
			res = MACRODEVICE_FAILURE;
			break;
		}
		
		
		// read requested state
		if( hid_read( m_device, buffer, 65 ) < 0 )
		{
			res = MACRODEVICE_FAILURE;
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
