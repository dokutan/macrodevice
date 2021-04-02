/*
 * macrodevice-hidapi.h
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
#ifndef MACRODEVICE_HIDAPI
#define MACRODEVICE_HIDAPI

#include <vector>
#include <map>
#include <string>
#include <exception>

#include </usr/include/hidapi/hidapi.h>

#include "helpers.h"

namespace macrodevice
{
	class device_hidapi;
}

/**
 * The class for the hidapi backend
 */
class macrodevice::device_hidapi
{
	
	private:
		
		hid_device *m_device;
		
		int m_vid = 0, m_pid = 0;
		
	public:
		
		/**
		 * Loads the device settings, e.g. USB VID, USB PID
		 * Valid settings keys are: vid, pid
		 * @param settings A map of settings keys to their values
		 * @return 0 if successful, >0 if required settings are missing or invalid
		 */
		int load_settings( const std::map< std::string, std::string > &settings );
		
		/**
		 * Opens the device specified through load_settings
		 * @return 0 if successful, !=0 if unsuccessful
		 * @see load_settings
		 */
		int open_device();
		
		/**
		 * Closes the device opened by open_device
		 * @return 0 if successful, !=0 if unsuccessful
		 * @see open_device
		 */
		int close_device();
		
		/**
		 * Waits for an event, i.e. keypress to occur
		 * @param event The received event, typically of size == 2
		 * @return 0 if successful, !=0 if unsuccessful
		 */
		int wait_for_event( std::vector< std::string > &event );
		
};

#endif
