#!/usr/bin/env python3
"""Build the pinned, unmodified LVGL software renderer into a static library."""
from pathlib import Path
import argparse,subprocess,concurrent.futures
p=argparse.ArgumentParser();p.add_argument('--mintlib',type=Path,required=True);p.add_argument('--profile',choices=['mint060','emutos060'],default='mint060');a=p.parse_args()
r=Path(__file__).resolve().parents[1];out=r/'build'/a.profile/'lvgl';out.mkdir(parents=True,exist_ok=True)
subprocess.run(['python3',str(r/'scripts/verify_dependencies.py')],check=True)
source=r/'third_party/lvgl';objects=[]
def compile_one(src):
 obj=out/src.relative_to(source).with_suffix('.o');obj.parent.mkdir(parents=True,exist_ok=True)
 subprocess.run(['m68k-atari-mint-gcc','-std=gnu99','-m68060','-msoft-float','-Os','-DLV_CONF_INCLUDE_SIMPLE','-I'+str(r/'config'),'-isystem',str(a.mintlib.resolve()/'include'),'-S',str(src),'-o',str(obj.with_suffix('.s'))],check=True)
 subprocess.run(['m68k-atari-mint-gcc','-m68060','-msoft-float','-c',str(obj.with_suffix('.s')),'-o',str(obj)],check=True)
 return str(obj)
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:objects=list(pool.map(compile_one,sorted((source/'src').rglob('*.c'))))
subprocess.run(['m68k-atari-mint-ar','rcs',str(out.parent/'liblvgl.a')]+objects,check=True)
print(out.parent/'liblvgl.a')
