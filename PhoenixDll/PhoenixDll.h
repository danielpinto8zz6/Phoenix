// O bloco ifdef seguinte � o modo standard de criar macros que tornam a
// exporta��o de fun��es e vari�veis mais simples. Todos os ficheiros neste
// projeto DLL s�o compilados com o s�mbolo DLL_IMP_EXPORTS definido. Este
#include <tchar.h>
#include <windows.h>

#define TAM 256

#ifdef DLL_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif
// Functions to be imported / exported
DLL_IMP_API void CheckDLL(void);
#ifdef __cplusplus
}
#endif
