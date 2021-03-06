/*
 * macrodevice-libevdev.cpp
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

#include "macrodevice-libevdev.h"

/**
 * @copydoc macrodevice::device_libevdev::load_settings
 */
int macrodevice::device_libevdev::load_settings( const std::map< std::string, std::string > &settings )
{
	
	// read settings
	try
	{
		m_eventfile_path = settings.at( "eventfile" );
		
		if( settings.contains( "grab" ) )
		{
			m_grab = macrodevice::string_to_bool( settings.at( "grab" ), true );
		}
		if( settings.contains( "numbers" ) )
		{
			m_numbers = macrodevice::string_to_bool( settings.at( "numbers" ), true );
		}
		if( settings.contains( "timeout" ) )
		{
			m_timeout = std::stoi( settings.at( "timeout" ) );
		}
		
	}
	catch( std::exception &e )
	{
		return MACRODEVICE_FAILURE;
	}
	
	return MACRODEVICE_SUCCESS;
}

/**
 * @copydoc macrodevice::device_libevdev::open_device
 */
int macrodevice::device_libevdev::open_device()
{
	// open eventfile
	m_filedesc = open( m_eventfile_path.c_str(), O_RDONLY|O_NONBLOCK );
	if( m_filedesc < 0 )
	{
		return MACRODEVICE_FAILURE;
	}
	
	// create libevdev device
	if( libevdev_new_from_fd( m_filedesc, &m_device ) < 0 )
	{
		return MACRODEVICE_FAILURE;
	}
	
	// set up polling
	m_pollfd[0].fd = m_filedesc;
	m_pollfd[0].events = POLLIN;
	
	// grab device (no input to other programs)
	if( m_grab )
	{
		if( libevdev_grab( m_device, LIBEVDEV_GRAB ) < 0 )
		{
			return MACRODEVICE_FAILURE;
		}
	}
	
	return MACRODEVICE_SUCCESS;
}

/**
 * @copydoc macrodevice::device_libevdev::close_device
 */
int macrodevice::device_libevdev::close_device()
{	
	// ungrab the device
	libevdev_grab( m_device, LIBEVDEV_UNGRAB );
	
	// free the device
	libevdev_free( m_device );
	
	return MACRODEVICE_SUCCESS;
}

/**
 * @copydoc macrodevice::device_libevdev::wait_for_event
 */
int macrodevice::device_libevdev::wait_for_event( std::vector< std::string > &event )
{
	
	struct input_event libevdev_event;
	
	// wait for change in /dev/input/event* if no events are pending
	if( libevdev_has_event_pending( m_device ) == 0 )
	{
		int p = poll( m_pollfd, 1, m_timeout );
		if( p < 0 )
		{
			return MACRODEVICE_FAILURE;
		}
		else if( p == 0 )
		{
			return MACRODEVICE_TIMEOUT;
		}
	}
	
	event.clear();
	
	// get event
	if( libevdev_next_event( m_device, LIBEVDEV_READ_FLAG_NORMAL, &libevdev_event) == 0 )
	{
		if( m_numbers )
		{
			event.push_back( std::to_string( libevdev_event.type ) );
			event.push_back( std::to_string( libevdev_event.code ) );
			event.push_back( std::to_string( libevdev_event.value ) );
		}
		else
		{
			
			// add event type
			if( libevdev_event_type_get_name( libevdev_event.type ) != NULL )
			{
				event.push_back( libevdev_event_type_get_name( libevdev_event.type ) );
			}
			else
			{
				event.push_back( std::to_string( libevdev_event.type ) );
			}
			
			// add event code
			if( libevdev_event_code_get_name( libevdev_event.type, libevdev_event.code ) != NULL )
			{
				event.push_back( libevdev_event_code_get_name( libevdev_event.type, libevdev_event.code ) );
			}
			else
			{
				event.push_back( std::to_string( libevdev_event.code ) );
			}
			
			// add event value
			if( libevdev_event_value_get_name( libevdev_event.type, libevdev_event.code, libevdev_event.value ) != NULL )
			{
				event.push_back( libevdev_event_value_get_name( libevdev_event.type, libevdev_event.code, libevdev_event.value ) );
			}
			else
			{
				event.push_back( std::to_string( libevdev_event.value ) );
			}
			
		}
		
		return MACRODEVICE_SUCCESS;
	}
	
	return MACRODEVICE_FAILURE;
}
