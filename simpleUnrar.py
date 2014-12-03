import ctypes,sys,time

class deletable(object):
 def setDel(self,func,what):
  self.func=func
  self.what=what

 def __del__(self):
  self.func(ctypes.cast(self.what,ctypes.c_void_p))
#  list.__del__(self)

class delList(deletable,list):
 pass

class delString(deletable,str):
 pass

class RarFile(object):
 def __init__(self,filename):
  self.fn=filename
  self.lib=ctypes.cdll.LoadLibrary("./simpleUnrar.so")

 def read(self,name,size=0):
  rType=ctypes.POINTER(ctypes.c_char)
  rd=self.lib.rarRead
  rd.restype=rType
  sz=ctypes.c_long(size)
  szp=ctypes.pointer(sz)
  t=rd(self.fn,name,0,szp)
  s=t[:sz.value]
  self.lib.rarFreeFp(t)
  return s

 def namelist(self):
  rType=ctypes.POINTER(ctypes.c_char_p)
  rl=self.lib.rarList
  rl.restype=rType
  p=rl(self.fn)
  l=[]
  for i in p:
   if i==None:
    break
   l.append(i)
  self.lib.rarFreeFl(p)
  return l


def test():
 f=RarFile(sys.argv[1])
 a=f.namelist()
 for i in a:
  print i, len(f.read(i)),f.read(i,3)

if __name__=='__main__':
 test()

