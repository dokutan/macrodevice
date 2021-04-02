/*
 * macrodevice-serial.cpp
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

#include "macrodevice-serial.h"

/**
 * @copydoc macrodevice::device_serial::load_settings
 */
int macrodevice::device_serial::load_settings( const std::map< std::string, std::string > &settings )
{
	
	// read settings
	try
	{
		m_port_path = settings.at( "port" );
		
	}
	catch( std::exception &e )
	{
		return MACRODEVICE_FAILURE;
	}
	
	return MACRODEVICE_SUCCESS;
}

/**
 * @copydoc macrodevice::device_serial::open_device
 */
int macrodevice::device_serial::open_device()
{
	// open eventfile
	m_filedesc = open( m_port_path.c_str(), O_RDONLY|O_NOCTTY|O_SYNC );
	if( m_filedesc < 0 )
	{
		return MACRODEVICE_FAILURE;
	}
	
	// set up polling
	m_pollfd[0].fd = m_filedesc;
	m_pollfd[0].events = POLLIN;
	
	return MACRODEVICE_SUCCESS;
}

/**
 * @copydoc macrodevice::device_serial::close_device
 */
int macrodevice::device_serial::close_device()
{	
	// close the serial port
	close( m_filedesc );
	
	return MACRODEVICE_SUCCESS;
}

/**
 * @copydoc macrodevice::device_serial::wait_for_event
 */
int macrodevice::device_serial::wait_for_event( std::vector< std::string > &event )
{
	
	std::string received_message = "";
	char received_char[1];
	int num_received = 0;
	
	// wait for change in /dev/input/event* (no timeout)
	num_received = read( m_filedesc, received_char, 1 ); 
	if( num_received == 0 ) // read single byte
	{
		// poll if no byte received
		if( poll( m_pollfd, 1, -1 ) < 0 )
		{
			return MACRODEVICE_FAILURE;
		}
	}
	else if( num_received == 1 ) // single byte received
	{
		received_message.push_back( received_char[0] );
	}
	else
	{
		return MACRODEVICE_FAILURE; // read failure
	}
	
	event.clear();
	
	// get whole message
	while( ( num_received = read( m_filedesc, received_char, 1 ) ) == 1 ) // while receiving bytes
	{
		if( received_char[0] == '\n' ) // break if newline is received
		{
			break;
		}
		
		received_message.push_back( received_char[0] );
	}
	
	// if read failure
	if( num_received < 0 )
	{
		return MACRODEVICE_FAILURE;
	}
	
	event.push_back( received_message );
	
	return MACRODEVICE_SUCCESS;
}
