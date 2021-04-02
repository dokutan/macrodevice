/*
 * macrodevice-libevdev.h
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
#ifndef MACRODEVICE_LIBEVDEV
#define MACRODEVICE_LIBEVDEV

#include <vector>
#include <map>
#include <string>
#include <exception>

#include <sys/types.h> // for open()
#include <sys/stat.h> // for open()
#include <fcntl.h> // for open()
#include <poll.h>

#include <libevdev-1.0/libevdev/libevdev.h>

#include "helpers.h"

namespace macrodevice
{
	class device_libevdev;
}

/**
 * The class for the libevdev backend
 */
class macrodevice::device_libevdev
{
	
	private:
		
		/// path for the eventfile
		std::string m_eventfile_path;
		
		/// file descriptor for the eventfile /dev/input/event*
		int m_filedesc;
		
		/// libevdev device
		struct libevdev *m_device = NULL;
		
		/// grab libevdev device?
		bool m_grab = true;
		
		/// return event codes as numbers instead of names?
		bool m_numbers = false;
		
		struct pollfd m_pollfd[1];

		/// poll timeout
		int m_timeout = -1;

	public:
		
		/**
		 * Loads the device settings, e.g. eventfile
		 * Valid settings keys are: eventfile
		 * @param settings A map of settings keys to their values
		 * @return MACRODEVICE_SUCCESS or MACRODEVICE_FAILURE
		 */
		int load_settings( const std::map< std::string, std::string > &settings );
		
		/**
		 * Opens the device specified through load_settings
		 * @return MACRODEVICE_SUCCESS or MACRODEVICE_FAILURE
		 * @see load_settings
		 */
		int open_device();
		
		/**
		 * Closes the device opened by open_device
		 * @return MACRODEVICE_SUCCESS or MACRODEVICE_FAILURE
		 * @see open_device
		 */
		int close_device();
		
		/**
		 * Waits for an event, i.e. keypress to occur
		 * @param event The received event, typically of size == 3
		 * @return MACRODEVICE_SUCCESS, MACRODEVICE_FAILURE or MACRODEVICE_TIMEOUT
		 */
		int wait_for_event( std::vector< std::string > &event );
		
};

#endif
