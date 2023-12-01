#ifndef TIME_CONTROL_H_INCLUDED
#define TIME_CONTROL_H_INCLUDED

#define MOD_NOREPEAT 0x4000

DWORD get_PID(CHAR *PrName);

DWORD_PTR GetModuleBase(char *lpModuleName, DWORD dwProcessId);

BOOL PatchEx(HANDLE hProcess, LPVOID dst, LPCVOID src, SIZE_T size, SIZE_T *BytesWritten);

void print_hex(char *str, int len);

void print_process_memory(HANDLE hProcess, DWORD_PTR pBuffer, SIZE_T size);

DWORD_PTR search_process_memory(HANDLE hProcess, DWORD_PTR StartAddress, char *bytes, SIZE_T size);

BOOL patch_process_memory(HANDLE hProcess, DWORD_PTR pBuffer, char* new_buffer, SIZE_T size);

void message_box(char* message, UINT uType);

void inc_time(float *curr_time, float step);

#endif // TIME_CONTROL_H_INCLUDED
