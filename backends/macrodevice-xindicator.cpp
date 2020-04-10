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

#include "macrodevice-xindicator.h"

/**
 * @copydoc macrodevice::device_xindicator::load_settings
 */
int macrodevice::device_xindicator::load_settings( const std::map< std::string, std::string > &settings )
{
	
	// read settings, this backend has no settings (at the moment)
	
	// this prevents an unused parameter compiler warning
	(void)settings;
	
	return 0;
}

/**
 * @copydoc macrodevice::device_xindicator::open_device
 */
int macrodevice::device_xindicator::open_device()
{
	
	// open X display
	m_display = XOpenDisplay( NULL ); // get display from DISPLAY environmental variable
	
	if( m_display == NULL )
		return 1;
	
	// select to receive indicator state events
	if( XkbSelectEvents( m_display, XkbUseCoreKbd, XkbIndicatorStateNotifyMask, XkbIndicatorStateNotifyMask ) != True )
		return 1;
	
	return 0;
}

/**
 * @copydoc macrodevice::device_xindicator::close_device
 */
int macrodevice::device_xindicator::close_device()
{	
	
	// close X Display
	XCloseDisplay( m_display );
	
	return 0;
}

/**
 * @copydoc macrodevice::device_xindicator::wait_for_event
 */
int macrodevice::device_xindicator::wait_for_event( std::vector< std::string > &event )
{
	
	while( 1 )
	{
		// get next event (this blocks)
		XEvent xevent;
		XNextEvent( m_display, &xevent );
		
		// indicator state event received
		if( xevent.type & XkbIndicatorStateNotify )
		{
			
			// get indicator state
			// Maybe there is a way to get the state from the event, that would be more elegant
			unsigned int state;
			if( XkbGetIndicatorState( m_display, XkbUseCoreKbd, &state ) == Success )
			{
				event.push_back( std::to_string(state) );
				break;
			}
			else
			{
				// could not get indicator state
				return 1;
			}
			
		}
		
	}
	
	return 0;
}
