#include "algorithm_thread.h"
#include "windows-specific/framework.h"
#include "windows-specific/Resource.h"
#include "../algorithms/all.h"



extern std::atomic<bool> should_continue_global;
extern HINSTANCE hinstance;
extern HWND hwnd;



INT_PTR CALLBACK about_callbacks(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}



LRESULT CALLBACK window_callbacks(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_CREDITS:
            DialogBox(hinstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, about_callbacks);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case IDM_SELECTION_SORT:
            algorithm_thread::assign_sort(selection_sort);
            break;
        case IDM_INSERTION_SORT:
            algorithm_thread::assign_sort(insertion_sort);
            break;
        case IDM_BUBBLE_SORT:
            algorithm_thread::assign_sort(bubble_sort);
            break;
        case IDM_STD_SORT_HEAP:
            algorithm_thread::assign_sort(std_sort_heap);
            break;
        case IDM_STD_STABLE_SORT:
            algorithm_thread::assign_sort(std_stable_sort);
            break;
        case IDM_STD_SORT:
            algorithm_thread::assign_sort(std_sort);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        should_continue_global.store(false, std::memory_order_release);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}