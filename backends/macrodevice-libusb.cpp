/*
 * macrodevice-libusb.cpp
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

#include "macrodevice-libusb.h"

/**
 * @copydoc macrodevice::device_libusb::load_settings
 */
int macrodevice::device_libusb::load_settings( const std::map< std::string, std::string > &settings )
{
	
	// set device vid and pid
	try
	{
		
		// open with usb bus id and device address (optional)
		if( settings.find( "use_bus_device" ) != settings.end() )
		{
			m_use_bus_device = macrodevice::string_to_bool( settings.at( "use_bus_device" ), false );
		}
		
		// usb vid and pid (required when m_use_bus_device == false)
		if( !m_use_bus_device )
		{
			m_vid = std::stoi( settings.at("vid"), 0, 16);
			m_pid = std::stoi( settings.at("pid"), 0, 16);
		}
		else
		{
			// usb vid and pid (optional when m_use_bus_device == false)
			if( settings.find( "vid" ) != settings.end() )
			{
				m_vid = std::stoi( settings.at("vid"), 0, 16);
			}
			if( settings.find( "pid" ) != settings.end() )
			{
				m_vid = std::stoi( settings.at("pid"), 0, 16);
			}
		}
		
		// usb bus id and device address (required when m_use_bus_device==true)
		if( m_use_bus_device )
		{
			m_bus_id = std::stoi( settings.at("bus"), 0, 10);
			m_device_address = std::stoi( settings.at("device"), 0, 10);
		}
		else
		{
			// usb bus id and device address (optional when m_use_bus_device==false)
			if( settings.find( "bus" ) != settings.end() )
			{
				m_bus_id = std::stoi( settings.at("bus"), 0, 10);
			}
			if( settings.find( "device" ) != settings.end() )
			{
				m_device_address = std::stoi( settings.at("device"), 0, 10);
			}
		}
		
	}
	catch( std::exception &e )
	{
		return 1;
	}
	
	return 0;
}

/**
 * @copydoc macrodevice::device_libusb::open_device
 */
int macrodevice::device_libusb::open_device()
{
	int res = 0;
	
	// libusb init
	res = libusb_init( NULL );
	if( res < 0 )
	{
		return res;
	}
	
	// open device
	if( m_use_bus_device )
	{
		// open with bus and device
		libusb_device **dev_list; // device list
		ssize_t num_devs = libusb_get_device_list(NULL, &dev_list); //get device list
		
		if( num_devs < 0 )
		{
			return 1;
		}
		
		// iterate over device list
		for( ssize_t i = 0; i < num_devs; i++ )
		{
			// check if correct bus and device
			if( libusb_get_bus_number( dev_list[i] ) == m_bus_id && libusb_get_device_address( dev_list[i] ) == m_device_address )
			{
				// open device
				if( libusb_open( dev_list[i], &m_device ) != 0 )
				{
					return 1;
				}
				else
				{
					break;
				}
			}
		}
		
		//free device list, unreference devices
		libusb_free_device_list( dev_list, 1 );
	}
	else
	{
		// open with vid and pid
		m_device = libusb_open_device_with_vid_pid( NULL, m_vid, m_pid );
		if( !m_device )
		{
			return 1;
		}
	}
	
	// detach kernel driver on interface 0 if active 
	if( libusb_kernel_driver_active( m_device, 0 ) )
	{
		res = libusb_detach_kernel_driver( m_device, 0 );
		
		if( res == 0 )
		{
			m_detached_kernel_driver = true;
		}
		else
		{
			return res;
		}
	}
	
	// claim interface 0
	res = libusb_claim_interface( m_device, 0 );
	if( res != 0 )
	{
		return res;
	}
	
	return 0;
}

/**
 * @copydoc macrodevice::device_libusb::close_device
 */
int macrodevice::device_libusb::close_device()
{
	if( m_device == NULL )
		return 1;
	
	// release interface 0
	libusb_release_interface( m_device, 0 );
	
	// reattach kernel driver for interface 0 if detached previously
	if( m_detached_kernel_driver )
	{
		libusb_attach_kernel_driver( m_device, 0 );
	}
	
	// exit libusb
	libusb_exit( NULL );
	
	return 0;
}

/**
 * @copydoc macrodevice::device_libusb::wait_for_event
 */
int macrodevice::device_libusb::wait_for_event( std::vector< std::string > &event )
{
	uint8_t buffer[8]; // usb data buffer
	unsigned char key_old=0, key_new=0;
	int transferred; // number of bytes transferred
	int func_res = 0; // return value of called functions
	
	while( 1 )
	{
		
		// read from endpoint 1
		func_res = libusb_interrupt_transfer( m_device, 0x81, buffer, 8, &transferred, -1 );
		if( func_res != 0 || transferred == 0 )
		{
			return func_res;
		}
		
		
		key_old = key_new;
		key_new = buffer[2];
		
		// if key is pressed
		if( key_old == 0 && key_new != 0 )
		{
			
			event.clear();
			event.push_back( std::to_string( buffer[0] ) );
			event.push_back( std::to_string( buffer[2] ) );
			break;
		}
		
	}
	
	return 0;
}
