/**
 * Executor client for camera with database backend..
 * Copyright (C) 2005-2007 Petr Kubanek <petr@kubanek.net>
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

#include "execclidb.h"
#include "../rts2fits/imagedb.h"

using namespace rts2script;

DevClientCameraExecDb::DevClientCameraExecDb (Rts2Conn * in_connection):DevClientCameraExec (in_connection)
{
}


DevClientCameraExecDb::~DevClientCameraExecDb (void)
{

}

void DevClientCameraExecDb::exposureStarted ()
{
	if (currentTarget)
		currentTarget->startObservation ();
	DevClientCameraExec::exposureStarted ();
}

rts2image::Image * DevClientCameraExecDb::createImage (const struct timeval *expStart)
{
	imgCount++;
	exposureScript = getScript ();
	if (currentTarget)
		// create image based on target type and shutter state
		return new rts2image::ImageDb (currentTarget, this, expStart);
	logStream (MESSAGE_ERROR)
		<< "Rts2DevClientCameraExec::createImage creating no-target image"
		<< sendLog;
	return NULL;
}

void DevClientCameraExecDb::beforeProcess (rts2image::Image * image)
{
	rts2image::Image *outimg = setImage (image, setValueImageType (image));
	rts2image::img_type_t imageType = outimg->getImageType ();
	// dark images don't need to wait till imgprocess will pick them up for reprocessing
	if (imageType == rts2image::IMGTYPE_DARK)
	{
		outimg->toDark ();
	}
	else if (imageType == rts2image::IMGTYPE_FLAT)
	{
		outimg->toFlat ();
	}
}