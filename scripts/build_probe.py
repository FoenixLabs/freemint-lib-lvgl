#!/usr/bin/env python3
"""Build a private copy of pinned GEMlib with its upstream make rules."""
import argparse,subprocess,tarfile,io
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--mintlib',type=Path,required=True);p.add_argument('--profile',choices=['mint060','emutos060'],default='mint060');a=p.parse_args()
r=Path(__file__).resolve().parents[1];out=r/'build'/a.profile;out.mkdir(parents=True,exist_ok=True)
subprocess.run(['python3',str(r/'scripts/verify_dependencies.py')],check=True)
gem=out/'gemlib';gem.mkdir(exist_ok=True)
archive=subprocess.check_output(['git','-C',str(r/'third_party/gemlib'),'archive','HEAD'])
with tarfile.open(fileobj=io.BytesIO(archive)) as t:t.extractall(gem,filter='data')
cc='m68k-atari-mint-gcc';mint=a.mintlib.resolve();lib=mint/'build/m68020'
flags=['-m68060','-msoft-float','-O2','-Wall','-Wextra','-isystem',str(mint/'include')]
subprocess.run(['make','-j4','-C',str(gem),'.lib68020-60/libgem.a','CFLAGS_68020-60=-m68060 -msoft-float','CFLAGS=-O2 -fomit-frame-pointer -I'+str(mint/'include')],check=True,stdout=(out/'gemlib-build.log').open('w'),stderr=subprocess.STDOUT)
objects=[]
for src in ['probe','pixels','vdi_native','native_format']:
 obj=out/(src+'.o');subprocess.run([cc]+flags+['-I'+str(gem),'-I'+str(r/'include'),'-c',str(r/'src'/(src+'.c')),'-o',str(obj)],check=True);objects.append(str(obj))
subprocess.run([cc]+flags+['-nostdlib','-Wl,--stack,64k','-Wl,-Map,'+str(out/'LVPROBE.map'),'-o',str(out/'LVPROBE.APP'),str(lib/'crt0.o')]+objects+['-L'+str(gem/'.lib68020-60'),'-L'+str(lib),'-Wl,--start-group','-lgem','-lc','-lgcc','-Wl,--end-group'],check=True)
print(out/'LVPROBE.APP')
