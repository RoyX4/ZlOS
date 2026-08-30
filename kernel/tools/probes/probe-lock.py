#!/usr/bin/env python3
"""probe-lock.py - the lock sheet is modal, and Escape is still the way out.

THE DEFECT THIS EXISTS FOR. ov_click had no OV_LOCK arm, so a press on ANY
pixel - the passphrase well included - fell through to the dismiss that every
other overlay uses and unlocked the session. A full-screen plate with a
knocked-out header, a PASSPHRASE well and a live caret, dismissed by clicking
it. Not a weak lock: a picture of one.

THE SECOND DEFECT, FOUND BY AUDIT AFTER THE FIRST WAS FIXED. ov_click got its
OV_LOCK arm; desk_key did not. So the plate stopped the MOUSE and forwarded the
KEYBOARD - every printable key fell straight through to the focused window. Type
at the PASSPHRASE well and the shell behind the plate ran what you typed. A lock
that only stops the pointer is a screensaver with a caret.

WHY THE ESCAPE ASSERTION MATTERS MORE THAN EITHER FIX. Making a surface consume
every press is easy and is exactly how you lock someone out of their own
machine. Escape is handled at the TOP of desk_key, above the new lock arm, and
reading that code is not the same as watching it work - so this presses it and
checks the desktop comes back.

THE KEYBOARD LEAK IS NOT VISIBLE ON THE SCREEN, WHICH IS WHY IT SURVIVED. The
plate covers the desktop, so whatever the shell does underneath it changes no
pixel in the band - measured, not assumed: with the desk_key arm deleted, the
"typing LEFT it unchanged" figure below reads the SAME 0.0002 as with it in
place. A pixel probe cannot see behind an opaque overlay.

The console can. This types `help` at the locked plate and counts the bytes the
kernel emits afterwards. Swallowed, the shell says nothing; leaked, it prints
its whole command listing. That count is the assertion for this defect and the
pixel figures are not.

SIX MEASUREMENTS, in one boot; the first four are over the same 700x300 band at
the centre of the screen:

    lock changed the screen         0.8016   the sheet is up
    a click LEFT it unchanged       0.0002   the click did not dismiss
    typing LEFT it unchanged        0.0002   (blind to the leak - see above)
    Escape returned to the desktop  0.8022   the way out still works
    the desktop is the one we left  0.0099   and nothing ran behind the plate
    console bytes while locked      0        THE KEYBOARD ASSERTION

To see the click fix fail, delete the `if ov_mode == OV_LOCK { return 1 }` arm
from ov_click: "a click LEFT it unchanged" jumps to the order of the outer two.
To see the keyboard fix fail, delete the identical arm from desk_key: the byte
count goes from 0 to the length of the help listing, and Escape stops landing
inside this probe's settle time as well.
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
    A=os.path.join(tmp,"a.ppm"); B=os.path.join(tmp,"b.ppm"); C=os.path.join(tmp,"c.ppm"); D=os.path.join(tmp,"d.ppm"); E=os.path.join(tmp,"e.ppm")
    qmp.screendump(A)                       # desktop
    ser.send("lock\r"); time.sleep(1.6)
    qmp.screendump(B)                       # locked
    click(qmp,960,600); time.sleep(0.9)     # a click on the plate
    qmp.screendump(C)                       # must still be locked
    # TYPE A COMMAND AT THE LOCKED PLATE. `help\r` is chosen because it is loud:
    # if these bytes reach the shell underneath, the terminal scrolls a screenful
    # and the desktop cannot come back looking the way it left. The screen is
    # read while STILL LOCKED, so this figure isolates the keyboard from the
    # unlock - a leak that only showed after Escape would be indistinguishable
    # from the redraw.
    ser.drain(0.6); ser.buf = ""            # the console starts here
    for ch in "help\r": ser.send(ch); time.sleep(0.12)
    time.sleep(1.2)
    ser.drain(0.8)
    leaked = len(ser.buf.strip())           # 0 if the plate swallowed them
    qmp.screendump(E)                       # must still be locked, unchanged
    ser.send("\x1b"); time.sleep(1.4)       # Escape
    qmp.screendump(D)                       # must be the desktop again
    # IS THE MACHINE STILL ALIVE? Three identical frames read the same whether
    # the plate is refusing to go away or the guest died holding it, and the
    # figures alone cannot tell those apart. The console can: this drains what
    # the kernel said during the whole sequence and prints its tail, so a panic
    # or a fault appears as text instead of as a suspiciously steady 0.0002.
    ser.drain(1.0)
    tail = [l for l in ser.buf.split("\n") if l.strip()][-3:]
    BOX=(600,300,700,300)
    a=band(A,*BOX); b=band(B,*BOX); c=band(C,*BOX); d=band(D,*BOX); e=band(E,*BOX)
    print("lock changed the screen        : %.4f" % diff(a,b))
    print("a click LEFT it unchanged      : %.4f" % diff(b,c))
    print("typing LEFT it unchanged       : %.4f" % diff(c,e))
    print("Escape returned to the desktop : %.4f" % diff(b,d))
    # AND THE DESKTOP UNDERNEATH IS THE ONE WE LEFT. The figure above only says
    # the plate went away; this one says nothing ran behind it while it was up.
    print("the desktop is the one we left : %.4f" % diff(a,d))
    # THE ONE THAT ACTUALLY CATCHES THE KEYBOARD LEAK. Everything above is a
    # pixel count taken through an opaque plate.
    print("console bytes while locked     : %d" % leaked)
    print("console tail                   : %s" % " | ".join(tail))
    # THE FRAME ITSELF, KEPT. Three figures reading 0.0002 in a row say the
    # screen stopped changing and cannot say WHAT is on it - the run that found
    # the desk_key leak printed exactly that and the numbers alone could not
    # distinguish "still locked" from "unlocked onto something else". The last
    # frame is written out so the next reader can look instead of infer.
    try:
        from PIL import Image
        W,H,px = rd(D)
        Image.frombytes("RGB",(W,H),px[:W*H*3]).save(
            "/home/roy/Documents/repos/zl-linux-presswork/kernel/shots/lock-after-escape.png")
        print("last frame                     : kernel/shots/lock-after-escape.png")
    except Exception as e:
        print("last frame                     : not written (%s)" % e)
finally:
    p.terminate()
