#!/usr/bin/env python3
"""Check compiler instructions and linked external text, excluding LVGL asset data."""
from pathlib import Path
import argparse,re,subprocess
p=argparse.ArgumentParser();p.add_argument('directory',type=Path);a=p.parse_args()
r=a.directory
assembly=list(r.rglob('*.s'))
if not assembly:raise SystemExit('No compiler assembly; build the GEM demo first')
bad=[str(src)+': '+line for src in assembly for line in src.read_text().splitlines() if re.match(r'^\s+f[a-z]+(?:\s|\.)',line)]
# a.out merges const image/font tables into .text. Project code is checked from
# compiler assembly; map ranges identify the separately supplied runtime code.
ranges=[]
for line in (r/'LVWIDGET.map').read_text().splitlines():
 m=re.match(r'\s*\.text\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)\s+(.+)',line)
 if m and 'libwidgets.a(' not in m[3] and '/port/' not in m[3]:
  start,size=int(m[1],16),int(m[2],16)
  if size:ranges.append((start,start+size,m[3]))
if not ranges:raise SystemExit('No external text ranges found in link map')
text=subprocess.check_output(['m68k-atari-mint-objdump','-d',str(r/'LVWIDGET.APP')],text=True)
(r/'LVWIDGET.dis').write_text(text)
for line in text.splitlines():
 fields=line.split('\t')
 if len(fields)>=3 and re.match(r'^f[a-z]',fields[2].strip()):
  address=int(fields[0].strip().rstrip(':'),16)
  for start,end,name in ranges:
   if start<=address<end:bad.append(name+': '+line)
if bad:raise SystemExit('FPU candidates require inspection:\n'+'\n'.join(bad))
print(f'No FPU instructions in {len(assembly)} compiler assembly files or {len(ranges)} linked external text ranges; asset bytes excluded')
