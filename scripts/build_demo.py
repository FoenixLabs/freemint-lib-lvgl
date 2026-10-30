#!/usr/bin/env python3
"""Build the unmodified upstream Widgets demo for GEM or the SDL host preview."""
from pathlib import Path
import argparse,subprocess,concurrent.futures,shlex
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--example',choices=['widgets','hello'],default='widgets')
p.add_argument('--platform',choices=['gem','mcp'],default='gem')
p.add_argument('--sanitize',action='store_true',help='ASan/UBSan for host preview')
p.add_argument('--mintlib',type=Path);p.add_argument('--host',action='store_true')
p.add_argument('--profile',choices=['mint060','emutos060'],default='mint060');a=p.parse_args()
r=Path(__file__).resolve().parents[1];source=r/'third_party/lvgl'
out=r/'build'/('host-demo' if a.host else a.profile+('/widgets' if a.example=='widgets' and a.platform=='gem' else '/'+a.example+'-'+a.platform));out.mkdir(parents=True,exist_ok=True)
subprocess.run(['python3',str(r/'scripts/verify_dependencies.py')],check=True)
if a.host:
 cc='cc';ar='ar';flags=['-DLG_HOST_PREVIEW'];sdl=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2'],text=True))
else:
 if a.mintlib is None:p.error('--mintlib is required for GEM builds')
 cc='m68k-atari-mint-gcc';ar='m68k-atari-mint-ar'
 flags=['-m68060','-msoft-float','-isystem',str(a.mintlib.resolve()/'include')]
 if not (r/'build'/a.profile/'gemlib/.lib68020-60/libgem.a').exists():
  subprocess.run(['python3',str(r/'scripts/build_probe.py'),'--profile',a.profile,'--mintlib',str(a.mintlib)],check=True)
flags+=['-std=gnu99','-O2','-DLV_CONF_INCLUDE_SIMPLE','-DLV_LVGL_H_INCLUDE_SIMPLE','-I'+str(r/'config/demo'),'-I'+str(source),'-I'+str(r/'include')]
if a.example=='widgets':flags+=['-DLG_WIDGETS_DEMO']
if not a.host:flags+=['-I'+str(r/'build'/a.profile/'gemlib')]
if a.sanitize:
 if not a.host:p.error('--sanitize requires --host')
 flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-g']
# Configuration and tool flags are part of the cache identity.
import hashlib
key=hashlib.sha256((' '.join([cc]+flags)).encode()+subprocess.check_output([cc,'--version'])+Path(__file__).read_bytes()+(r/'dependencies.lock').read_bytes()+(r/'config/lv_conf.h').read_bytes()+(r/'config/demo/lv_conf.h').read_bytes()+b''.join(h.read_bytes() for h in sorted((r/'include').glob('*.h')))).hexdigest()
stamp=out/'config.sha256';same=stamp.exists() and stamp.read_text()==key

def compile_one(src):
 rel=src.relative_to(source) if src.is_relative_to(source) else Path('port')/src.name
 obj=out/rel.with_suffix('.o');obj.parent.mkdir(parents=True,exist_ok=True)
 if same and obj.exists() and (a.host or obj.with_suffix('.s').exists()) and obj.stat().st_mtime>src.stat().st_mtime:return str(obj)
 local_flags=flags[:]
 if src.name=='lv_draw_sw_blend_to_rgb888.c':local_flags+=['-Dlv_draw_sw_blend_color_to_rgb888=lg_base_color','-Dlv_draw_sw_blend_image_to_rgb888=lg_base_image']
 if a.host:subprocess.run([cc]+local_flags+['-c',str(src),'-o',str(obj)],check=True)
 else:
  asm=obj.with_suffix('.s');subprocess.run([cc]+local_flags+['-S',str(src),'-o',str(asm)],check=True)
  subprocess.run([cc,'-m68060','-msoft-float','-c',str(asm),'-o',str(obj)],check=True)
 return str(obj)
sources=sorted((source/'src').rglob('*.c'))+sorted((source/'demos/widgets').rglob('*.c'))
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:objects=list(pool.map(compile_one,sources))
library=out/'libwidgets.a'
if library.exists():library.unlink()
subprocess.run([ar,'rcs',str(library)]+objects,check=True)
if a.host:
 binary=out/'lvwidgets';subprocess.run([cc]+flags+[str(r/'src/widgets_host.c'),str(r/'src/pixels.c'),str(r/'src/widgets_policy.c'),str(r/'src/wheel.c'),str(r/'tests/wheel.c'),str(r/'src/render.c'),str(r/'tests/render.c'),str(r/'src/scroll_cache.c'),str(library),'-o',str(binary)]+sdl,check=True)
 # Headless API contract tests use a fake backend, not a desktop platform.
 for test,backend in [('app',''),('mcp','platform_mcp.c')]:
  test_sources=[r/'src/app.c',r/'src/render.c',r/('tests/'+test+'.c')]
  if backend:test_sources.append(r/'src'/backend)
  subprocess.run([cc]+flags+[str(s) for s in test_sources]+[str(library),'-o',str(out/('test-'+test))],check=True)
else:
 port=['app.c','platform_'+a.platform+'.c','render.c']
 if a.platform=='gem':port+=['gem_demo.c','pixels.c','vdi_native.c','native_format.c','wheel.c','scroll_cache.c']
 if a.example=='widgets':port+=['widgets_policy.c']
 objs=[compile_one(r/'src'/n) for n in port]
 app_library=out/'liblvgl-app.a'
 if app_library.exists():app_library.unlink()
 subprocess.run([ar,'rcs',str(app_library)]+objs,check=True)
 main=r/'src/widgets_main.c' if a.example=='widgets' else r/'examples'/(a.example+'.c')
 objs=[compile_one(main),str(app_library)]
 lib=a.mintlib.resolve()/'build/m68020';name={'widgets':'LVWIDGET','hello':'LVHELLO'}[a.example];binary=out/(name+'.APP')
 subprocess.run([cc]+flags+['-nostdlib','-Wl,--stack,128k','-Wl,-Map,'+str(out/(name+'.map')),'-o',str(binary),str(lib/'crt0.o')]+objs+[str(library),'-L'+str(r/'build'/a.profile/'gemlib/.lib68020-60'),'-L'+str(lib),'-Wl,--start-group','-lgem','-lc','-lgcc','-Wl,--end-group'],check=True)
 # Check actual compiler instructions, excluding constant bitmap/font data.
 import re
 bad=[str(s)+': '+line for s in out.rglob('*.s') for line in s.read_text().splitlines() if re.match(r'^\s+f[a-z]+(?:\s|\.)',line)]
 if bad:raise SystemExit('FPU instruction candidates:\n'+'\n'.join(bad))
if not a.host:subprocess.run(['python3',str(r/'scripts/inspect_demo.py'),str(out),'--name',name],check=True)
stamp.write_text(key);print(binary)
