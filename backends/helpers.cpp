/*
 * helpers.cpp
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

#include "helpers.h"

/**
 * @copydoc macrodevice::string_to_bool
 */ 
bool macrodevice::string_to_bool( std::string value, bool fallback )
{
	// convert string to lowercase
	for( auto &i : value ){
		i = std::tolower( i );
	}
	
	if( value == "true" || value == "yes" || value == "1" )
	{
		return true;
	}
	else if( value == "false" || value == "no" || value == "0" )
	{
		return false;
	}
	
	return fallback;
}
