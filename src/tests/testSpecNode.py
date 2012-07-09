import unittest

from pynusmv.nusmv.cinit import cinit
from pynusmv.nusmv.cmd import cmd
from pynusmv.nusmv.parser import parser

from pynusmv.node.specnode import SpecNode
from pynusmv.node.specnode import (true as sptrue, false as spfalse, imply, iff,
                                   ex, eg, ef, eu, ew, ax, ag, af, au, aw)

class TestSpecNode(unittest.TestCase):
    
    def setUp(self):
        cinit.NuSMVCore_init_data()
        cinit.NuSMVCore_init(None, 0)
    
        
    def tearDown(self):
        cinit.NuSMVCore_quit()
        
    
    def test_types(self):
        spec = au(ex(sptrue()), ag(spfalse() & sptrue()))
        self.assertEqual(spec.type, parser.AU)
        
        exspec = spec.car
        self.assertEqual(exspec.type, parser.EX)
        self.assertIsNone(exspec.cdr)
        self.assertEqual(exspec.car.type, parser.TRUEEXP)
        
        agspec = spec.cdr
        self.assertEqual(agspec.type, parser.AG)
        self.assertIsNone(agspec.cdr)
        
        andspec = agspec.car
        self.assertEqual(andspec.type, parser.AND)
        self.assertEqual(andspec.car.type, parser.FALSEEXP)
        self.assertEqual(andspec.cdr.type, parser.TRUEEXP)
        
    
    def test_true(self):
        true = sptrue()
        self.assertEqual(true.type, parser.TRUEEXP)
        self.assertIsNone(true.car)
        self.assertIsNone(true.cdr)
        
    
    def test_false(self):
        false = spfalse()
        self.assertEqual(false.type, parser.FALSEEXP)
        self.assertIsNone(false.car)
        self.assertIsNone(false.cdr)
        
        
    def test_not(self):
        notspec = ~(sptrue())
        self.assertEqual(notspec.type, parser.NOT)
        self.assertIsNotNone(notspec.car)
        self.assertIsNone(notspec.cdr)
        
        
    def test_and(self):
        andspec = sptrue() & spfalse()
        self.assertEqual(andspec.type, parser.AND)
        self.assertIsNotNone(andspec.car)
        self.assertIsNotNone(andspec.cdr)
        

    def test_or(self):
        orspec = sptrue() | spfalse()
        self.assertEqual(orspec.type, parser.OR)
        self.assertIsNotNone(orspec.car)
        self.assertIsNotNone(orspec.cdr)
        
        
    def test_imply(self):
        impspec = imply(sptrue(), spfalse())
        self.assertEqual(impspec.type, parser.IMPLIES)
        self.assertIsNotNone(impspec.car)
        self.assertIsNotNone(impspec.cdr)
        
        
    def test_iff(self):
        iffspec = iff(sptrue(), spfalse())
        self.assertEqual(iffspec.type, parser.IFF)
        self.assertIsNotNone(iffspec.car)
        self.assertIsNotNone(iffspec.cdr)
        
        
    def test_ex(self):
        exspec = ex(sptrue())
        self.assertEqual(exspec.type, parser.EX)
        self.assertIsNotNone(exspec.car)
        self.assertIsNone(exspec.cdr)
        

    def test_ef(self):
        efspec = ef(sptrue())
        self.assertEqual(efspec.type, parser.EF)
        self.assertIsNotNone(efspec.car)
        self.assertIsNone(efspec.cdr)
        

    def test_eg(self):
        egspec = eg(sptrue())
        self.assertEqual(egspec.type, parser.EG)
        self.assertIsNotNone(egspec.car)
        self.assertIsNone(egspec.cdr)
        
        
    def test_eu(self):
        euspec = eu(sptrue(), spfalse())
        self.assertEqual(euspec.type, parser.EU)
        self.assertIsNotNone(euspec.car)
        self.assertIsNotNone(euspec.cdr)
        
        
    def test_ew(self):
        ewspec = ew(sptrue(), spfalse())
        self.assertEqual(ewspec.type, parser.EW)
        self.assertIsNotNone(ewspec.car)
        self.assertIsNotNone(ewspec.cdr)
        
        
    def test_ax(self):
        axspec = ax(sptrue())
        self.assertEqual(axspec.type, parser.AX)
        self.assertIsNotNone(axspec.car)
        self.assertIsNone(axspec.cdr)
        

    def test_af(self):
        afspec = af(sptrue())
        self.assertEqual(afspec.type, parser.AF)
        self.assertIsNotNone(afspec.car)
        self.assertIsNone(afspec.cdr)
        

    def test_ag(self):
        agspec = ag(sptrue())
        self.assertEqual(agspec.type, parser.AG)
        self.assertIsNotNone(agspec.car)
        self.assertIsNone(agspec.cdr)
        
        
    def test_au(self):
        auspec = au(sptrue(), spfalse())
        self.assertEqual(auspec.type, parser.AU)
        self.assertIsNotNone(auspec.car)
        self.assertIsNotNone(auspec.cdr)
        
        
    def test_aw(self):
        awspec = aw(sptrue(), spfalse())
        self.assertEqual(awspec.type, parser.AW)
        self.assertIsNotNone(awspec.car)
        self.assertIsNotNone(awspec.cdr)