#!/usr/bin/env python3
from pathlib import Path
import argparse,subprocess
p=argparse.ArgumentParser();p.add_argument('--mintlib',type=Path,required=True);p.add_argument('--profile',default='mint060',choices=['mint060','emutos060']);a=p.parse_args()
r=Path(__file__).resolve().parents[1];out=r/'build'/a.profile;lib=a.mintlib.resolve()/'build/m68020'
subprocess.run(['m68k-atari-mint-gcc','-m68060','-msoft-float','-Os','-DLV_CONF_INCLUDE_SIMPLE','-I'+str(r/'config'),'-I'+str(r/'include'),'-I'+str(r/'third_party/lvgl'),'-isystem',str(a.mintlib.resolve()/'include'),'-nostdlib','-Wl,--stack,64k','-o',str(out/'LVBYTES.TTP'),str(lib/'crt0.o'),str(r/'tests/target_bytes.c'),str(r/'src/pixels.c'),str(out/'liblvgl.a'),'-L'+str(lib),'-Wl,--start-group','-lc','-lgcc','-Wl,--end-group'],check=True)
