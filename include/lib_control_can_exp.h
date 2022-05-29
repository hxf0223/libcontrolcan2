#pragma once

#ifdef WIN32

#ifdef BUILDING_LIBCC_DLL
#define LIBCC_DLL __declspec(dllexport)
#else
#define LIBCC_DLL __declspec(dllimport)
#endif

#define LIBCC_EXPORT_CALL LIBCC_DLL __stdcall

#else

#ifdef BUILDING_LIBCC_DLL
#define LIBCC_DLL __attribute__((visibility("default")))
#else
#define LIBCC_DLL //__declspec(dllimport)
#endif

#define LIBCC_EXPORT_CALL __stdcall LIBCC_DLL

#endif