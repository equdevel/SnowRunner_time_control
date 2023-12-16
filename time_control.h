#ifndef TIME_CONTROL_H_INCLUDED
#define TIME_CONTROL_H_INCLUDED

#define VERSION "1.0.9"
#define MOD_NOREPEAT 0x4000
#define BTN_DONATE 100
#define INFO_MESSAGE "\n\nThe game timer is stopped and set to 12:00\n\nPlease do not close this application while the game is running!\n\n\nNumPad /    Start game timer\n\nNumPad *    Stop game timer\n\nNumPad -    Reduce timer by 2 hours\n\nNumPad +    Inrease timer by 2 hours"
#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 350

DWORD get_PID(CHAR *PrName);

DWORD_PTR GetModuleBase(char *lpModuleName, DWORD dwProcessId);

BOOL PatchEx(HANDLE hProcess, LPVOID dst, LPCVOID src, SIZE_T size, SIZE_T *BytesWritten);

void print_hex(char *str, int len);

void print_process_memory(HANDLE hProcess, DWORD_PTR pBuffer, SIZE_T size);

DWORD_PTR search_process_memory(HANDLE hProcess, DWORD_PTR StartAddress, char *bytes, SIZE_T size);

BOOL patch_process_memory(HANDLE hProcess, DWORD_PTR pBuffer, char* new_buffer, SIZE_T size);

void message_box(char* message, UINT uType);

void inc_time(float *curr_time, float step);

BOOL get_time(HANDLE hProcess, DWORD_PTR pNewMemoryRegion, float *time);

BOOL set_time(HANDLE hProcess, DWORD_PTR pNewMemoryRegion, float *time);

BOOL start_time(HANDLE hProcess, DWORD_PTR pNewMemoryRegion);

BOOL stop_time(HANDLE hProcess, DWORD_PTR pNewMemoryRegion);

#endif // TIME_CONTROL_H_INCLUDED
