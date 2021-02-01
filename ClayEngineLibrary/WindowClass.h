#pragma once

#include "ClayEngine.h"

_CE_BEGIN
using RectPtr = std::unique_ptr<RECT>;

class WindowClass
{
    HWND hWnd = nullptr;
    WNDCLASSEXW wcex{};
    RectPtr rect;

public:
    WindowClass(HINSTANCE hInstance, WNDPROC fn, String title);
    ~WindowClass() = default;

    void SetClientRect(const LPRECT rectPtr);

    const HWND GetWindowHandle() const { return hWnd; }
    const int GetWindowWidth() { return rect->right - rect->left; }
    const int GetWindowHeight() { return rect->bottom - rect->top; }
};
using WindowClassPtr = std::shared_ptr<WindowClass>;
_CE_END
