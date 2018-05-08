#pragma once

#include <tchar.h>
#include <windows.h>

#ifdef DLL_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif
extern DLL_IMP_API int nDLL;
DLL_IMP_API int UmaString(void);
#ifdef __cplusplus
}
#endif