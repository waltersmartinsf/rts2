#include "rts2devscript.h"
#include "rts2execcli.h"

Rts2DevScript::Rts2DevScript (Rts2Conn * in_script_connection):Rts2Object ()
{
  currentTarget = NULL;
  nextComd = NULL;
  script = NULL;
  blockMove = 0;
  getObserveStart = 0;
  waitScript = NO_WAIT;
  dont_execute_for = -1;
  script_connection = in_script_connection;
}

Rts2DevScript::~Rts2DevScript (void)
{
  deleteScript ();
}

void
Rts2DevScript::startTarget ()
{
  char scriptBuf[MAX_COMMAND_LENGTH];
  // currentTarget should be nulled when script ends in
  // deleteScript
  if (!currentTarget)
    return;
  currentTarget->getScript (script_connection->getName (), scriptBuf);
  script = new Rts2Script (scriptBuf, script_connection);
  script_connection->getMaster ()->
    postEvent (new Rts2Event (EVENT_SCRIPT_STARTED));
}

void
Rts2DevScript::postEvent (Rts2Event * event)
{
  int sig;
  int acqEnd;
  switch (event->getType ())
    {
    case EVENT_KILL_ALL:
      currentTarget = NULL;
      // stop actual observation..
      blockMove = 0;
      unsetWait ();
      waitScript = NO_WAIT;
      if (script)
	{
	  delete script;
	  script = NULL;
	}
      break;
    case EVENT_SET_TARGET:
      currentTarget = (Target *) event->getArg ();
      if (currentTarget && currentTarget->getTargetID () == dont_execute_for)
	{
	  currentTarget = NULL;
	}
      getObserveStart = 0;
      break;
    case EVENT_LAST_READOUT:
    case EVENT_SCRIPT_ENDED:
      if (!event->getArg () || (getObserveStart && script))
	break;
      // we get new target..handle that same way as EVENT_OBSERVE command,
      // telescope will not move
      currentTarget = (Target *) event->getArg ();
      if (currentTarget && currentTarget->getTargetID () == dont_execute_for)
	{
	  currentTarget = NULL;
	  break;
	}
    case EVENT_OBSERVE:
      if (script)		// we are still observing..we will be called after last command finished
	{
	  getObserveStart = 1;
	  break;
	}
      startTarget ();
      if ((script_connection->
	   getState (0) & (CAM_MASK_EXPOSE | CAM_MASK_DATA |
			   CAM_MASK_READING)) ==
	  (CAM_NOEXPOSURE & CAM_NODATA & CAM_NOTREADING))
	{
	  nextCommand ();
	}
      // otherwise we post command after end of camera readout
      getObserveStart = 0;
      break;
    case EVENT_CLEAR_WAIT:
      clearWait ();
      // in case we have some command pending..send it
      nextCommand ();
      break;
    case EVENT_MOVE_QUESTION:
      if (blockMove)
	{
	  *(int *) event->getArg () = *(int *) event->getArg () + 1;
	}
      break;
    case EVENT_OK_ASTROMETRY:
    case EVENT_NOT_ASTROMETRY:
      if (script)
	{
	  script->postEvent (new Rts2Event (event));
	  if (isWaitMove ())
	    // get a change to process updates..
	    nextCommand ();
	}
      break;
    case EVENT_MIRROR_FINISH:
      if (script && waitScript == WAIT_MIRROR)
	{
	  script->postEvent (new Rts2Event (event));
	  waitScript = NO_WAIT;
	  nextCommand ();
	}
      break;
    case EVENT_ACQUSITION_END:
      if (waitScript != WAIT_SLAVE)
	break;
      waitScript = NO_WAIT;
      acqEnd = *(int *) event->getArg ();
      switch (acqEnd)
	{
	case NEXT_COMMAND_PRECISION_OK:
	  if (currentTarget)
	    currentTarget->acqusitionEnd ();
	  nextCommand ();
	  break;
	case -5:		// failed with script deletion..
	case NEXT_COMMAND_PRECISION_FAILED:
	  if (currentTarget)
	    currentTarget->interupted ();
	  deleteScript ();
	  break;
	}
      break;
    case EVENT_SIGNAL:
      if (!script)
	break;
      sig = *(int *) event->getArg ();
      script->postEvent (new Rts2Event (EVENT_SIGNAL, (void *) &sig));
      if (sig == -1)
	{
	  waitScript = NO_WAIT;
	  nextCommand ();
	}
      break;
    }
  Rts2Object::postEvent (event);
}

void
Rts2DevScript::deleteScript ()
{
  blockMove = 0;
  unsetWait ();
  if (waitScript == WAIT_MASTER)
    {
      int acqRet;
      // should not happen
      acqRet = -5;
      script_connection->getMaster ()->
	postEvent (new Rts2Event (EVENT_ACQUSITION_END, (void *) &acqRet));
    }
  waitScript = NO_WAIT;
  if (script)
    {
      if (currentTarget && script->getExecutedCount () == 0)
	dont_execute_for = currentTarget->getTargetID ();
      delete script;
      script = NULL;
      script_connection->getMaster ()->
	postEvent (new Rts2Event (EVENT_SCRIPT_ENDED));
    }
}

int
Rts2DevScript::nextPreparedCommand ()
{
  int ret;
  if (nextComd)
    return 0;
  ret = getNextCommand ();
  switch (ret)
    {
    case NEXT_COMMAND_WAITING:
      setWaitMove ();
      break;
    case NEXT_COMMAND_CHECK_WAIT:
      unblockWait ();
      setWaitMove ();
      break;
    case NEXT_COMMAND_RESYNC:
      script_connection->getMaster ()->
	postEvent (new Rts2Event (EVENT_TEL_SCRIPT_RESYNC));
      setWaitMove ();
      break;
    case NEXT_COMMAND_PRECISION_OK:
    case NEXT_COMMAND_PRECISION_FAILED:
      clearWait ();		// don't wait for mount move - it will not happen
      script_connection->getMaster ()->
	postEvent (new Rts2Event (EVENT_ACQUSITION_END, (void *) &ret));
      if (ret == NEXT_COMMAND_PRECISION_OK)
	{
	  // there wouldn't be a recursion, as Rts2ScriptElement->nextCommand
	  // should never return NEXT_COMMAND_PRECISION_OK
	  ret = nextPreparedCommand ();
	}
      else
	{
	  ret = NEXT_COMMAND_END_SCRIPT;
	}
      break;
    case NEXT_COMMAND_WAIT_ACQUSITION:
      waitScript = WAIT_SLAVE;
      break;
    case NEXT_COMMAND_ACQUSITION_IMAGE:
      currentTarget->acqusitionStart ();
      script_connection->getMaster ()->
	postEvent (new Rts2Event (EVENT_ACQUIRE_START));
      waitScript = WAIT_MASTER;
      break;
    case NEXT_COMMAND_WAIT_SIGNAL:
      waitScript = WAIT_SIGNAL;
      break;
    case NEXT_COMMAND_WAIT_MIRROR:
      waitScript = WAIT_MIRROR;
      break;
    }
  return ret;
}

int
Rts2DevScript::haveNextCommand ()
{
  int ret;

  if (!script || waitScript == WAIT_SLAVE)	// waiting for script or acqusition
    {
      return 0;
    }
  ret = nextPreparedCommand ();
  if (ret < 0)
    {
      deleteScript ();
      // we don't get new command..delete us and look if there is new
      // target..
      if (!getObserveStart)
	{
	  return 0;
	}
      getObserveStart = 0;
      startTarget ();
      if (!script)
	{
	  return 0;
	}
      ret = nextPreparedCommand ();
      // we don't have any next command:(
      if (ret < 0)
	{
	  deleteScript ();
	  return 0;
	}
    }
  if (isWaitMove () || waitScript == WAIT_SLAVE || waitScript == WAIT_SIGNAL
      || waitScript == WAIT_MIRROR)
    return 0;
  if (!strcmp (cmd_device, "TX"))	// some telescope command..
    {
      script_connection->getMaster ()->
	postEvent (new
		   Rts2Event (EVENT_TEL_SCRIPT_CHANGE, (void *) nextComd));
      // postEvent have to create copy (in case we will serve more devices) .. so we have to delete command
      delete nextComd;
      nextComd = NULL;
      setWaitMove ();
      blockMove = 1;
      return 0;
    }
  if (strcmp (cmd_device, script_connection->getName ()))
    {
      syslog (LOG_ERR,
	      "Rts2DevScript::haveNextCommand cmd_device %s ret %i conn %s",
	      cmd_device, ret, script_connection->getName ());
      return 0;
    }
  return 1;
}
