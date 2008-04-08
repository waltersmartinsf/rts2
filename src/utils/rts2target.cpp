/* 
 * Abstract target class.
 * Copyright (C) 2008 Petr Kubanek <petr@kubanek.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "rts2target.h"

const char *getEventMaskName (int eventMask)
{
	switch (eventMask)
	{
		case SEND_START_OBS:
			return "START_OBS";
		case SEND_ASTRO_OK:
			return "IMAGE_OK";
		case SEND_END_OBS:
			return "END_OBS";
		case SEND_END_PROC:
			return "END_PROC";
		case SEND_END_NIGHT:
			return "END_NIGHT";
	}
	return "UNKNOW";
}


void printEventMask (int eventMask, std::ostream & _os)
{
	bool printed = false;
	if (eventMask & SEND_START_OBS)
	{
		printed = true;
		_os << getEventMaskName (SEND_START_OBS);
	}
	if (eventMask & SEND_ASTRO_OK)
	{
		if (printed)
			_os << " ";
		else
			printed = true;
		_os << getEventMaskName (SEND_ASTRO_OK);
	}
	if (eventMask & SEND_END_OBS)
	{
		if (printed)
			_os << " ";
		else
			printed = true;
		_os << getEventMaskName (SEND_END_OBS);
	}
	if (eventMask & SEND_END_PROC)
	{
		if (printed)
			_os << " ";
		else
			printed = true;
		_os << getEventMaskName (SEND_END_PROC);
	}
	if (eventMask & SEND_END_NIGHT)
	{
		if (printed)
			_os << " ";
		else
			printed = true;
		_os << getEventMaskName (SEND_END_NIGHT);
	}
}
