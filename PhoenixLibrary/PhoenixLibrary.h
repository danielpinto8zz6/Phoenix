#pragma once

#include "structs.h"

#ifdef PHOENIXLIBRARY_EXPORTS
#define PHOENIXLIBRARY_API __declspec(dllexport)
#else
#define PHOENIXLIBRARY_API __declspec(dllimport)
#endif

#define MAX_SEM_COUNT 10
#define Buffers 10

#define smReadName TEXT("smReadName")
#define smWriteName TEXT("smWriteName")
#define mReadName TEXT("mRead")
#define mWriteName TEXT("mWrite")

#define PIPE_NAME_INBOUND TEXT("\\\\.\\pipe\\phoenix-gateway-inbountd")
#define PIPE_NAME_OUTBOUND TEXT("\\\\.\\pipe\\phoenix-gateway-outbound")

#ifdef __cplusplus
extern "C" {
#endif
PHOENIXLIBRARY_API void Error(const TCHAR *text);
PHOENIXLIBRARY_API BOOL initMemAndSync(ControlData *data);
PHOENIXLIBRARY_API BOOL initSemaphores(ControlData *data);
PHOENIXLIBRARY_API unsigned peekData(ControlData *data);
PHOENIXLIBRARY_API void readData(ControlData *data, Game *game);
PHOENIXLIBRARY_API void writeData(ControlData *data, Game *game);
#ifdef __cplusplus
}
#endif
