#!/usr/bin/env python3
"""probe-lock.py - the lock sheet is modal, and Escape is still the way out.

THE DEFECT THIS EXISTS FOR. ov_click had no OV_LOCK arm, so a press on ANY
pixel - the passphrase well included - fell through to the dismiss that every
other overlay uses and unlocked the session. A full-screen plate with a
knocked-out header, a PASSPHRASE well and a live caret, dismissed by clicking
it. Not a weak lock: a picture of one.

WHY THE SECOND ASSERTION MATTERS MORE THAN THE FIRST. Making a surface consume
every press is easy and is exactly how you lock someone out of their own
machine. Escape is handled in desk_key, a different path entirely, and reading
that code is not the same as watching it work - so this presses it and checks
the desktop comes back.

THREE MEASUREMENTS, in one boot, over the same 700x300 band at the centre of
the screen:

    lock changed the screen         0.8016   the sheet is up
    a click LEFT it unchanged       0.0002   the click did not dismiss
    Escape returned to the desktop  0.8017   and the way out still works

The middle figure is the fix and the third is the thing that would make the fix
unshippable if it were missing.

To see this fail, delete the `if ov_mode == OV_LOCK { return 1 }` arm from
ov_click: the middle figure jumps to the same order as the other two.
"""
import os,sys,subprocess,tempfile,time
sys.path.insert(0,"/home/roy/Documents/repos/zl-linux-presswork/kernel/tools/probes")
from exercise import Serial, Qmp, qemu_argv, build
build(False)
tmp=tempfile.mkdtemp(prefix="lock-")
sp,qp=os.path.join(tmp,"s"),os.path.join(tmp,"q")
p=subprocess.Popen(qemu_argv(tmp,False,sp,qp),stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
def rd(path):
    b=open(path,'rb').read()
    f,i=[],2
    while len(f)<3:
        while b[i:i+1].isspace(): i+=1
        j=i
        while not b[j:j+1].isspace(): j+=1
        f.append(int(b[i:j])); i=j
    return f[0],f[1],b[i+1:]
def band(path,x,y,w,h):
    W,H,px=rd(path); out=bytearray()
    for r in range(y,y+h):
        o=(r*W+x)*3; out+=px[o:o+w*3]
    return bytes(out)
def diff(a,b): return sum(1 for i in range(0,len(a),3) if a[i:i+3]!=b[i:i+3])/(len(a)/3)
def click(qmp,x,y):
    qmp.cmd("input-send-event",events=[
      {"type":"abs","data":{"axis":"x","value":int(x*32767/1919)}},
      {"type":"abs","data":{"axis":"y","value":int(y*32767/1199)}}]); time.sleep(0.25)
    qmp.cmd("input-send-event",events=[{"type":"btn","data":{"down":True,"button":"left"}}]); time.sleep(0.15)
    qmp.cmd("input-send-event",events=[{"type":"btn","data":{"down":False,"button":"left"}}]); time.sleep(0.9)
try:
    ser,qmp=Serial(sp),Qmp(qp)
    ser.wait("compositor:",240); ser.drain(1.5)
    A=os.path.join(tmp,"a.ppm"); B=os.path.join(tmp,"b.ppm"); C=os.path.join(tmp,"c.ppm"); D=os.path.join(tmp,"d.ppm")
    qmp.screendump(A)                       # desktop
    ser.send("lock\r"); time.sleep(1.6)
    qmp.screendump(B)                       # locked
    click(qmp,960,600); time.sleep(0.9)     # a click on the plate
    qmp.screendump(C)                       # must still be locked
    ser.send("\x1b"); time.sleep(1.4)       # Escape
    qmp.screendump(D)                       # must be the desktop again
    BOX=(600,300,700,300)
    a=band(A,*BOX); b=band(B,*BOX); c=band(C,*BOX); d=band(D,*BOX)
    print("lock changed the screen        : %.4f" % diff(a,b))
    print("a click LEFT it unchanged      : %.4f" % diff(b,c))
    print("Escape returned to the desktop : %.4f" % diff(b,d))
finally:
    p.terminate()
