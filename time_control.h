#ifndef TIME_CONTROL_H_INCLUDED
#define TIME_CONTROL_H_INCLUDED

#define VERSION "1.2.2"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define MOD_NOREPEAT 0x4000
#define BTN_DONATE 100
#define BTN_YOUTUBE 110
#define BTN_TELEGRAM 120
#define IDT_TIMER 200

#define MUL 1
#define DIV 2
#define SUB 3
#define ADD 4
#define ALT_DIV 5
#define CTRL_SUB 6
#define SHIFT_SUB 7
#define ALT_SUB 8
#define CTRL_ADD 9
#define SHIFT_ADD 10
#define ALT_ADD 11
#define CTRL_0 12
#define CTRL_1 13
#define CTRL_2 14
#define CTRL_3 15
#define CTRL_4 16
#define CTRL_5 17
#define CTRL_6 18
#define CTRL_7 19
#define CTRL_8 20
#define CTRL_9 21
#define ALT_0 22
#define ALT_1 23
#define ALT_2 24
#define ALT_3 25
#define ALT_4 26
#define ALT_5 27
#define ALT_6 28

//#define TOP_MESSAGE "\nPlease do not close this application while the game is running!"
#define LEFT_MESSAGE "\n"\
                     "  ----------------- Start/stop time -----------------\n"\
                     "          NumPad*         stop time\n"\
                     "          NumPad/         start default game time\n"\
                     "    Alt + NumPad/         sync with real clock\n\n"\
                     "  ------------------ Speed up time ------------------\n"\
                     "  Shift + NumPad2         x2\n"\
                     "  Shift + NumPad3         x3\n"\
                     "  Shift + NumPad4         x4\n"\
                     "  Shift + NumPad5         x5\n"\
                     "  Shift + NumPad6         x6\n"\
                     "  Shift + NumPad7         x7\n"\
                     "  Shift + NumPad8         x8\n"\
                     "  Shift + NumPad9         x9\n"\
                     "  Shift + NumPad1         x10\n"\
                     "  Shift + NumPad0         x12\n\n"\
                     "  ----------------- Slow down time ------------------\n"\
                     "    Alt + NumPad2         x2\n"\
                     "    Alt + NumPad3         x3\n"\
                     "    Alt + NumPad4         x4\n"\
                     "    Alt + NumPad5         x5\n"\
                     "    Alt + NumPad6         x6\n"\
                     "    Alt + NumPad7         x7\n"\
                     "    Alt + NumPad8         x8\n"\
                     "    Alt + NumPad9         x9\n"\
                     "    Alt + NumPad1         x10\n"\
                     "    Alt + NumPad0         x12"
#define RIGHT_MESSAGE "\n"\
                     "  ----------------- Forward time by -----------------\n"\
                     "          NumPad+         1 hour\n"\
                     "   Ctrl + NumPad+         2 hours\n"\
                     "  Shift + NumPad+         3 hours\n"\
                     "    Alt + NumPad+         4 hours\n\n"\
                     "  ----------------- Backward time by ----------------\n"\
                     "          NumPad-         1 hour\n"\
                     "   Ctrl + NumPad-         2 hours\n"\
                     "  Shift + NumPad-         3 hours\n"\
                     "    Alt + NumPad-         4 hours\n\n"\
                     "  ------------- Set time to and stop it -------------\n"\
                     "   Ctrl + NumPad0         00:00\n"\
                     "   Ctrl + NumPad1         10:00\n"\
                     "   Ctrl + NumPad2         12:00\n"\
                     "   Ctrl + NumPad3         13:00\n"\
                     "   Ctrl + NumPad4         14:00\n"\
                     "   Ctrl + NumPad5         15:00\n"\
                     "   Ctrl + NumPad6         6:00\n"\
                     "   Ctrl + NumPad7         17:00\n"\
                     "   Ctrl + NumPad8         18:00\n"\
                     "   Ctrl + NumPad9         9:00"

DWORD get_PID(CHAR *PrName);

DWORD_PTR GetModuleBase(char *lpModuleName, DWORD dwProcessId);

BOOL PatchEx(LPVOID dst_addr, LPCVOID src_addr, SIZE_T size, SIZE_T *bytes_written);

void print_hex(char *str, int len);

void print_process_memory(DWORD_PTR addr, SIZE_T size);

DWORD_PTR search_process_memory(DWORD_PTR StartAddress, char *bytes, SIZE_T size);

BOOL patch_process_memory(DWORD_PTR dst_addr, char* src_addr, SIZE_T size);

void message_box(char* message, UINT uType);

void inc_time(float *curr_time, float step, BOOL h_round);

BOOL get_time(float *time);

BOOL set_time(float *time);

BOOL start_time();

BOOL stop_time();

BOOL shift_time(float *time, float step);

BOOL set_time_rate(float *time, unsigned char rate_factor, BOOL sync_real_time);

float get_local_time();

int init_memory();

void restore_memory();

#endif // TIME_CONTROL_H_INCLUDED
