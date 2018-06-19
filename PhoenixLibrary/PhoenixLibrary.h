#pragma once

#include "structs.h"

#ifdef PHOENIXLIBRARY_EXPORTS
#define PHOENIXLIBRARY_API __declspec(dllexport)
#else
#define PHOENIXLIBRARY_API __declspec(dllimport)
#endif

#define MAX_ENEMY_SHIPS 20
#define MAX_PLAYERS 5
#define MAX_CLIENTS 10

#define GAMEDATA_READ_SEMAPHORE_NAME TEXT("phoenix_gamedata_read_semaphore")
#define GAMEDATA_WRITE_SEMAPHORE_NAME TEXT("phoenix_gamedata_write_semaphore")

#define MESSAGES_READ_SEMAPHORE_NAME TEXT("phoenix_messages_read_semaphore")
#define MESSAGES_WRITE_SEMAPHORE_NAME TEXT("phoenix_messages_write_semaphore")

#define GAMEDATA_MUTEX_NAME TEXT("phoenix_gamedata_mutex")
#define GAMEDATA_SHARED_MEMORY_NAME TEXT("phoenix_gamedata_shared_memory")

#define MESSAGES_MUTEX_NAME TEXT("phoenix_messages_mutex")
#define MESSAGES_SHARED_MEMORY_NAME TEXT("phoenix_messages_shared_memory")

#define PIPE_GAME_NAME TEXT("\\\\.\\pipe\\phoenix_pipe_game")
#define PIPE_MESSAGE_NAME TEXT("\\\\.\\pipe\\phoenix_pipe_message")

#define GAME_UPDATE_EVENT TEXT("Global\\phoenix_game_update_event")
#define MESSAGE_SERVER_UPDATE_EVENT                                            \
  TEXT("Global\\phoenix_server_message_update_event")
#define MESSAGE_GATEWAY_UPDATE_EVENT                                           \
  TEXT("Global\\phoenix_gateway_message_update_event")

#define SERVER_RUNNING_EVENT TEXT("Global\\phoenix_server_running")
#define GATEWAY_RUNNING_EVENT TEXT("Global\\phoenix_gateway_running")

#define ENEMYSHIPS_MUTEX TEXT("phoenix_enemyships_mutex")

#define SERVER_CLOSE_EVENT TEXT("phoenix_server_close_event")

#define CLIENT_CLOSE_EVENT TEXT("phoenix_clinet_close_event")

#define BUFSIZE sizeof(Message)

#ifdef __cplusplus
extern "C" {
#endif
PHOENIXLIBRARY_API VOID error(LPCWSTR text, ...);
PHOENIXLIBRARY_API VOID debug(LPCWSTR text, ...);
PHOENIXLIBRARY_API BOOL initMemAndSync(HANDLE *hMapFile,
                                       LPCWSTR sharedMemoryName, HANDLE *hMutex,
                                       LPCWSTR mutexName);
PHOENIXLIBRARY_API VOID writeDataToSharedMemory(LPVOID sharedMemory,
                                                LPVOID data, SIZE_T size,
                                                HANDLE hMutex, HANDLE hEvent);
PHOENIXLIBRARY_API VOID readDataFromSharedMemory(LPVOID sharedMemory,
                                                 LPVOID data, SIZE_T size,
                                                 HANDLE *hMutex);
PHOENIXLIBRARY_API BOOL initMessageZone(MessageData *messageData);
PHOENIXLIBRARY_API BOOL initGameZone(GameData *gameData);
PHOENIXLIBRARY_API BOOL isGatewayRunning();
PHOENIXLIBRARY_API BOOL isServerRunning();
PHOENIXLIBRARY_API VOID errorGui(LPCWSTR text);
PHOENIXLIBRARY_API BOOL readDataFromPipe(HANDLE hPipe, LPVOID data,
                                         SIZE_T size);
PHOENIXLIBRARY_API BOOL writeDataToPipe(HANDLE hPipe, LPVOID data, SIZE_T size);
PHOENIXLIBRARY_API BOOL writeDataToPipeAsync(HANDLE hPipe, HANDLE hEvent,
                                             LPVOID data, SIZE_T size);
PHOENIXLIBRARY_API BOOL readDataFromPipeAsync(HANDLE hPipe, HANDLE hEvent,
                                              LPVOID data, SIZE_T size);
#ifdef __cplusplus
}
#endif
