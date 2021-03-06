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

import unittest
import os
from rts2saf.config import Configuration 
from rts2saf.data import DataSxtr
from rts2saf.sextract import Sextract
from rts2saf.ds9region import Ds9DisplayThread


import logging
if not os.path.isdir('/tmp/rts2saf_log'):
    os.mkdir('/tmp/rts2saf_log')


logging.basicConfig(filename='/tmp/rts2saf_log/unittest.log', level=logging.INFO, format='%(asctime)s %(levelname)s %(message)s')
logger = logging.getLogger()



# sequence matters
def suite():
    suite = unittest.TestSuite()
    suite.addTest(TestDs9Region('test_displayWithRegion'))

    return suite

#@unittest.skip('class not yet implemented')
class TestDs9Region(unittest.TestCase):

    def tearDown(self):
        pass

    def setUp(self):
        self.rt = Configuration(logger=logger)
        self.rt.readConfiguration(fileName='./rts2saf-no-filter-wheel.cfg')
        self.dSx=Sextract(debug=False, rt=self.rt, logger=logger).sextract(fitsFn='../samples/20071205025911-725-RA.fits')
        print self.dSx.fitsFn

    @unittest.skip('test not yet meaningful')
    def test_displayWithRegion(self):
        logger.info('== {} =='.format(self._testMethodName))
        ds9r=Ds9DisplayThread(dataSxtr=self.dSx, logger=logger)

        self.assertTrue( ds9r, 'return value: '.format(type(DataSxtr)))

if __name__ == '__main__':
    
    unittest.TextTestRunner(verbosity=0).run(suite())
