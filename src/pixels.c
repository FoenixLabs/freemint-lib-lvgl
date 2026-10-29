#include "lvgl_gem_pixels.h"
#include "lvgl_gem_native.h"
#include <limits.h>
int lg_intersect(lg_rect a,lg_rect b,lg_rect *o) {
    long x=a.x>b.x?a.x:b.x,y=a.y>b.y?a.y:b.y;
    long r=(long)a.x+a.w, s=(long)b.x+b.w;
    long d=(long)a.y+a.h, t=(long)b.y+b.h;
    if(s<r)r=s;
    if(t<d)d=t;
    if(a.w<=0||a.h<=0||b.w<=0||b.h<=0||r<=x||d<=y)return 0;
    o->x=(int)x;o->y=(int)y;o->w=(int)(r-x);o->h=(int)(d-y);return 1;
}
int lg_surface_size(unsigned w,unsigned h,unsigned planes,size_t *stride,size_t *size) {
    size_t row;
    if(!w||!h||w>4096||h>4096||(planes!=1&&planes!=16&&planes!=32))return 0;
    row=((size_t)w+15)/16*2*planes;
    if(h>(size_t)-1/row)return 0;
    *stride=row;*size=row*h;return 1;
}
void lg_pixel565(uint8_t *row,unsigned x,uint16_t p,unsigned planes) {
    unsigned r=(p>>11)&31,g=(p>>5)&63,b=p&31;
    r=(r<<3)|(r>>2);g=(g<<2)|(g>>4);b=(b<<3)|(b>>2);
    if(planes==1){unsigned mask=128u>>(x&7); if(77*r+150*g+29*b<128*256)row[x/8]|=mask;else row[x/8]&=(uint8_t)~mask;}
    else if(planes==16){row[2*x]=(uint8_t)(p>>8);row[2*x+1]=(uint8_t)p;}
    else if(planes==32){row[4*x]=255;row[4*x+1]=r;row[4*x+2]=g;row[4*x+3]=b;}
}

void lg_pixel888_mono(uint8_t *row,unsigned x,unsigned y,uint8_t r,uint8_t g,uint8_t b){
    static const uint8_t bayer[4][4]={{0,8,2,10},{12,4,14,6},{3,11,1,9},{15,7,13,5}};
    unsigned l=(77u*r+150u*g+29u*b)>>8,mask=128u>>(x&7);
    if(l<(unsigned)bayer[y&3][x&3]*16+8)row[x/8]|=mask;
    else row[x/8]&=(uint8_t)~mask;
}

void lg_pixel888_native(uint8_t *row,unsigned x,unsigned planes,uint8_t r,uint8_t g,uint8_t b){
    if(planes==32){row[4*x]=255;row[4*x+1]=r;row[4*x+2]=g;row[4*x+3]=b;}
    else if(planes==16){uint16_t p=((uint16_t)(r>>3)<<11)|((uint16_t)(g>>2)<<5)|(b>>3);row[2*x]=p>>8;row[2*x+1]=(uint8_t)p;}
}
