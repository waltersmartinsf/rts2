#ifndef __RTS2_COMMAND__
#define __RTS2_COMMAND__

#include "rts2block.h"

#define RTS2_COMMAND_KEEP	-2
#define RTS2_COMMAND_REQUE	-5

typedef enum
{ EXP_LIGHT, EXP_DARK } exposureType;

typedef enum
{ NO_COND, NO_EXPOSURE_MOVE, NO_EXPOSURE_NO_MOVE,
  IN_WAIT_STATE, WHILE_EXPOSING
} commandCondType;

/**
 * Base class which represents commands send over network to other component.
 * This object is usually send through Rts2Conn::queCommand to connection,
 * which process it, wait for the other side to reply, pass return code to
 * Rts2Conn::commandReturn callback and delete it.
 *
 * @see Rts2Conn
 *
 * @ingroup RTS2Block
 */
class Rts2Command
{
private:
  int bopMask;
protected:
    Rts2Block * owner;
  Rts2Conn *connection;
  char *text;
  commandCondType commandCond;
public:
    Rts2Command (Rts2Block * in_owner);
    Rts2Command (Rts2Block * in_owner, char *in_text);
    Rts2Command (Rts2Command * in_command)
  {
    owner = in_command->owner;
    connection = in_command->connection;
    setCommand (in_command->getText ());
    commandCond = in_command->getCommandCond ();
    bopMask = 0;
  }
  virtual ~ Rts2Command (void);
  int setCommand (char *in_text);
  void setConnection (Rts2Conn * conn)
  {
    connection = conn;
  }
  Rts2Conn *getConnection ()
  {
    return connection;
  }
  virtual int send ();
  int commandReturn (int status);
  char *getText ()
  {
    return text;
  }
  commandCondType getCommandCond ()
  {
    return commandCond;
  }

  /**
   * Set command Block of OPeration mask.
   *
   * @param in_bopMask New BOP mask.
   */
  void setBopMask (int in_bopMask)
  {
    bopMask = in_bopMask;
  }

  /**
   * Return command BOP mask.
   *
   * @see Rts2Command::setBopMask
   *
   * @return Commmand BOP mask.
   */
  int getBopMask ()
  {
    return bopMask;
  }

  virtual int commandReturnOK ();
  virtual int commandReturnQued ();
  virtual int commandReturnFailed (int status);
};

class Rts2CentraldCommand:public Rts2Command
{

public:
  Rts2CentraldCommand (Rts2Block * in_owner,
		       char *in_text):Rts2Command (in_owner, in_text)
  {
  }
};

class Rts2CommandSendKey:public Rts2Command
{
private:
  int key;
public:
    Rts2CommandSendKey (Rts2Block * in_master, int in_key);
  virtual int send ();

  virtual int commandReturnOK ()
  {
    connection->setConnState (CONN_AUTH_OK);
    return -1;
  }
  virtual int commandReturnFailed (int status)
  {
    connection->setConnState (CONN_AUTH_FAILED);
    return -1;
  }
};

class Rts2CommandAuthorize:public Rts2Command
{
public:
  Rts2CommandAuthorize (Rts2Block * in_master, const char *device_name);
};

// common class for all command, which changed camera settings
// that will call at the end settingsOK or seetingsFailed
class Rts2CommandCameraSettings:public Rts2Command
{
private:
  Rts2DevClientCamera * camera;
public:
  Rts2CommandCameraSettings (Rts2DevClientCamera * in_camera);

  virtual int commandReturnOK ();
  virtual int commandReturnFailed (int status);
};

// devices commands

class Rts2CommandBinning:public Rts2CommandCameraSettings
{
public:
  Rts2CommandBinning (Rts2DevClientCamera * in_camera, int binning_v,
		      int binning_h);
};

class Rts2CommandExposure:public Rts2Command
{
private:
  Rts2DevClientCamera * camera;
public:
  Rts2CommandExposure (Rts2Block * in_master, Rts2DevClientCamera * in_camera,
		       int chip, exposureType exp_type, float exp_time,
		       bool readout = false);
  virtual int commandReturnFailed (int status);
};

class Rts2CommandReadout:public Rts2Command
{
public:
  Rts2CommandReadout (Rts2Block * in_master, int chip);
};

class Rts2CommandFilter:public Rts2Command
{
private:
  Rts2DevClientCamera * camera;
  Rts2DevClientPhot *phot;
  Rts2DevClientFilter *filterCli;
  void setCommandFilter (int filter);
public:
    Rts2CommandFilter (Rts2DevClientCamera * in_camera, int filter);
    Rts2CommandFilter (Rts2DevClientPhot * in_phot, int filter);
    Rts2CommandFilter (Rts2DevClientFilter * in_filter, int filter);

  virtual int commandReturnOK ();
  virtual int commandReturnFailed (int status);
};

class Rts2CommandBox:public Rts2CommandCameraSettings
{
public:
  Rts2CommandBox (Rts2DevClientCamera * in_camera, int chip, int x, int y,
		  int w, int h);
};

class Rts2CommandCenter:public Rts2CommandCameraSettings
{
public:
  Rts2CommandCenter (Rts2DevClientCamera * in_camera, int chip, int width,
		     int height);
};

/**
 * Issue command to change value, but do not send return status.
 */
class Rts2CommandChangeValueDontReturn:public Rts2Command
{
public:
  Rts2CommandChangeValueDontReturn (Rts2DevClient * in_client,
				    std::string in_valName, char op,
				    int in_operand);
    Rts2CommandChangeValueDontReturn (Rts2DevClient * in_client,
				      std::string in_valName, char op,
				      float in_operand);
    Rts2CommandChangeValueDontReturn (Rts2DevClient * in_client,
				      std::string in_valName, char op,
				      double in_operand);
    Rts2CommandChangeValueDontReturn (Rts2DevClient * in_client,
				      std::string in_valName, char op,
				      bool in_operand);
    Rts2CommandChangeValueDontReturn (Rts2DevClient * in_client,
				      std::string in_valName, char op,
				      std::string in_operand,
				      commandCondType in_commandCond =
				      NO_COND);
};

/**
 * Issue command to change value, send return status and handle it.
 */
class Rts2CommandChangeValue:public Rts2CommandChangeValueDontReturn
{
private:
  Rts2DevClient * client;
public:
  Rts2CommandChangeValue (Rts2DevClient * in_client, std::string in_valName,
			  char op, int in_operand);
    Rts2CommandChangeValue (Rts2DevClient * in_client, std::string in_valName,
			    char op, float in_operand);
    Rts2CommandChangeValue (Rts2DevClient * in_client, std::string in_valName,
			    char op, double in_operand);
    Rts2CommandChangeValue (Rts2DevClient * in_client, std::string in_valName,
			    char op, bool in_operand);
    Rts2CommandChangeValue (Rts2DevClient * in_client, std::string in_valName,
			    char op, std::string in_operand,
			    commandCondType in_commandCond = NO_COND);
  virtual int commandReturnOK ();
  virtual int commandReturnFailed (int status);
};

class Rts2CommandMove:public Rts2Command
{
  Rts2DevClientTelescope *tel;
protected:
    Rts2CommandMove (Rts2Block * in_master, Rts2DevClientTelescope * in_tel);
public:
    Rts2CommandMove (Rts2Block * in_master, Rts2DevClientTelescope * in_tel,
		     double ra, double dec);
  virtual int commandReturnFailed (int status);
};

class Rts2CommandMoveUnmodelled:public Rts2CommandMove
{
public:
  Rts2CommandMoveUnmodelled (Rts2Block * in_master,
			     Rts2DevClientTelescope * in_tel, double ra,
			     double dec);
};

class Rts2CommandMoveFixed:public Rts2Command
{
  Rts2DevClientTelescope *tel;
public:
    Rts2CommandMoveFixed (Rts2Block * in_master,
			  Rts2DevClientTelescope * in_tel, double ra,
			  double dec);
  virtual int commandReturnFailed (int status);
};

class Rts2CommandResyncMove:public Rts2Command
{
  Rts2DevClientTelescope *tel;
public:
    Rts2CommandResyncMove (Rts2Block * in_master,
			   Rts2DevClientTelescope * in_tel, double ra,
			   double dec);
  virtual int commandReturnFailed (int status);
};

class Rts2CommandSearch:public Rts2Command
{
  Rts2DevClientTelescope *tel;
public:
    Rts2CommandSearch (Rts2DevClientTelescope * in_tel, double searchRadius,
		       double searchSpeed);
  virtual int commandReturnFailed (int status);
};

class Rts2CommandSearchStop:public Rts2Command
{
public:
  Rts2CommandSearchStop (Rts2DevClientTelescope * in_tel);
};

class Rts2CommandChange:public Rts2Command
{
  Rts2DevClientTelescope *tel;
public:
    Rts2CommandChange (Rts2Block * in_master, double ra, double dec);
    Rts2CommandChange (Rts2DevClientTelescope * in_tel, double ra,
		       double dec);
    Rts2CommandChange (Rts2CommandChange * in_command,
		       Rts2DevClientTelescope * in_tel);
  virtual int commandReturnFailed (int status);
};

class Rts2CommandCorrect:public Rts2Command
{
public:
  Rts2CommandCorrect (Rts2Block * in_master, int corr_mark, double ra,
		      double dec, double ra_err, double dec_err);
};

class Rts2CommandStartGuide:public Rts2Command
{
public:
  Rts2CommandStartGuide (Rts2Block * in_master, char dir, double dir_dist);
};

class Rts2CommandStopGuide:public Rts2Command
{
public:
  Rts2CommandStopGuide (Rts2Block * in_master, char dir);
};

class Rts2CommandStopGuideAll:public Rts2Command
{
public:
  Rts2CommandStopGuideAll (Rts2Block * in_master):Rts2Command (in_master)
  {
    setCommand ("stop_guide_all");
  }
};

class Rts2CommandCupolaMove:public Rts2Command
{
  Rts2DevClientCupola *copula;
public:
    Rts2CommandCupolaMove (Rts2DevClientCupola * in_copula, double ra,
			   double dec);
  virtual int commandReturnFailed (int status);
};

class Rts2CommandChangeFocus:public Rts2Command
{
private:
  Rts2DevClientFocus * focuser;
  Rts2DevClientCamera *camera;
  void change (int in_steps);
public:
    Rts2CommandChangeFocus (Rts2DevClientFocus * in_focuser, int in_steps);
    Rts2CommandChangeFocus (Rts2DevClientCamera * in_camera, int in_steps);
  virtual int commandReturnFailed (int status);
};

class Rts2CommandSetFocus:public Rts2Command
{
private:
  Rts2DevClientFocus * focuser;
  Rts2DevClientCamera *camera;
  void set (int in_steps);
public:
    Rts2CommandSetFocus (Rts2DevClientFocus * in_focuser, int in_steps);
    Rts2CommandSetFocus (Rts2DevClientCamera * in_camera, int in_steps);
  virtual int commandReturnFailed (int status);
};

class Rts2CommandMirror:public Rts2Command
{
private:
  Rts2DevClientMirror * mirror;
public:
  Rts2CommandMirror (Rts2DevClientMirror * in_mirror, int in_pos);
  virtual int commandReturnFailed (int status);
};

class Rts2CommandIntegrate:public Rts2Command
{
private:
  Rts2DevClientPhot * phot;
public:
  Rts2CommandIntegrate (Rts2DevClientPhot * in_phot, int in_filter,
			float in_exp, int in_count);
    Rts2CommandIntegrate (Rts2DevClientPhot * in_phot, float in_exp,
			  int in_count);
  virtual int commandReturnFailed (int status);
};

class Rts2CommandExecNext:public Rts2Command
{
public:
  Rts2CommandExecNext (Rts2Block * in_master, int next_id);
};

class Rts2CommandExecNow:public Rts2Command
{
public:
  Rts2CommandExecNow (Rts2Block * in_master, int now_id);
};

class Rts2CommandExecGrb:public Rts2Command
{
public:
  Rts2CommandExecGrb (Rts2Block * in_master, int grb_id);
};

class Rts2CommandExecShower:public Rts2Command
{
public:
  Rts2CommandExecShower (Rts2Block * in_master);
};

class Rts2CommandKillAll:public Rts2Command
{
public:
  Rts2CommandKillAll (Rts2Block * in_master);
};

class Rts2CommandScriptEnds:public Rts2Command
{
public:
  Rts2CommandScriptEnds (Rts2Block * in_master);
};

class Rts2CommandMessageMask:public Rts2Command
{
public:
  Rts2CommandMessageMask (Rts2Block * in_master, int in_mask);
};

class Rts2CommandInfo:public Rts2Command
{
public:
  Rts2CommandInfo (Rts2Block * in_master);
  virtual int commandReturnOK ();
  virtual int commandReturnFailed (int status);
};

#endif /* !__RTS2_COMMAND__ */
