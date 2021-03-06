/*
 * helpers.h
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
#ifndef MACRODEVICE_HELPERS
#define MACRODEVICE_HELPERS

#include <string>
#include <locale>

#define MACRODEVICE_SUCCESS 0
#define MACRODEVICE_TIMEOUT -1
#define MACRODEVICE_FAILURE 1

namespace macrodevice
{
	
	/**
	 * \brief Converts a string to a bool
	 * true: "true" "yes" "1"
	 * false: "false" "no" "0"
	 */
	bool string_to_bool( std::string value, bool fallback );
	
}
#endif
