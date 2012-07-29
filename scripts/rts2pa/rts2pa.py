"""Class definitions for rts2pa"""

# (C) 2004-2012, Markus Wildi, wildi.markus@bluewin.ch
#
#   
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2, or (at your option)
#   any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software Foundation,
#   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
#   Or visit http://www.gnu.org/licenses/gpl.html.

__author__ = 'wildi.markus@bluewin.ch'


import sys
import logging
from optparse import OptionParser
import ConfigParser
import string

class Configuration:
    """Configuration for any PAScript"""    
    def __init__(self, cfn=None):
        self.cfn= cfn
        self.values={}
        self.cp = ConfigParser.RawConfigParser()
        self.dcf={}
        self.dcf[('basic', 'CONFIGURATION_FILE')]= '/etc/rts2/rts2pa/rts2pa.cfg'
        self.dcf[('basic', 'BASE_DIRECTORY')]= '/tmp/rts2pa'
        self.dcf[('basic', 'FILE_GLOB')]= '*fits'
        self.dcf[('basic', 'TEMP_DIRECTORY')]= '/tmp/'

        self.dcf[('astrometry', 'ASTROMETRY_PRG')]= 'solve-field'

        self.dcf[('mode', 'mode')]= 'KingA'

        self.dcf[('coordinates', 'ha')]= 0. # hour angle [deg]
        self.dcf[('coordinates', 'pd')]= 1. # polar distance [deg]

        self.cf={}
        for (section, identifier), value in sorted(self.dcf.iteritems()):
            self.cf[(identifier)]= value

        if not os.path.isfile( cfn):
            logging.error('config file: {0} does not exist, exiting'.format(cfn))
            sys.exit(1)
        
        self.readConfiguration()
        #self.dumpConfiguration()
        #self.dumpDefaultConfiguration()

    def dumpDefaultConfiguration(self):
        for (section, identifier), value in sorted(self.dcf.iteritems()):
            print section, '=>', identifier, '=>', value
            if( self.cp.has_section(section)== False):
                self.cp.add_section(section)

            self.cp.set(section, identifier, value)


        #with open( self.configFileName, 'w') as configfile:
        with open( '', 'w') as configfile:
            configfile.write('# 2012-07-27, Markus Wildi\n')
            configfile.write('# default configuration for rts2pa.py\n')
            configfile.write('#\n')
            configfile.write('#\n')
            self.cp.write(configfile)

    def dumpConfiguration(self):
        for identifier,value  in  self.cf.items():
            logging.debug('Configuration.readConfiguration: {0}={1}'.format( identifier, value)) 


    def readConfiguration( self):
        config = ConfigParser.ConfigParser()
        try:
            config.readfp(open(self.cfn))
        except:
            logging.error('Configuration.readConfiguration: config file {0}  has wrong syntax, exiting'.format(self.cfn))
            sys.exit(1)

# over write the defaults
        for (section, identifier), value in self.dcf.iteritems():

            try:
                value = config.get( section, identifier)
            except KeyError:
                logging.error('Configuration.readConfiguration: key error')
            except:
                #logging.info('Configuration.readConfiguration: no section {0}  or identifier {1} in file {2}'.format(section, identifier, self.cfn))
                continue

            # first bool, then int !
            if(isinstance(self.cf[identifier], bool)):
                if( value == 'True'):
                    self.cf[identifier]= True
                else:
                    self.cf[identifier]= False
            elif( isinstance(self.cf[identifier], int)):
                try:
                    self.cf[identifier]= int(value)
                except:
                    logging.error('Configuration.readConfiguration: no int {0}  in section {1}, identifier {2} in file {3}'.format(value, section, identifier, self.cfn))
            elif(isinstance(self.cf[identifier], float)):
                try:
                    self.cf[identifier]= float(value)
                except:
                    logging.error('Configuration.readConfiguration: no float {0}  in section {1}, identifier {2} in file {3}'.format(value, section, identifier, self.cfn)) 

            else:
                try:
                    self.cf[identifier]= value
                except KeyError:
                    logging.error('Configuration.readConfiguration: no key {0}  in section {1},  in file {3}'.format( identifier, section,  self.cfn)) 
                except:
                    logging.error('Configuration.readConfiguration: no string {0}  in section {1}, identifier {2} in file {3}'.format(value, section, identifier, self.cfn)) 

import time
import datetime
import glob
import os

class Environment():
    """Class performing various task on files, e.g. expansion to (full) path"""
    def __init__(self, runTimeConfig=None):
        self.runTimeConfig=runTimeConfig
        self.now= datetime.datetime.now().isoformat()
        self.runTimePath= '{0}/{1}/'.format(self.runTimeConfig.cf['BASE_DIRECTORY'], self.now) 
        os.makedirs( self.runTimePath)

    def prefix( self, fileName):
        return 'rts2pa-' +fileName

    def expandToTmp( self, fileName=None):
        if( os.path.isabs(fileName)):
            return fileName

        fileName= self.runTimeConfig.value('TEMP_DIRECTORY') + '/'+ fileName
        return fileName

#    def defineRunTimePath(self, fileName=None):
#        for root, dirs, names in os.walk(self.runTimeConfig.value('BASE_DIRECTORY')):
#            if( fileName.rstrip() in names):
#                self.runTimePath= root
#                return True
#        else:
#            logging.error('FileOperations.defineRunTimePath: file not found: {0}'.format(fileName))
#
#        return False

#    def expandToRunTimePath(self, pathName=None):
#        if( os.path.isabs(pathName)):
#            self.runDateTime= pathName.split('/')[-3] 
#            return pathName
#        else:
#            fileName= pathName.split('/')[-1]
#            if( self.defineRunTimePath( fileName)):
#                self.runDateTime= self.runTimePath.split('/')[-2]
# 
#                return self.runTimePath + '/' + fileName
#            else:
#                return None




class PAScript:
    """Class for any PA script"""
    def __init__(self, scriptName=None, parser=None):
        self.scriptName= scriptName
        if(parser):
            self.parser=parser
        else:
            self.parser= OptionParser()

# fetch default configuration
        self.parser.add_option('--config',help='configuration file name',metavar='CONFIGFILE', nargs=1, type=str, dest='config', default='/etc/rts2/rts2pa/rts2pa.cfg')
        self.parser.add_option('--loglevel',help='log level: usual levels',metavar='LOG_LEVEL', nargs=1, type=str, dest='level', default='warning')
        self.parser.add_option('--logTo',help='log file: filename or - for stdout',metavar='DESTINATION', nargs=1, type=str, dest='logTo', default='-')
        self.parser.add_option('--verbose',help='verbose output',metavar='', action='store_true', dest='verbose', default=False)
#        self.parser.add_option('--',help='',metavar='', nargs=1, type=str, dest='', default='')

        (options,args) = self.parser.parse_args()

        logformat= '%(asctime)s:%(name)s:%(levelname)s:%(message)s'
        try:
            logging.basicConfig(filename=options.logTo, level=options.level.upper(), format= logformat)
        except:
            print 'no log level {0}, exiting'.format(options.level)
            sys.exit(1)

        if(options.logTo== '-'):
            soh = logging.StreamHandler(sys.stdout)
            soh.setLevel(options.level.upper())
            soh.setFormatter(logging.Formatter(logformat))
            logger = logging.getLogger()
            logger.addHandler(soh)

        self.runTimeConfig= Configuration( cfn=options.config) #default configuration, overwrite default values with the ones form options.config
        self.environmant= Environment(runTimeConfig=self.runTimeConfig)