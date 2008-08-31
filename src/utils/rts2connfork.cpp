/* 
 * Forking connection handling.
 * Copyright (C) 2003-2007 Petr Kubanek <petr@kubanek.net>
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

#include "rts2connfork.h"

#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

Rts2ConnFork::Rts2ConnFork (Rts2Block * in_master, const char *in_exe,
int in_timeout):
Rts2ConnNoSend (in_master)
{
	childPid = -1;
	sockErr = 0;
	exePath = new char[strlen (in_exe) + 1];
	strcpy (exePath, in_exe);
	if (in_timeout > 0)
	{
		time (&forkedTimeout);
		forkedTimeout += in_timeout;
	}
	else
	{
		forkedTimeout = 0;
	}
}


Rts2ConnFork::~Rts2ConnFork (void)
{
	if (childPid > 0)
		kill (-childPid, SIGINT);
	delete[]exePath;
}


void
Rts2ConnFork::connectionError (int last_data_size)
{
	if (last_data_size < 0 && errno == EAGAIN)
	{
		logStream (MESSAGE_DEBUG) <<
			"Rts2ConnFork::connectionError reported EAGAIN - that should not happen, ignoring"
			<< sendLog;
		return;
	}
	Rts2Conn::connectionError (last_data_size);
}


void
Rts2ConnFork::beforeFork ()
{
}


void
Rts2ConnFork::initFailed ()
{
}


int
Rts2ConnFork::newProcess ()
{
	// create new process with execl..
	// now empty
	return -1;
}


int
Rts2ConnFork::init ()
{
	int ret;
	if (childPid > 0)
	{
		// continue
		kill (childPid, SIGCONT);
		initFailed ();
		return 1;
	}
	int pipeStd[2];
	int pipeErr[2];
	if (pipe (pipeStd) || pipe (pipeErr))
	{
		logStream (MESSAGE_ERROR) <<
			"Rts2ConnImgProcess::run cannot create pipe for process: " <<
			strerror (errno) << sendLog;
		initFailed ();
		return -1;
	}
	// do everything that will be needed to done before forking
	beforeFork ();
	childPid = fork ();
	if (childPid == -1)
	{
		logStream (MESSAGE_ERROR) << "Rts2ConnImgProcess::run cannot fork: " <<
			strerror (errno) << sendLog;
		initFailed ();
		return -1;
	}
	else if (childPid)			 // parent
	{
		fcntl (pipeStd[0], F_SETFL, O_NONBLOCK);
		fcntl (pipeErr[0], F_SETFL, O_NONBLOCK);
		sock = pipeStd[0];
		sockErr = pipeErr[0];
		close (pipeStd[1]);
		close (pipeErr[1]);
		return 0;
	}
	// child
	close (pipeStd[0]);
	close (pipeErr[0]);
	close (1);
	dup (pipeStd[1]);
	close (2);
	dup (pipeErr[1]);
	// close all sockets so when we crash, we don't get any dailing
	// sockets
	master->forkedInstance ();
	// start new group, so SIGTERM to group will kill all children
	setpgrp ();

	ret = newProcess ();
	if (ret)
		logStream (MESSAGE_ERROR) << "Rts2ConnFork::init newProcess return : " <<
			ret << " " << strerror (errno) << sendLog;
	exit (0);
}


void
Rts2ConnFork::stop ()
{
	if (childPid > 0)
		kill (-childPid, SIGSTOP);
}


void
Rts2ConnFork::term ()
{
	if (childPid > 0)
		kill (-childPid, SIGKILL);
	childPid = -1;
}

int
Rts2ConnFork::add (fd_set * readset, fd_set * writeset, fd_set * expset)
{
	if (sockErr > 0)
		FD_SET (sockErr, readset);
	return Rts2Conn::add (readset, writeset, expset);
}

int
Rts2ConnFork::idle ()
{
	if (forkedTimeout > 0)
	{
		time_t now;
		time (&now);
		if (now > forkedTimeout)
		{
			term ();
			endConnection ();
		}
	}
	return 0;
}


void
Rts2ConnFork::childReturned (pid_t in_child_pid)
{
	if (childPid == in_child_pid)
	{
		childEnd ();
		childPid = -1;
		// endConnection will be called after read from pipe fails
	}
}
