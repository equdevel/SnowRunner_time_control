#ifndef TIME_CONTROL_H_INCLUDED
#define TIME_CONTROL_H_INCLUDED

#define VERSION "1.1.0"

#define MOD_NOREPEAT 0x4000
#define BTN_DONATE 100
#define IDT_TIMER 200

#define MUL 1
#define DIV 2
#define SUB 3
#define ADD 4
#define SHIFT_DIV 5
#define CTRL_SUB 6
#define SHIFT_SUB 7
#define ALT_SUB 8
#define CTRL_ADD 9
#define SHIFT_ADD 10
#define ALT_ADD 11
#define SHIFT_0 12
#define SHIFT_1 13
#define SHIFT_2 14
#define SHIFT_3 15
#define SHIFT_4 16
#define SHIFT_5 17
#define SHIFT_6 18
#define ALT_0 19
#define ALT_1 20
#define ALT_2 21
#define ALT_3 22
#define ALT_4 23
#define ALT_5 24
#define ALT_6 25
#define ALT_DIV 26

#define INFO_MESSAGE "\n\nThe game timer is stopped and set to 12:00\n\nPlease do not close this application while the game is running!\n\n\nNumPad /    Start game timer\n\nNumPad *    Stop game timer\n\nNumPad -    Reduce timer by 2 hours\n\nNumPad +    Inrease timer by 2 hours"
#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 350

DWORD get_PID(CHAR *PrName);

DWORD_PTR GetModuleBase(char *lpModuleName, DWORD dwProcessId);

BOOL PatchEx(HANDLE hProcess, LPVOID dst_addr, LPCVOID src_addr, SIZE_T size, SIZE_T *bytes_written);

void print_hex(char *str, int len);

void print_process_memory(HANDLE hProcess, DWORD_PTR addr, SIZE_T size);

DWORD_PTR search_process_memory(HANDLE hProcess, DWORD_PTR StartAddress, char *bytes, SIZE_T size);

BOOL patch_process_memory(HANDLE hProcess, DWORD_PTR dst_addr, char* src_addr, SIZE_T size);

void message_box(char* message, UINT uType);

void inc_time(float *curr_time, float step, BOOL h_round);

BOOL get_time(float *time);

BOOL set_time(float *time);

BOOL start_time();

BOOL stop_time();

BOOL shift_time(float *time, float step);

BOOL set_time_rate(HWND hWnd, float *time, unsigned char rate_factor, BOOL sync_real_time);

float get_local_time();

int init_memory();

void restore_memory();

#endif // TIME_CONTROL_H_INCLUDED
