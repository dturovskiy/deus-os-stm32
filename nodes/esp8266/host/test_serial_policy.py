"""No-USB unit tests for Node Link CH340 serial lifecycle policy."""
from __future__ import annotations
from pathlib import Path
from unittest.mock import patch
import unittest

import serial_policy as policy

class FakeTermios:
    HUPCL=0x400
    TCSANOW=0
    def __init__(self,flags):self.attrs=[0,0,flags,0,0,0,[]];self.sets=[]
    def tcgetattr(self,fd):
        assert fd==9
        return list(self.attrs)
    def tcsetattr(self,fd,how,attrs):
        assert fd==9 and how==self.TCSANOW
        self.attrs=list(attrs);self.sets.append(list(attrs))

class FakeSerial:
    def __init__(self,**kwargs):
        assert kwargs["port"] is None
        assert kwargs["exclusive"] is True
        assert kwargs["baudrate"]==115200
        self.dtr=True;self.rts=True;self.port=None
        self.opened=False;self.closed=False
    def open(self):
        assert not self.dtr and not self.rts
        assert self.port==policy.ESP_CH340_BY_ID
        self.opened=True
    def fileno(self):return 9
    def close(self):self.closed=True

class FakeSerialModule:
    def __init__(self):self.clients=[]
    def Serial(self,**kwargs):
        obj=FakeSerial(**kwargs)
        self.clients.append(obj)
        return obj

class PolicyTests(unittest.TestCase):
    def test_clear_only_hupcl(self):
        api=FakeTermios(0x400|0x80|0x21)
        self.assertTrue(policy.clear_hupcl_on_open_fd(9,termios_api=api))
        self.assertEqual(api.attrs[2],0x80|0x21)
        self.assertEqual(len(api.sets),1)
    def test_idempotent(self):
        api=FakeTermios(0x21)
        self.assertFalse(policy.clear_hupcl_on_open_fd(9,termios_api=api))
        self.assertFalse(api.sets)
    def test_rejected_unapplied_setting(self):
        api=FakeTermios(0x400)
        api.tcsetattr=lambda *args:None
        with self.assertRaisesRegex(policy.SerialPolicyError,"HUPCL_REMAINED_ENABLED"):
            policy.clear_hupcl_on_open_fd(9,termios_api=api)
    def test_physical_open_is_never_implicit(self):
        api=FakeSerialModule()
        with patch.object(policy.Path,"is_symlink",return_value=False):
            with self.assertRaisesRegex(policy.SerialPolicyError,"EXPECTED_CH340"):
                with policy.open_esp_uart(serial_api=api):pass
        self.assertFalse(api.clients)
    def test_open_with_no_control_line_pulse_and_close(self):
        api=FakeSerialModule()
        with patch.object(policy.Path,"is_symlink",return_value=True), \
             patch.object(policy.Path,"exists",return_value=True), \
             patch.object(policy,"clear_hupcl_on_open_fd",return_value=True) as drop:
            with policy.open_esp_uart(serial_api=api) as c:
                self.assertTrue(c.opened)
                self.assertFalse(c.dtr)
                self.assertFalse(c.rts)
                drop.assert_called_once_with(9)
            self.assertTrue(c.closed)
    def test_close_even_if_policy_raises(self):
        api=FakeSerialModule()
        with patch.object(policy.Path,"is_symlink",return_value=True), \
             patch.object(policy.Path,"exists",return_value=True), \
             patch.object(policy,"clear_hupcl_on_open_fd",side_effect=policy.SerialPolicyError("TEST_FAIL")):
            with self.assertRaisesRegex(policy.SerialPolicyError,"TEST_FAIL"):
                with policy.open_esp_uart(serial_api=api):pass
            self.assertTrue(api.clients[0].closed)

if __name__=="__main__":
    unittest.main(verbosity=2)
