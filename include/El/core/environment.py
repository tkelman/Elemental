#
#  Copyright (c) 2009-2014, Jack Poulson
#  All rights reserved.
#
#  This file is part of Elemental and is under the BSD 2-Clause License, 
#  which can be found in the LICENSE file in the root directory, or at 
#  http://opensource.org/licenses/BSD-2-Clause
#
import sys, ctypes
from ctypes import pythonapi

# TODO: Greatly improve this search functionality. At the moment, it is likely
#       necessary to manually specify the path
from ctypes.util import find_library
libPath = find_library('El')
if libPath == None:
  raise Exception("Could not find Elemental library")
lib = ctypes.cdll.LoadLibrary(libPath)

# Environment
# ===========

# Basic types
# -----------

from ctypes import c_size_t, c_ubyte, c_uint, c_int, c_float, c_double
from ctypes import c_void_p, c_char_p
from ctypes import pointer, POINTER

# TODO: Switch to a different boolean type if appropriate
bType = c_ubyte
# TODO: Switch from c_int if Elemental was configured for 64-bit integers
iType = c_int
sType = c_float
dType = c_double

class ComplexFloat(ctypes.Structure):
  _fields_ = [("real",sType),("imag",sType)]
  def __init__(self,val0=0,val1=0):
    real = sType()
    imag = sType()
    if type(val0) is cType or type(val0) is zType:
      real = val0.real
      imag = val0.imag
    else:
      real = val0
      imag = val1
    super(ComplexFloat,self).__init__(real,imag)
cType = ComplexFloat
class ComplexDouble(ctypes.Structure):
  _fields_ = [("real",dType),("imag",dType)]
  def __init__(self,val0=0,val1=0):
    real = dType()
    imag = dType()
    if type(val0) is cType or type(val0) is zType:
      real = val0.real
      imag = val0.imag
    else:
      real = val0
      imag = val1
    super(ComplexDouble,self).__init__(real,imag)
zType = ComplexDouble

# Create a simple enum for the supported datatypes
(iTag,sTag,dTag,cTag,zTag)=(0,1,2,3,4)
def CheckTag(tag):
  if tag != iTag and \
     tag != sTag and tag != dTag and \
     tag != cTag and tag != zTag:
    print 'Unsupported datatype'

def Base(tag):
  if   tag == iTag: return iTag
  elif tag == sTag: return sTag
  elif tag == dTag: return dTag
  elif tag == cTag: return sTag
  elif tag == zTag: return dTag
  else: raise Exception('Invalid tag')

def Complexify(tag):
  if   tag == sTag: return cTag
  elif tag == dTag: return zTag
  elif tag == cTag: return cTag
  elif tag == zTag: return zTag
  else: raise Exception('Invalid tag')

def TagToType(tag):
  if   tag == iTag: return iType
  elif tag == sTag: return sType
  elif tag == dTag: return dType
  elif tag == cTag: return cType
  elif tag == zTag: return zType
  else: raise Exception('Invalid tag')

# Emulate an enum for matrix distributions
(MC,MD,MR,VC,VR,STAR,CIRC)=(0,1,2,3,4,5,6)

def GatheredDist(dist):
  if   dist == CIRC: return CIRC
  else: return STAR

def DiagColDist(colDist,rowDist):
  if colDist == MC and rowDist == MR:   return MD
  elif colDist == MR and rowDist == MC: return MD
  elif colDist == STAR:                 return rowDist
  else:                                 return colDist

def DiagRowDist(colDist,rowDist):
  if colDist == MC and rowDist == MR:   return STAR
  elif colDist == MR and rowDist == MC: return STAR
  elif colDist == STAR:                 return colDist
  else:                                 return rowDist

class IndexRange(ctypes.Structure):
  _fields_ = [("beg",iType),("end",iType)]
  def __init__(self,ind0,ind1=None):
    beg = iType()
    end = iType()
    if isinstance(ind0,slice):
      if ind0.step != None and ind0.step != 1:
        raise Exception('Slice step must be one')
      beg = iType(ind0.start)
      end = iType(ind0.stop)
    else:
      beg = iType(ind0)
      if ind1 is None: end = iType(beg.value+1)
      else:            end = iType(ind1)
    super(IndexRange,self).__init__(beg,end)

# Emulate various enums
# ---------------------

# Grid ordering
(ROW_MAJOR,COL_MAJOR)=(0,1)

# Upper or lower
(LOWER,UPPER)=(0,1)

# Left or right
(LEFT,RIGHT)=(0,1)

# Matrix orientation 
(NORMAL,TRANSPOSE,ADJOINT)=(0,1,2)

# Unit/non-unit diagonal
(NON_UNIT,UNIT)=(0,1)

# File format 
(AUTO,ASCII,ASCII_MATLAB,BINARY,BINARY_FLAT,BMP,JPG,JPEG,MATRIX_MARKET,
 PNG,PPM,XBM,XPM)=(0,1,2,3,4,5,6,7,8,9,10,11,12)

# Colormap
(GRAYSCALE,GRAYSCALE_DISCRETE,RED_BLACK_GREEN,BLUE_RED)=(0,1,2,3)

# Norm types
(ONE_NORM,INFINITY_NORM,ENTRYWISE_ONE_NORM,MAX_NORM,NUCLEAR_NORM,FROBENIUS_NORM,
 TWO_NORM)=(0,1,2,3,4,5,6)

# Sort type
(UNSORTED,DESCENDING,ASCENDING)=(0,1,2)

# Pencil
(AXBX,ABX,BAX)=(1,2,3)

# TODO: Many more enums

# Miscellaneous small data structures
# -----------------------------------
class SafeProduct_s(ctypes.Structure):
  _fields_ = [("rho",sType),("kappa",sType),("n",iType)]
class SafeProduct_d(ctypes.Structure):
  _fields_ = [("rho",dType),("kappa",dType),("n",iType)]
class SafeProduct_c(ctypes.Structure):
  _fields_ = [("rho",cType),("kappa",sType),("n",iType)]
class SafeProduct_z(ctypes.Structure):
  _fields_ = [("rho",zType),("kappa",dType),("n",iType)]
def TagToSafeProduct(tag):
  if   tag == sTag: return SafeProduct_s()
  elif tag == dTag: return SafeProduct_d()
  elif tag == cTag: return SafeProduct_c()
  elif tag == zTag: return SafeProduct_z()
  else: DataExcept()

class InertiaType(ctypes.Structure):
  _fields_ = [("numPositive",iType),("numNegative",iType),("numZero",iType)]

# Initialization
# --------------

lib.ElInitialize.argtypes = [POINTER(c_int),POINTER(POINTER(c_char_p))]
lib.ElInitialize.restype = c_uint
def Initialize():
  argc = c_int()
  argv = POINTER(c_char_p)()
  pythonapi.Py_GetArgcArgv(ctypes.byref(argc),ctypes.byref(argv))
  lib.ElInitialize(pointer(argc),pointer(argv))

lib.ElFinalize.argtypes = []
lib.ElFinalize.restype = c_uint
def Finalize():
  lib.ElFinalize()

lib.ElInitialized.argtypes = [bType]
lib.ElInitialized.restype = c_uint
def Initialized():
  # NOTE: This is not expected to be portable and should be fixed
  active = bType()
  activeP = pointer(active)
  lib.ElInitialized( activeP )
  return active.value

lib.ElSetBlocksize.argtypes = [iType]
lib.ElSetBlocksize.restype = c_uint
def SetBlocksize(blocksize):
  lib.ElSetBlocksize(blocksize)

lib.ElBlocksize.argtypes = [POINTER(iType)]
lib.ElBlocksize.restype = c_uint
def Blocksize():
  blocksize = iType()
  lib.ElBlocksize(pointer(blocksize))
  return blocksize

# Import several I/O routines
# ---------------------------
pythonapi.PyFile_AsFile.argtypes = [ctypes.py_object]
pythonapi.PyFile_AsFile.restype = c_void_p

lib.ElPrintVersion.argtypes = [c_void_p]
lib.ElPrintVersion.restype = c_uint
def PrintVersion(f=pythonapi.PyFile_AsFile(sys.stdout)):
  lib.ElPrintVersion(f)

lib.ElPrintConfig.argtypes = [c_void_p]
lib.ElPrintConfig.restype = c_uint
def PrintConfig(f=pythonapi.PyFile_AsFile(sys.stdout)):
  lib.ElPrintConfig(f)

lib.ElPrintCCompilerInfo.argtypes = [c_void_p]
lib.ElPrintCCompilerInfo.restype = c_uint
def PrintCCompilerInfo(f=pythonapi.PyFile_AsFile(sys.stdout)):
  lib.ElPrintCCompilerInfo(f)

lib.ElPrintCxxCompilerInfo.argtypes = [c_void_p]
lib.ElPrintCxxCompilerInfo.restype = c_uint
def PrintCxxCompilerInfo(f=pythonapi.PyFile_AsFile(sys.stdout)):
  lib.ElPrintCxxCompilerInfo(f)

# Input routines
# --------------
# TODO?

# Initialize MPI
# --------------
Initialize()

def TypeExcept():
  raise Exception('Unsupported matrix type')
def DataExcept():
  raise Exception('Unsupported datatype')
