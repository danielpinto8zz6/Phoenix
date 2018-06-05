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

#define GAMEDATA_READ_SEMAPHORE_NAME TEXT("phoenix_gamedata_read_semaphore")
#define GAMEDATA_WRITE_SEMAPHORE_NAME TEXT("phoenix_gamedata_write_semaphore")

#define MESSAGES_READ_SEMAPHORE_NAME TEXT("phoenix_messages_read_semaphore")
#define MESSAGES_WRITE_SEMAPHORE_NAME TEXT("phoenix_messages_write_semaphore")

#define GAMEDATA_MUTEX_NAME TEXT("phoenix_gamedata_mutex")
#define GAMEDATA_SHARED_MEMORY_NAME TEXT("phoenix_gamedata_shared_memory")

#define MESSAGES_MUTEX_NAME TEXT("phoenix_messages_mutex")
#define MESSAGES_SHARED_MEMORY_NAME TEXT("phoenix_messages_shared_memory")

#define PIPE_NAME_INBOUND TEXT("\\\\.\\pipe\\phoenix-gateway-inbountd")
#define PIPE_NAME_OUTBOUND TEXT("\\\\.\\pipe\\phoenix-gateway-outbound")

#define GAME_UPDATE_EVENT TEXT("Global\\phoenix_game_update_event")
#define MESSAGE_SERVER_UPDATE_EVENT                                            \
  TEXT("Global\\phoenix_server_message_update_event")
#define MESSAGE_GATEWAY_UPDATE_EVENT                                           \
  TEXT("Global\\phoenix_gateway_message_update_event")

#define SERVER_RUNNING_EVENT TEXT("Global\\phoenix_server_running")
#define GATEWAY_RUNNING_EVENT TEXT("Global\\phoenix_gateway_running")

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
PHOENIXLIBRARY_API BOOL initMessageZone(MessageData *messageData);
PHOENIXLIBRARY_API BOOL initGameZone(GameData *gameData);
PHOENIXLIBRARY_API BOOL isGatewayRunning();
PHOENIXLIBRARY_API BOOL isServerRunning();
#ifdef __cplusplus
}
#endif
