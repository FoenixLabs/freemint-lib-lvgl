#!/usr/bin/env python3
"""Inspect decoded executable sections. This flags candidates, not CPU certification."""
import subprocess,sys,re
from pathlib import Path
for name in sys.argv[1:]:
 p=Path(name)
 text=subprocess.check_output(['m68k-atari-mint-objdump','-d',str(p)],text=True)
 (p.with_suffix(p.suffix+'.dis')).write_text(text)
 if p.suffix=='.a':
  # MiNT a.out puts const data in .text; font bitmap bytes decode as FPU.
  # Compiler assembly distinguishes instructions from .byte/.word data.
  assembly=list((p.parent/'lvgl').rglob('*.s'))
  if not assembly: raise SystemExit('Missing compiler assembly; rebuild LVGL')
  bad=[str(src)+': '+line for src in assembly for line in src.read_text().splitlines() if re.match(r'^\s+f[a-z]+(?:\s|\.)',line)]
  if bad: print('\n'.join(bad));sys.exit(1)
  print(f'{p}: compiler assembly has no FPU instructions; data directives excluded')
  continue
 found=[]
 for line in text.splitlines():
  fields=line.split('\t')
  if len(fields)>=3:
   mnemonic=fields[2].strip().split(' ')[0]
   if re.match(r'^f[a-z]',mnemonic):found.append(line)
 if found:
  print(f'{p}: FPU candidates; inspect function boundaries and embedded data:')
  print('\n'.join(found));sys.exit(1)
 print(f'{p}: no decoded FPU mnemonics; review .dis and link map for CPU compatibility')
