#!/usr/bin/env python3
from pathlib import Path
import subprocess
root=Path(__file__).resolve().parents[1]
for line in (root/'dependencies.lock').read_text().splitlines():
    sha,rel=line.split()
    actual=subprocess.check_output(['git','-C',str(root/rel),'rev-parse','HEAD'],text=True).strip()
    if actual != sha: raise SystemExit(f'{rel}: expected {sha}, got {actual}')
    dirty=subprocess.check_output(['git','-C',str(root/rel),'status','--porcelain','--untracked-files=no'],text=True)
    if dirty: raise SystemExit(f'{rel}: modified dependency sources')
print('Dependency pins verified')
