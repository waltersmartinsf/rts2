/* 
 * Class representing FITS file.
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

#include "rts2fitsfile.h"

#include "../utils/rts2app.h"
#include "../utils/utilsfunc.h"

#include "config.h"

#include <string.h>

std::string
Rts2FitsFile::getFitsErrors ()
{
	std::ostringstream os;
	char
		buf[30];
	fits_get_errstatus (fits_status, buf);
	os << " file " << getFileName () << " " << buf;
	return os.str ();
}


void
Rts2FitsFile::setFileName (const char *_fileName)
{
	if (fileName != absoluteFileName)
		delete[] absoluteFileName;
	delete[] fileName;

	if (_fileName == NULL)
	{
		fileName = NULL;
		absoluteFileName = NULL;
		return;
	}

	fileName = new char[strlen (_fileName) + 1];
	strcpy (fileName, _fileName);

	// not an absolute filename..
	if (fileName[0] != '/')
	{
		char *path = get_current_dir_name ();
		absoluteFileName = new char[strlen (path) + strlen(_fileName) + 1];
		strcpy (absoluteFileName, path);
		strcpy (absoluteFileName + strlen (path), fileName);
	}
	else
	{
		absoluteFileName = fileName;
	}
}


int
Rts2FitsFile::createFile ()
{
	fits_status = 0;
	ffile = NULL;

	int ret;
	// make path for us..
	ret = mkpath (getFileName (), 0777);
	if (ret)
		return -1;

	fits_create_file (&ffile, getFileName (), &fits_status);

	if (fits_status)
	{
		logStream (MESSAGE_ERROR) << "Rts2FitsFile::createImage " << getFitsErrors () << sendLog;
		return -1;
	}

	return 0;
}


int
Rts2FitsFile::createFile (const char *_fileName)
{
	setFileName (_fileName);
	return createFile ();
}


int
Rts2FitsFile::createFile (std::string _fileName)
{
	setFileName (_fileName.c_str ());
	return createFile ();
}


int
Rts2FitsFile::openFile (const char *_fileName, bool readOnly)
{
	closeFile ();

	fits_status = 0;

	if (_fileName && _fileName != getFileName ())
		setFileName (_fileName);

	if (getFileName () == NULL)
		return -1;

	#ifdef DEBUG_EXTRA
	logStream (MESSAGE_DEBUG) << "Opening " << getFileName () << " ffile " << getFitsFile () << sendLog;
	#endif						 /* DEBUG_EXTRA */

	fits_open_diskfile (&ffile, getFileName (), readOnly ? READONLY : READWRITE, &fits_status);
	if (fits_status)
	{
		if (!(flags & IMAGE_CANNOT_LOAD))
		{
			logStream (MESSAGE_ERROR) << "openImage: " << getFitsErrors () <<
				sendLog;
			flags |= IMAGE_CANNOT_LOAD;
		}
		return -1;
	}

	return 0;
}


int
Rts2FitsFile::closeFile ()
{
	if ((flags & IMAGE_SAVE) && getFitsFile ())
	{
		fits_close_file (getFitsFile (), &fits_status);
		flags &= ~IMAGE_SAVE;
	}
	else if (getFitsFile ())
	{
		// close ffile to free resources..
		fits_close_file (getFitsFile (), &fits_status);
	}
	setFitsFile (NULL);
	return 0;
}


int
Rts2FitsFile::fitsStatusValue (const char *valname, const char *operation, bool required)
{
	int ret = 0;
	if (fits_status)
	{
		ret = -1;
		if (required)
		{
			logStream (MESSAGE_ERROR) << operation << " value " << valname <<
				" error " << getFitsErrors () << sendLog;
		}
	}
	fits_status = 0;
	return ret;
}


Rts2FitsFile::Rts2FitsFile ():rts2core::Expander ()
{
	ffile = NULL;
	fileName = NULL;
	absoluteFileName = NULL;
	fits_status = 0;
}


Rts2FitsFile::Rts2FitsFile (Rts2FitsFile * _fitsfile):rts2core::Expander (_fitsfile)
{
	fileName = NULL;
	absoluteFileName = NULL;

	setFitsFile (_fitsfile->getFitsFile ());
	_fitsfile->setFitsFile (NULL);

	setFileName (_fitsfile->getFileName ());

	fits_status = _fitsfile->fits_status;
}


Rts2FitsFile::Rts2FitsFile (const char *_fileName):rts2core::Expander ()
{
	fileName = NULL;
	absoluteFileName = NULL;
	fits_status = 0;

	createFile (_fileName);
}


Rts2FitsFile::Rts2FitsFile (const struct timeval *_tv):rts2core::Expander (_tv)
{
	ffile = NULL;
	fileName = NULL;
	absoluteFileName = NULL;
	fits_status = 0;
}


Rts2FitsFile::Rts2FitsFile (const char *_expression, const struct timeval *_tv):rts2core::Expander (_tv)
{
	fileName = NULL;
	absoluteFileName = NULL;
	fits_status = 0;

	createFile (expandPath (_expression));
}


Rts2FitsFile::~Rts2FitsFile (void)
{
	closeFile ();

	if (fileName != absoluteFileName)
		delete[] absoluteFileName;
	delete[] fileName;
}


int
Rts2FitsFile::writeHistory (const char *history)
{
	fits_write_history (ffile, (char *) history, &fits_status);
	return fitsStatusSetValue ("history", true);
}


int
Rts2FitsFile::writeComment (const char *comment)
{
	fits_write_comment (ffile, (char *) comment, &fits_status);
	return fitsStatusSetValue ("comment", true);
}
