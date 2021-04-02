/*
 * macrodevice-libusb.h
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

/// Header guard
#ifndef MACRODEVICE_LIBUSB
#define MACRODEVICE_LIBUSB

#include <vector>
#include <map>
#include <string>
#include <exception>

#include <libusb-1.0/libusb.h>

#include "helpers.h"

namespace macrodevice
{
	class device_libusb;
}

/**
 * The class for the libusb backend
 */
class macrodevice::device_libusb
{
	
	private:
		
		libusb_device_handle *m_device = NULL;
		
		bool m_detached_kernel_driver = false;
		
		int m_vid = 0, m_pid = 0;
		
		// open usb device with bus id and device address ?
		bool m_use_bus_device = false;
		int m_bus_id = 0, m_device_address = 0;
		
	public:
		
		/**
		 * Loads the device settings, e.g. USB VID, USB PID
		 * Valid settings keys are: vid, pid
		 * @param settings A map of settings keys to their values
		 * @return MACRODEVICE_SUCCESS if successful, MACRODEVICE_FAILURE if required settings are missing or invalid
		 */
		int load_settings( const std::map< std::string, std::string > &settings );
		
		/**
		 * Opens the device specified through load_settings
		 * @return MACRODEVICE_SUCCESS if successful, MACRODEVICE_FAILURE if unsuccessful
		 * @see load_settings
		 */
		int open_device();
		
		/**
		 * Closes the device opened by open_device
		 * @return MACRODEVICE_SUCCESS if successful, MACRODEVICE_FAILURE if unsuccessful
		 * @see open_device
		 */
		int close_device();
		
		/**
		 * Waits for an event, i.e. keypress to occur
		 * @param event The received event, typically of size == 2
		 * @return MACRODEVICE_SUCCESS if successful, MACRODEVICE_FAILURE if unsuccessful
		 */
		int wait_for_event( std::vector< std::string > &event );
		
};

#endif
