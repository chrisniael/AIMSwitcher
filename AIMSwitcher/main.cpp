#include <iostream>
#include <string>

#include <Windows.h>

#pragma comment(lib, "imm32.lib")

#define IMC_GETCONVERSIONMODE 0x01
#define IMC_SETCONVERSIONMODE 0x02

int GetCurrentInputMethod() {
    HWND hWnd = GetForegroundWindow();
    DWORD threadId = GetWindowThreadProcessId(hWnd, nullptr);
    HKL hKL = GetKeyboardLayout(threadId);
    uintptr_t langId = reinterpret_cast<uintptr_t>(hKL) & 0xFFFF;
    return static_cast<int>(langId);
}

void SwitchInputMethod(int langId) {
    HWND hWnd = GetForegroundWindow();
    PostMessage(hWnd, WM_INPUTLANGCHANGEREQUEST, 0, static_cast<LPARAM>(langId));
}

int GetCurrentInputMethodMode() {
    HWND hWnd = GetForegroundWindow();
    HWND hIMEWnd = ImmGetDefaultIMEWnd(hWnd);
    LRESULT mode = SendMessage(hIMEWnd, WM_IME_CONTROL, IMC_GETCONVERSIONMODE, 0);
    return static_cast<int>(mode);
}

void SwitchInputMethodMode(int mode) {
    HWND hWnd = GetForegroundWindow();
    HWND hIMEWnd = ImmGetDefaultIMEWnd(hWnd);
    SendMessage(hIMEWnd, WM_IME_CONTROL, IMC_SETCONVERSIONMODE, static_cast<LPARAM>(mode));
}

void ShowUsage() {
    std::cout
        << "AIMSwitcher, An Input Method Switcher\n"
        << "Usage: AIMSwitcher [OPTION]... [ARG]...\n"
        << "\n"
        << "  --im [INPUTMETHOD]                              show the current input method if INPUTMETHOD is omitted,\n"
        << "                                                  otherwise switch to the specified input method\n"
        << "  --imm [INPUTMETHODMODE]                         show the current input method mode if INPUTMETHODMODE is omitted,\n"
        << "                                                  otherwise switch to the specified input method mode\n"
        << "  --immt [INPUTMETHODMODE1] [INPUTMETHODMODE2]    toggle current input method mode between INPUTMETHODMODE1 and INPUTMETHODMODE2"
        << std::endl;
}

// ȫ�ֹ��Ӿ��
HHOOK hKeyboardHook;

// �ص��������ڲ���ָ���Ŀ�ݼ����ʱִ��
void OnWinSpacePressed() {
    std::cout << "Win + Space pressed!" << std::endl;
    // �ڴ˴������ϣ��ִ�еĺ����߼�

    int mode1 = 0;
    int mode2 = 1025;
    int curMode = GetCurrentInputMethodMode();
    if (curMode == mode1) {
        SwitchInputMethodMode(mode2);
    }
    else {
        SwitchInputMethodMode(mode1);
    }
}

// �ͼ����̹��ӻص�����
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;

        // ����Ƿ�����Win��
        bool bIsWinPressed = (GetAsyncKeyState(VK_LWIN) & 0x8000) || (GetAsyncKeyState(VK_RWIN) & 0x8000);

        // ����Ƿ�����Space��
        bool bIsSpacePressed = (pKeyboard->vkCode == VK_SPACE);

        if (bIsWinPressed && bIsSpacePressed) {
            OnWinSpacePressed();
            return 1; // ĳЩ��ϼ�������Ҫ����1�Է�ֹ���ݵ����������ϵͳ
        }
    }
    return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

int main(int argc, char** argv) {
    // ����ȫ�ֵͼ����̹���
    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
    if (!hKeyboardHook) {
        std::cerr << "Failed to install hook!" << std::endl;
        return 1;
    }

    std::cout << "Hook installed. Press Win + Space to trigger the function. Press Ctrl+C to exit." << std::endl;

    // ��Ϣѭ��
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // ж�ع���
    UnhookWindowsHookEx(hKeyboardHook);

    return 0;
}
