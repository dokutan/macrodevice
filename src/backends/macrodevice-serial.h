/*
 * macrodevice-serial.h
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
#ifndef MACRODEVICE_SERIAL
#define MACRODEVICE_SERIAL

#include <vector>
#include <map>
#include <string>
#include <exception>

#include <sys/types.h> // for open()
#include <sys/stat.h> // for open()
#include <fcntl.h> // for open()
#include <poll.h>
#include <unistd.h> // for read()

#include "helpers.h"

namespace macrodevice
{
	class device_serial;
}

/**
 * The class for the serial backend
 */
class macrodevice::device_serial
{
	
	private:
		
		/// path for the serial port
		std::string m_port_path;
		
		/// file descriptor for the serial port
		int m_filedesc;
		
		struct pollfd m_pollfd[1];
		
	public:
		
		/**
		 * Loads the device settings, e.g. serial port
		 * Valid settings keys are: port
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
		 * @param event The received event, typically of size == 1
		 * @return MACRODEVICE_SUCCESS if successful, MACRODEVICE_FAILURE if unsuccessful
		 */
		int wait_for_event( std::vector< std::string > &event );
		
};

#endif
