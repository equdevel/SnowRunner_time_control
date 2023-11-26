#include <stdio.h>
#include <Windows.h>
#include <tlhelp32.h>
#include <stdint.h>


DWORD get_PID(CHAR * PrName)
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            //printf("%s\n", entry.szExeFile);
            if (strcmp(entry.szExeFile, PrName) == 0)
            {
                return entry.th32ProcessID;
            }
        }
    }
    CloseHandle(snapshot);
    return NULL;
}

DWORD_PTR GetModuleBase(char *lpModuleName, DWORD dwProcessId)
{
    MODULEENTRY32 lpModuleEntry = { 0 };
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);

    if (!hSnapShot)
        return NULL;
    lpModuleEntry.dwSize = sizeof(lpModuleEntry);
    BOOL bModule = Module32First(hSnapShot, &lpModuleEntry);
    while (bModule)
    {
        if (!strcmp(lpModuleEntry.szModule, lpModuleName))
        {
            CloseHandle(hSnapShot);
            return (DWORD_PTR)lpModuleEntry.modBaseAddr;
        }
        bModule = Module32Next(hSnapShot, &lpModuleEntry);
    }
    CloseHandle(hSnapShot);
    return NULL;
}

BOOL PatchEx(HANDLE hProcess, LPVOID dst, LPCVOID src, SIZE_T size, SIZE_T *BytesWritten)
{
    DWORD oldprotect;
    VirtualProtectEx(hProcess, dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
    BOOL result = WriteProcessMemory(hProcess, dst, src, size, BytesWritten);
    VirtualProtectEx(hProcess, dst, size, oldprotect, &oldprotect);
    return result;
}

void print_hex(char *str, int len) {
    for(int i=0; i<len; i++){
        printf("%02x", (unsigned char)str[i]);
    }
}

void print_memory(HANDLE hProcess, DWORD_PTR pBuffer, SIZE_T len) {
    SIZE_T bytes_read = 0;
    char local_buffer[len];
    ReadProcessMemory(hProcess, (void*)pBuffer, &local_buffer, len, &bytes_read);
    printf("\nBuffer: ");
    print_hex(local_buffer, len);
    printf("\nRead bytes = %d\n\n", bytes_read);
}

void main()
{
    DWORD PID;
    HANDLE hProcess;
    DWORD_PTR BaseAddress;
    char *PrName = "SnowRunner.exe";
    if (!(PID = get_PID(PrName)))
    {
        printf("Process does not exist\n");
        system("pause");
        return;
    }
    printf("Process founded!\n");
    printf("Process name: %s\n", PrName);
    printf("PID: %d\n\n", PID);

    if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID)))
    {
        printf("OpenProcess error\n");
        return;
    }

    printf("OpenProcess is ok\n");
    printf("Now we have handle of process %s\n", PrName);
    printf("Handle: %d\n\n", hProcess);


    if (!(BaseAddress = GetModuleBase(PrName, PID)))
    {
        printf("GetModuleBase error\n");
        return;
    }
    printf("GetModuleBase is ok\n");
    printf("BaseAddress: %llx\n\n", BaseAddress);

    DWORD_PTR pBuffer = BaseAddress + 0xA8C306;
    printf("pBuffer: %llx\n", pBuffer);

    print_memory(hProcess, pBuffer, 9);

    //INJECTION
    char new_buffer[9] = "\x90\x90\x90\x90\x90\x90\x90\x90\x90";

    //HANDLE hProcThread;
    //DWORD_PTR pInjectedBuffer = (DWORD_PTR)VirtualAllocEx(hProcess, NULL, 9, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    SIZE_T bytes_written;
    //BOOL result = WriteProcessMemory(hProcess, (LPVOID)pInjectedBuffer, new_buffer, 12, &bytes_written);
    BOOL result = PatchEx(hProcess, (LPVOID)pBuffer, new_buffer, 9, &bytes_written);
    //hProcThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)pInjectedFunction, NULL, NULL, NULL);
    printf("Result code = %d\n", result);
    printf("Written bytes = %d\n", bytes_written);
    printf("Last error = %d\n", GetLastError());

    print_memory(hProcess, pBuffer, 9);

    system("pause");
}
