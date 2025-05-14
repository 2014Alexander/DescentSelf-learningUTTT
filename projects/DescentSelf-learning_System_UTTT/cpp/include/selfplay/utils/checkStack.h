// checkStack.h
#pragma once
#include <windows.h>
#include <iostream>

// Определение типа функции GetCurrentThreadStackLimits
typedef VOID (WINAPI *GetCurrentThreadStackLimitsFunc)(PULONG_PTR, PULONG_PTR);

size_t getFreeStackSpace() {
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    GetCurrentThreadStackLimitsFunc pGetCurrentThreadStackLimits = nullptr;
    if (hKernel32)
        pGetCurrentThreadStackLimits = reinterpret_cast<GetCurrentThreadStackLimitsFunc>(
            GetProcAddress(hKernel32, "GetCurrentThreadStackLimits")
        );

    ULONG_PTR stackLow = 0, stackHigh = 0;
    char dummy;
    ULONG_PTR currentSP = reinterpret_cast<ULONG_PTR>(&dummy);

    if (pGetCurrentThreadStackLimits) {
        // Получаем границы стека текущего потока
        pGetCurrentThreadStackLimits(&stackLow, &stackHigh);
        // Расчет свободного места: разница между верхней границей и текущим указателем стека
        return stackHigh - currentSP;
    } else {
        // Если функция недоступна, вернуть 0 (или можно попробовать использовать VirtualQuery)
        return 0;
    }
}
