#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <ctype.h>
#include <wchar.h>
#include <stdbool.h>
#include "rar.hpp"
#include "dll.hpp"

#ifdef __cplusplus
extern "C" {
#endif
struct rar_t {
struct RAROpenArchiveDataEx *oad;
struct RARHeaderDataEx *hd;
void *handle;
char *fp;
char *fpTemp;
long fpos;
long fplimit;
};
typedef struct rar_t rar;

int cb(uint msg, long udata, long p1, long p2);
rar *rarClose(rar *r);
rar *rarOpen(char *filename, int mode);
rar *rarInit();
rar *testRar(char *filename);
char *rarRead(char *filename, char* internalFilename,char *ptr, int *sz);
char **rarList(char *filename);
void rarFreeFp(char *);
void rarFreeFl(char **);
void rarFree(rar *r);

void rarFreeFl(char **fl) {
int i;
for(i=0;fl[i];i++) {
free(fl[i]);
}
free(fl);
};

void rarFreeFp(char *f) {
if(f) {
free(f);
}
return;
}

char *rarRead(char *filename, char* iFilename, char *ptr, int *sz) {
rar *r;
char *f;
f=NULL;
int err,res;
r=rarOpen(filename,0);
err=0;
while(1) {
res=RARReadHeaderEx(r->handle,r->hd);
if(res!=0) {
err=1;
break;
}
if(strcmp(r->hd->FileName,iFilename)==0) {
break;
}
RARProcessFile(r->handle,RAR_SKIP,NULL,NULL);
}
if(err) {
rarClose(r);
rarFree(r);
return NULL;
}
RARSetCallback(r->handle,cb,(long)r);
if(ptr) {
r->fp=ptr;
} else {
r->fp=(char *)calloc(1,r->hd->UnpSize);
}
r->fpTemp=r->fp;
r->fpos=0;
r->fplimit=sz?(*sz):0;
RARProcessFile(r->handle,RAR_TEST,NULL,NULL);
if(r->fp) {
f=r->fp;
if(sz) {
*sz=r->fpos;
}
}
rarClose(r);
rarFree(r);
return f;
}

void rarFree(rar *r) {
if(r->oad) {
free(r->oad);
}
if(r->hd) {
free(r->hd);
}
free(r);
}

rar *rarInit() {
rar *r=(rar *)calloc(1,sizeof(rar));
r->oad=(struct RAROpenArchiveDataEx *) calloc(1,sizeof(struct RAROpenArchiveDataEx));
r->hd=(struct RARHeaderDataEx *) calloc(1,sizeof(struct RARHeaderDataEx));
r->fp=NULL;
return r;
}

char **rarList(char *filename) {
int count,where,res;
char **names;
rar *r;
where=0;
count=256;
names=(char **)calloc(sizeof(char *),count);
r=rarOpen(filename,0);
while(1) {
res=RARReadHeaderEx(r->handle,r->hd);
if(res!=0) {
break;
}
if(where+1>=count) {
names=(char **) realloc(names,sizeof(char *)*count*2);
memset(names+(count-1),0,(count+2)*sizeof(char *));
count=count*2;
}
char *curFn;
curFn=r->hd->FileName;
//printf("%s\n",curFn);
names[where]=(char *) calloc(sizeof(char),strlen(curFn)+1);
strcpy(names[where],curFn);
where+=1;
RARProcessFile(r->handle,RAR_SKIP,NULL,NULL);
}
//names[where]=(char *) malloc(1);
//names[where][0]=(char) 0;
rarClose(r);
rarFree(r);
return names;
};

rar *rarClose(rar *r){
RARCloseArchive(r->handle);
return r;
};

rar *rarOpen(char *filename, int mode) {
rar *r;
char *newFn;
int res;
newFn=(char *)calloc(1,256);
r=rarInit();
r->oad->ArcName=strcpy(newFn,filename);
r->oad->OpenMode=RAR_OM_EXTRACT;
r->handle=RAROpenArchiveEx(r->oad);
return r;
}

rar *testRar(char *filename) {
int res,i;
rar *r;
r=NULL;
int y;
char **n,*x;
//r=rarOpen(filename,NULL);
//EXTRACT;
//printf("opened:%du\n",r->oad->OpenResult);
n=rarList(filename);
for(i=0;n[i];i++) {
x=rarRead(filename,n[i],NULL,&y);
printf("%d (%d), %s\n",i,y,n[i]);
free(x);
}
return r;
RARCloseArchive(r->handle);
return r;
}

int cb(uint msg, long udata, long p1, long p2) {
if(msg==UCM_PROCESSDATA) {
char *fc=(char *)p1;
rar *r=(rar *)udata;
long c,fl;
//if limit is set, copy upto limit bytes, or all the current bytes if we're under limit
//so 100,000 would be at least one cb call,
//and if our limit was set at 90000,
//we'd get one call where limit was set and greater than what had beenr ead+what would be read,
//then a call where the limit would be less than the value read+the value going to be read (90000<65536+65536) so fplimit would be used
fl=r->fplimit-r->fpos;
c=p2;
if(fl) {
if(p2>=fl) {
if(fl<=p2) {
c=fl;
}
}
}
//c=(c>p2)?p2:c;
//printf("copying %d of %d\n",c,p2);
memcpy(r->fpTemp,fc,c);
r->fpos+=c;
r->fpTemp+=c;
if(r->fpos>=r->fplimit) {
return -1;
}
//printf("%ld\n",p2);
}
//printf("cb\n");
return 0;
}

int main(int argc, char **argv) {
if(argc<2) {
printf("provide filename as first argument\n");
exit((int)0);
} else {
testRar(argv[1]);
}
}
#ifdef __cplusplus
}
#endif 
