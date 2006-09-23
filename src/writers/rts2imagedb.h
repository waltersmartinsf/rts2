/**
 * Used for Image-DB interaction.
 *
 * Build on Rts2Image, persistently update image information in
 * database.
 *
 * @author petr
 */

#ifndef __RTS2_IMAGEDB__
#define __RTS2_IMAGEDB__

#include "rts2image.h"
#include "../utilsdb/rts2taruser.h"

// process_bitfield content
#define ASTROMETRY_PROC	0x01
// when & -> image should be in archive, otherwise it's in trash|que
// depending on ASTROMETRY_PROC
#define ASTROMETRY_OK	0x02
#define DARK_OK		0x04
#define FLAT_OK		0x08
// some error durring image operations occured, information in DB is unrealiable
#define IMG_ERR		0x8000

/**
 * Abstract class, representing image in DB.
 *
 * Rts2ImageSkyDb, Rts2ImageFlatDb and Rts2ImageDarkDb inherit from that class.
 */
class Rts2ImageDb:public Rts2Image
{
protected:
  virtual void initDbImage ();
  void reportSqlError (char *msg);

  virtual int updateDB ()
  {
    return -1;
  }

  int getValueInd (char *name, double &value, int &ind, char *comment = NULL);
  int getValueInd (char *name, float &value, int &ind, char *comment = NULL);

public:
  Rts2ImageDb (Rts2Image * in_image);
  Rts2ImageDb (Target * currTarget, Rts2DevClientCamera * camera,
	       const struct timeval *expStart);
  Rts2ImageDb (const char *in_filename);
  Rts2ImageDb (int in_obs_id, int in_img_id);
  Rts2ImageDb (long in_img_date, int in_img_usec, float in_img_exposure);

  int getOKCount ();

  virtual int saveImage ();

  friend std::ostream & operator << (std::ostream & _os,
				     Rts2ImageDb & img_db);
};

class Rts2ImageSkyDb:public Rts2ImageDb
{
private:
  int updateAstrometry ();

  int setDarkFromDb ();

  int processBitfiedl;
  inline int isCalibrationImage ();
  void updateCalibrationDb ();

protected:
    virtual void initDbImage ();
  virtual int updateDB ();

public:
    Rts2ImageSkyDb (Target * currTarget, Rts2DevClientCamera * camera,
		    const struct timeval *expStartd);
    Rts2ImageSkyDb (const char *in_filename);
  //! Construct image from already existed Rts2ImageDb instance
    Rts2ImageSkyDb (Rts2Image * in_image);
  //! Construct image directly from DB (eg. retrieve all missing parameters)
    Rts2ImageSkyDb (int in_obs_id, int in_img_id);
  //! Construcy image from one database row..
    Rts2ImageSkyDb (int in_tar_id, int in_obs_id, int in_img_id,
		    char in_obs_subtype, long in_img_date, int in_img_usec,
		    float in_img_exposure, float in_img_temperature,
		    const char *in_img_filter, float in_img_alt,
		    float in_img_az, const char *in_camera_name,
		    const char *in_mount_name, bool in_delete_flag,
		    int in_process_bitfield, double in_img_err_ra,
		    double in_img_err_dec, double in_img_err,
		    int in_epoch_id);
    virtual ~ Rts2ImageSkyDb (void);

  virtual int toArchive ();
  virtual int toTrash ();

  virtual int saveImage ();
  virtual int deleteImage ();

  virtual bool haveOKAstrometry ()
  {
    return (processBitfiedl & ASTROMETRY_OK);
  }

  virtual bool isProcessed ()
  {
    return (processBitfiedl & ASTROMETRY_PROC);
  }

  virtual void printFileName (std::ostream & _os);

  virtual void getFileName (std::string & out_filename);

  virtual img_type_t getImageType ()
  {
    return IMGTYPE_OBJECT;
  }
};

class Rts2ImageDarkDb:public Rts2ImageDb
{
protected:
  virtual int updateDB ();
public:
    Rts2ImageDarkDb (Rts2Image * in_image);

  virtual void print (std::ostream & _os, int in_flags = 0);

  virtual img_type_t getImageType ()
  {
    return IMGTYPE_DARK;
  }
};

class Rts2ImageFlatDb:public Rts2ImageDb
{
protected:
  virtual int updateDB ();
public:
    Rts2ImageFlatDb (Rts2Image * in_image);

  virtual void print (std::ostream & _os, int in_flags = 0);

  virtual img_type_t getImageType ()
  {
    return IMGTYPE_FLAT;
  }
};

template < class img > img * setValueImageType (img * in_image)
{
  char *imgTypeText = "unknow";
  img *ret_i = NULL;
  // guess image type..
  if (in_image->getShutter () == SHUT_CLOSED)
    {
      ret_i = new Rts2ImageDarkDb (in_image);
      imgTypeText = "dark";
    }
  else if (in_image->getShutter () == SHUT_UNKNOW)
    {
      // that should not happen
      return in_image;
    }
  else if (in_image->getTargetType () == TYPE_FLAT)
    {
      ret_i = new Rts2ImageFlatDb (in_image);
      imgTypeText = "flat";
    }
  else
    {
      ret_i = new Rts2ImageSkyDb (in_image);
      imgTypeText = "object";
    }
  ret_i->setValue ("IMAGETYP", imgTypeText, "IRAF based image type");
  delete in_image;
  return ret_i;
}

Rts2Image *getValueImageType (Rts2Image * in_image);

#endif /* ! __RTS2_IMAGEDB__ */
