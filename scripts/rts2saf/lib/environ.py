#!/usr/bin/python
# (C) 2013, Markus Wildi, markus.wildi@bluewin.ch
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
#

__author__ = 'markus.wildi@bluewin.ch'


import time
import datetime
import glob
import os


class Environment():
    """Class performing various task on files, e.g. expansion to (full) path"""
    def __init__(self, debug=None, rt=None, logger=None):
        self.startTime= datetime.datetime.now().isoformat()
        self.debug=debug
        self.rt=rt
        self.logger=logger
        self.runTimePath= '{0}/{1}/'.format(self.rt.cfg['BASE_DIRECTORY'], self.startTime) 
        self.runDateTime=None

    def prefix( self, fileName):
        return 'rts2saf-' +fileName

    def notFits(self, fileName):
        items= fileName.split('/')[-1]
        return items.split('.fits')[0]

    def expandToTmp( self, fileName=None):
        if self.absolutePath(fileName):
            self.logger.error('Environment.expandToTmp: absolute path given: {0}'.format(fileName))
            return fileName

        fileName= self.rt.cfg['TEMP_DIRECTORY'] + '/'+ fileName
        return fileName

    def absolutePath(self, fileName=None):
        if fileName==None:
            self.logger.error('Environment.absolutePath: no file name given')
            
        pLeadingSlash = re.compile( r'\/.*')
        leadingSlash = pLeadingSlash.match(fileName)
        if leadingSlash:
            return True
        else:
            return False

    def defineRunTimePath(self, fileName=None):
        for root, dirs, names in os.walk(self.rt.value('BASE_DIRECTORY')):
            if fileName.rstrip() in names:
                self.runTimePath= root
                return True
        else:
            self.logger.error('Environment.defineRunTimePath: file not found: {0}'.format(fileName))

        return False

    def expandToRunTimePath(self, pathName=None):
        if self.absolutePath(pathName):
            self.runDateTime= pathName.split('/')[-3] # it is rts2saf, which creates the directory tree
            return pathName
        else:
            fileName= pathName.split('/')[-1]
            if self.defineRunTimePath( fileName):
                self.runDateTime= self.runTimePath.split('/')[-2]
 
                return self.runTimePath + '/' + fileName
            else:
                return None

# ToDo: refactor with expandToSkyList
    def expandToDs9RegionFileName( self, fitsHDU=None):
        if fitsHDU==None:
            self.logger.error('Environment.expandToDs9RegionFileName: no hdu given')
        
        
        items= self.rt.cfg['DS9_REGION_FILE'].split('.')
        # not nice
        names= fitsHDU.fitsFileName.split('/')
        try:
            fileName= self.prefix( items[0] +  '-' + fitsHDU.staticHeaderElements['FILTER'] + '-' + self.startTime + '-' + names[-1] + '-' + str(fitsHDU.variableHeaderElements['FOC_POS']) + '.' + items[1])
        except:
            fileName= self.prefix( items[0] + '-' +   self.startTime + '.' + items[1])
            
        return  self.expandToTmp(fileName)


    def expandToDs9CommandFileName( self, fitsHDU=None):
        if fitsHDU==None:
            self.logger.error('Environment.expandToDs9COmmandFileName: no hdu given')
        
        items= self.rt.cfg['DS9_REGION_FILE'].split('.')
        try:
            fileName= self.prefix( items[0] +  '-' + fitsHDU.staticHeaderElements['FILTER'] + '-' + self.startTime + '.' + items[1] + '.sh')
        except:
            fileName= self.prefix( items[0] + '-' +   self.startTime + '.' + items[1]+ '.sh')
            
        return  self.expandToTmp(fileName)
        
    def expandToAcquisitionBasePath(self, ftwName=None, ftName=None):
        if ftwName== None and ftName==None:
            return self.rt.cfg['BASE_DIRECTORY'] + '/' + self.startTime + '/'  

        elif ftwName== None:
            return self.rt.cfg['BASE_DIRECTORY'] + '/' + self.startTime + '/'  + ftName + '/'

        elif ftName== None:
            return self.rt.cfg['BASE_DIRECTORY'] + '/' + self.startTime + '/'  + ftwName + '/'
        else: 
            return self.rt.cfg['BASE_DIRECTORY'] + '/' + self.startTime + '/' + ftwName + '/' + ftName + '/'
        
    def createAcquisitionBasePath(self, ftwName=None, ftName=None):
        pth= self.expandToAcquisitionBasePath( ftwName=ftwName, ftName=ftName)
        if not os.path.exists(pth):
            try:
                os.makedirs( pth)
            except:
                self.logger.error('Environment.createAcquisitionBasePath failed for {0}'.format(pth))
                return False

        return True
    def setModeExecutable(self, path):
        #mode = os.stat(path)
        os.chmod(path, 0744)


if __name__ == '__main__':

    import argparse
    import sys
    import logging
    import os
    import glob

    try:
        import lib.config as cfgd
    except:
        import config as cfgd
    try:
        import lib.sextract as sx
    except:
        import sextract as sx
    try:
        import lib.log as lg
    except:
        import log as lg



    parser= argparse.ArgumentParser(prog=sys.argv[0], description='rts2asaf analysis')
    parser.add_argument('--debug', dest='debug', action='store_true', default=False, help=': %(default)s,add more output')
    parser.add_argument('--level', dest='level', default='INFO', help=': %(default)s, debug level')
    parser.add_argument('--logfile',dest='logfile', default='/tmp/{0}.log'.format(sys.argv[0]), help=': %(default)s, logfile name')
    parser.add_argument('--toconsole', dest='toconsole', action='store_true', default=False, help=': %(default)s, log to console')
    parser.add_argument('--config', dest='config', action='store', default='./rts2saf-my.cfg', help=': %(default)s, configuration file path')

    args=parser.parse_args()

    lgd= lg.Logger(debug=args.debug, args=args) # if you need to chage the log format do it here
    logger= lgd.logger 

    rt=cfgd.Configuration(logger=logger)
    rt.readConfiguration(fileName=args.config)

    ev= Environment(debug=args.debug, rt=rt, logger=logger)
    print ev.startTime