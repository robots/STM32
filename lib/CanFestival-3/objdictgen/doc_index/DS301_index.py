#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os,wx

DS301_PDF_INDEX = {0x1000: 86, 0x1001: 87, 0x1002: 87, 0x1003: 88, 0x1005: 89, 0x1006: 90, 0x1007: 90, 0x1008: 91, 0x1009: 91, 0x100A: 91, 0x100C: 92, 0x100D: 92, 0x1010: 92, 0x1011: 94, 0x1012: 97, 0x1013: 98, 0x1014: 98, 0x1015: 99, 0x1016: 100, 0x1017: 101, 0x1018: 101, 0x1020: 117, 0x1200: 103, 0x1201: 103, 0x1280: 105, 0x1400: 106, 0x1600: 109, 0x1800: 111, 0x1A00: 112}

def get_acroversion():
    " Return version of Adobe Acrobat executable or None"
    import _winreg
    adobesoft = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, r'Software\Adobe')
    for index in range(_winreg.QueryInfoKey(adobesoft)[0]):
        key = _winreg.EnumKey(adobesoft, index)
        if "acrobat" in key.lower():
            acrokey = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, 'Software\\Adobe\\%s' % key)
            for index in range(_winreg.QueryInfoKey(acrokey)[0]):
                numver = _winreg.EnumKey(acrokey, index)
                try:
                    res = _winreg.QueryValue(_winreg.HKEY_LOCAL_MACHINE, 'Software\\Adobe\\%s\\%s\\InstallPath' % (key, numver))
                    return res
                except:
                    pass
    return None

def OpenPDFDocIndex(index, cwd):
    if not os.path.isfile(os.path.join(cwd, "doc","301_v04000201.pdf")):
        return _("""No documentation file available.
Please read can festival documentation to know how to obtain one.""")
    try:
        if index in DS301_PDF_INDEX:
            if wx.Platform == '__WXMSW__':
                readerpath = get_acroversion()
                readerexepath = os.path.join(readerpath,"AcroRd32.exe")
                if(os.path.isfile(readerexepath)):
                    os.spawnl(os.P_DETACH, readerexepath, "AcroRd32.exe", "/A", "page=%d=OpenActions" % DS301_PDF_INDEX[index], '"%s"'%os.path.join(cwd, "doc","301_v04000201.pdf"))
            else:
                os.system("xpdf -remote DS301 %s %d &"%(os.path.join(cwd, "doc","301_v04000201.pdf"), DS301_PDF_INDEX[index]))
        else:
            if wx.Platform == '__WXMSW__':
                readerpath = get_acroversion()
                readerexepath = os.path.join(readerpath,"AcroRd32.exe")
                if(os.path.isfile(readerexepath)):
                    os.spawnl(os.P_DETACH, readerexepath, "AcroRd32.exe", '"%s"'%os.path.join(cwd, "doc","301_v04000201.pdf"))
            else:
                os.system("xpdf -remote DS301 %s &"%os.path.join(cwd, "doc","301_v04000201.pdf"))
        return True
    except:
        if wx.Platform == '__WXMSW__':
            return _("Check if Acrobat Reader is correctly installed on your computer")
        else:
            return _("Check if xpdf is correctly installed on your computer")
        