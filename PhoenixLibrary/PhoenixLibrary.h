#pragma once

#include "structs.h"

#ifdef PHOENIXLIBRARY_EXPORTS
#define PHOENIXLIBRARY_API __declspec(dllexport)
#else
#define PHOENIXLIBRARY_API __declspec(dllimport)
#endif

#define MAX_SEM_COUNT 10
#define Buffers 10

#define ENEMYSHIPS 20

#define smReadName TEXT("phoenix_read_semaphore")
#define smWriteName TEXT("phoenix_write_semaphore")

#define GAMEDATA_MUTEX_NAME TEXT("phoenix_gamedata_mutex")
#define GAMEDATA_SHARED_MEMORY_NAME TEXT("phoenix_gamedata_shared_memory")

#define PIPE_NAME_INBOUND TEXT("\\\\.\\pipe\\phoenix-gateway-inbountd")
#define PIPE_NAME_OUTBOUND TEXT("\\\\.\\pipe\\phoenix-gateway-outbound")

#ifdef __cplusplus
extern "C" {
#endif
PHOENIXLIBRARY_API VOID Error(CONST TCHAR *text);
PHOENIXLIBRARY_API BOOL writeDataToPipe(LPVOID data, SIZE_T size, HANDLE hPipe,
                                        LPDWORD nBytes);
PHOENIXLIBRARY_API BOOL receiveDataFromPipe(LPVOID data, SIZE_T size,
                                            HANDLE hPipe, LPDWORD nBytes);
PHOENIXLIBRARY_API BOOL initMemAndSync(HANDLE *hMapFile,
                                       const TCHAR *sharedMemoryName,
                                       HANDLE *hMutex, const TCHAR *mutexName);
PHOENIXLIBRARY_API VOID writeDataToSharedMemory(LPVOID sharedMemory,
                                                LPVOID data, SIZE_T size,
                                                HANDLE *hMutex);
PHOENIXLIBRARY_API VOID readDataFromSharedMemory(LPVOID sharedMemory,
                                                 LPVOID data, SIZE_T size,
                                                 HANDLE *hMutex);
#ifdef __cplusplus
}
#endif
