#
# DietConfig.cmake: CMake configuration file for external projects using Diet.
#

# --------------------------------------------------------------------------
# The DIET include path:
set(DIET_INCLUDE_DIR "C:/Program Files (x86)/DIET/include")

# --------------------------------------------------------------------------
# Version number
set(DIET_BUILD_VERSION   "2012/01/11-18:07:47"")
set(DIET_VERSION         "2.8.0")
set(DIET_USE_ALT_BATCH   "ON")
set(DIET_USE_WORKFLOW    "ON")
set(DIET_WITH_STATISTICS "ON")
set(DIET_WITH_MULTI_MA   "ON")
set(DIET_USE_USERSCHED   "ON")
set(DIET_USE_CCS         "ON")

# --------------------------------------------------------------------------
# Path to DIET libraries
if(CYGWIN AND BUILD_SHARED_LIBS)
  # When compiling with gcc in gycwin shared libraries are installed in
  # the bin directory
  set(DIET_LIBRARY_DIR "C:/Program Files (x86)/DIET/bin")
else(CYGWIN AND BUILD_SHARED_LIBS)
  set(DIET_LIBRARY_DIR "C:/Program Files (x86)/DIET/lib")
endif(CYGWIN AND BUILD_SHARED_LIBS)
link_directories(${DIET_LIBRARY_DIR})
# The DIET libraries to link against
set(DIET_CLIENT_LIBRARIES DIET_client DIET_Dagda C:/Users/absila/Workspace/omniORB-4.1.5-win32-vc10/omniORB-4.1.5/lib/x86_win32/omniDynamic4_rt.lib;C:/Users/absila/Workspace/omniORB-4.1.5-win32-vc10/omniORB-4.1.5/lib/x86_win32/omniORB4_rt.lib;C:/Users/absila/Workspace/omniORB-4.1.5-win32-vc10/omniORB-4.1.5/lib/x86_win32/omnithread_rt.lib;C:/Users/absila/Workspace/omniORB-4.1.5-win32-vc10/omniORB-4.1.5/lib/x86_win32/COS4_rt.lib;C:/Users/absila/Workspace/omniORB-4.1.5-win32-vc10/omniORB-4.1.5/lib/x86_win32/COSDynamic4_rt.lib;optimized;C:/OpenSSL-Win32/lib/VC/ssleay32MD.lib;debug;C:/OpenSSL-Win32/lib/VC/ssleay32MDd.lib;optimized;C:/OpenSSL-Win32/lib/VC/libeay32MD.lib;debug;C:/OpenSSL-Win32/lib/VC/libeay32MDd.lib;C:/Users/absila/Workspace/xerces-c-3.1.1-x86-windows-vc-9.0/lib/xerces-c_3.lib;C:/Users/absila/Workspace/XQilla-2.3.0/build/windows/VC8/Win32/Release/xqilla23.lib)
set(DIET_SERVER_LIBRARIES DIET_SeD DIET_Dagda C:/Users/absila/Workspace/omniORB-4.1.5-win32-vc10/omniORB-4.1.5/lib/x86_win32/omniDynamic4_rt.lib;C:/Users/absila/Workspace/omniORB-4.1.5-win32-vc10/omniORB-4.1.5/lib/x86_win32/omniORB4_rt.lib;C:/Users/absila/Workspace/omniORB-4.1.5-win32-vc10/omniORB-4.1.5/lib/x86_win32/omnithread_rt.lib;C:/Users/absila/Workspace/omniORB-4.1.5-win32-vc10/omniORB-4.1.5/lib/x86_win32/COS4_rt.lib;C:/Users/absila/Workspace/omniORB-4.1.5-win32-vc10/omniORB-4.1.5/lib/x86_win32/COSDynamic4_rt.lib;optimized;C:/OpenSSL-Win32/lib/VC/ssleay32MD.lib;debug;C:/OpenSSL-Win32/lib/VC/ssleay32MDd.lib;optimized;C:/OpenSSL-Win32/lib/VC/libeay32MD.lib;debug;C:/OpenSSL-Win32/lib/VC/libeay32MDd.lib;C:/Users/absila/Workspace/xerces-c-3.1.1-x86-windows-vc-9.0/lib/xerces-c_3.lib;C:/Users/absila/Workspace/XQilla-2.3.0/build/windows/VC8/Win32/Release/xqilla23.lib)
set(DIET_ADMIN_LIBRARIES DIET_admin DIET_SeD DIET_Dagda C:/Users/absila/Workspace/omniORB-4.1.5-win32-vc10/omniORB-4.1.5/lib/x86_win32/omniDynamic4_rt.lib;C:/Users/absila/Workspace/omniORB-4.1.5-win32-vc10/omniORB-4.1.5/lib/x86_win32/omniORB4_rt.lib;C:/Users/absila/Workspace/omniORB-4.1.5-win32-vc10/omniORB-4.1.5/lib/x86_win32/omnithread_rt.lib;C:/Users/absila/Workspace/omniORB-4.1.5-win32-vc10/omniORB-4.1.5/lib/x86_win32/COS4_rt.lib;C:/Users/absila/Workspace/omniORB-4.1.5-win32-vc10/omniORB-4.1.5/lib/x86_win32/COSDynamic4_rt.lib;optimized;C:/OpenSSL-Win32/lib/VC/ssleay32MD.lib;debug;C:/OpenSSL-Win32/lib/VC/ssleay32MDd.lib;optimized;C:/OpenSSL-Win32/lib/VC/libeay32MD.lib;debug;C:/OpenSSL-Win32/lib/VC/libeay32MDd.lib;C:/Users/absila/Workspace/xerces-c-3.1.1-x86-windows-vc-9.0/lib/xerces-c_3.lib;C:/Users/absila/Workspace/XQilla-2.3.0/build/windows/VC8/Win32/Release/xqilla23.lib)

if(DIET_USE_ALT_BATCH)
  add_definitions(-DHAVE_ALT_BATCH)
endif(DIET_USE_ALT_BATCH)
if(DIET_USE_WORKFLOW)
  add_definitions(-DHAVE_WORKFLOW)
endif(DIET_USE_WORKFLOW)

