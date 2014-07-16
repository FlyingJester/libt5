import os
import sys

defaultenv = Environment(ENV = os.environ)

msvc_ccflags = "/O2 /EHsc /Zi"
msvc_cflags = ""
msvc_cxxflags = ""

gcc_ccflags = " -Werror=return-type -Wall -Wno-pointer-arith -mtune=native -O3 -pipe -Wcast-align -g"
gcc_cflags = " -std=c11 "
gcc_cxxflags = " -std=c++11 "

if os.environ.get('TRAVIS') == 'true':
  gcc_cflags = " -std=c99 "
  gcc_cxxflags = " -std=gnu++0x "

clang_ccflags = gcc_ccflags + " -DHAS_UNISTD -DHAS_STAT_SYS -DUSE_STAT -Wno-return-type-c-linkage -Wno-null-arithmetic  "
clang_cflags = gcc_cflags
clang_cxxflags = gcc_cxxflags

#Checking platform.

def IsWindows():
  return sys.platform == 'win32' or sys.platform.startswith('win64')

def IsLinux():
  return sys.platform.startswith('linux')

def IsDarwin():
  return sys.platform == 'darwin'

def LibraryInstall(name, library, env):
  if(IsDarwin()):
    print library[0].name
    LibraryFileName = os.path.join(os.getcwd(), library[0].name)
    return env.Command(os.path.join(name, library[0].name), LibraryFileName, "install_name_tool $SOURCE -id "+library[0].name+" && cp $SOURCE $TARGET ")
  else:
    return env.Install(name, [library])

#Todo: Make these overrideable.
if IsWindows():
  compiler = 'msvc'
elif IsLinux():
  compiler = 'gcc'
elif IsDarwin():
  compiler = 'clang'
else:
  compiler = 'cc' # who knows.

#Create a unified environment.
globalenv = defaultenv.Clone()

if os.name == 'posix':
  globalenv.Append(tools = ['default', 'textile'])

if compiler == 'msvc':
  globalenv.Replace(CCFLAGS = msvc_ccflags, CFLAGS = msvc_cflags, CXXFLAGS = msvc_cxxflags)
if compiler == 'gcc':
  globalenv.Replace(CCFLAGS = gcc_ccflags, CFLAGS = gcc_cflags, CXXFLAGS = gcc_cxxflags)
if compiler == 'clang':
  globalenv.Replace(CCFLAGS = clang_ccflags, CFLAGS = clang_cflags, CXXFLAGS = clang_cxxflags)

globalenv.Append(CPPPATH = [ os.path.join(os.getcwd(), "t5")])

builddir = os.path.join(os.getcwd(), "build")

libt5 = SConscript(dirs = ["t5"], exports = "globalenv IsLinux IsWindows IsDarwin LibraryInstall compiler builddir ")

test =  SConscript(dirs = [os.path.join("t5", "t5convert")], exports = "globalenv IsLinux IsWindows IsDarwin LibraryInstall compiler builddir libt5 ")
