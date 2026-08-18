/* Real fill-rate test: draw N full-screen ALPHA-BLENDED quads, offscreen,
 * and count pixels per second. Blended, because that is what a compositor
 * actually does - stacking translucent surfaces. */
#include <GL/glx.h>
#include <GL/gl.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec/1e9;}
int main(void){
    Display *d=XOpenDisplay(NULL); if(!d){printf("no display\n");return 1;}
    int att[]={GLX_RGBA,GLX_DEPTH_SIZE,24,GLX_DOUBLEBUFFER,None};
    XVisualInfo*vi=glXChooseVisual(d,DefaultScreen(d),att); if(!vi){printf("no visual\n");return 1;}
    /* an offscreen pixmap - never appears on screen, does not disturb the desktop */
    int W=1920,H=1200;
    Pixmap pm=XCreatePixmap(d,RootWindow(d,vi->screen),W,H,vi->depth);
    GLXPixmap gp=glXCreateGLXPixmap(d,vi,pm);
    GLXContext cx=glXCreateContext(d,vi,NULL,True);
    if(!glXMakeCurrent(d,gp,cx)){printf("no context\n");return 1;}
    printf("GL renderer: %s\n",(const char*)glGetString(GL_RENDERER));
    glViewport(0,0,W,H);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    /* warm up */
    for(int i=0;i<20;i++){glColor4f(.3f,.5f,.9f,.5f);glBegin(GL_QUADS);
        glVertex2f(-1,-1);glVertex2f(1,-1);glVertex2f(1,1);glVertex2f(-1,1);glEnd();}
    glFinish();
    int N=300;
    double t0=now();
    for(int i=0;i<N;i++){glColor4f(.3f,.5f,.9f,.5f);glBegin(GL_QUADS);
        glVertex2f(-1,-1);glVertex2f(1,-1);glVertex2f(1,1);glVertex2f(-1,1);glEnd();}
    glFinish();
    double t1=now();
    double px=(double)N*W*H, secs=t1-t0;
    printf("%d blended full-screen quads at %dx%d in %.3f s\n",N,W,H,secs);
    printf("  -> %.2f Gpixel/s blended\n",px/secs/1e9);
    printf("  -> one 1920x1200 blended layer costs %.4f ms\n",secs/N*1000.0);
    return 0;
}
