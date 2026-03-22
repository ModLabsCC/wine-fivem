/* WinRT Windows.UI.Xaml Implementation
 *
 * Copyright (C) 2025 Mohamad Al-Jaf
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "initguid.h"
#include <wchar.h>
#include <stdio.h>
#include <stdarg.h>
#include <wctype.h>
#include "private.h"

WINE_DEFAULT_DEBUG_CHANNEL(xaml);

static void wine_xaml_log(const char *fmt, ...)
{
    FILE *f = fopen("/tmp/xaml_debug.log", "a");
    if (f) {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(f, fmt, ap);
        va_end(ap);
        fflush(f);
        fclose(f);
    }
}

HRESULT WINAPI DllGetActivationFactory( HSTRING classid, IActivationFactory **factory )
{
    const WCHAR *buffer = WindowsGetStringRawBuffer( classid, NULL );

    ERR( "==== XAML DllGetActivationFactory ==== class %s, factory %p.\n", debugstr_hstring(classid), factory );
    wine_xaml_log("DllGetActivationFactory called\n");

    *factory = NULL;

    if (!wcscmp( buffer, RuntimeClass_Windows_UI_ColorHelper ))
        IActivationFactory_QueryInterface( color_helper_factory, &IID_IActivationFactory, (void **)factory );
    else if (!wcscmp( buffer, L"Windows.UI.Xaml.Hosting.WindowsXamlManager" ))
        IActivationFactory_QueryInterface( windows_xaml_manager_factory, &IID_IActivationFactory, (void **)factory );
    else if (!wcscmp( buffer, L"Windows.UI.Xaml.Hosting.DesktopWindowXamlSource" ))
        IActivationFactory_QueryInterface( desktop_window_xaml_source_factory, &IID_IActivationFactory, (void **)factory );
    else if (!wcscmp( buffer, L"Windows.UI.Xaml.Markup.XamlReader" ))
        IActivationFactory_QueryInterface( xaml_reader_factory, &IID_IActivationFactory, (void **)factory );

    if (*factory) return S_OK;
    return CLASS_E_CLASSNOTAVAILABLE;
}

WCHAR wine_xaml_stub_text[4096] = L"Waiting for XAML UI...";
HWND wine_xaml_stub_hwnd = NULL;

static void draw_path_logo(HDC hdc, struct wine_xaml_node *node, RECT *rc)
{
    const WCHAR *p = node->text;
    POINT pts[128];
    int count = 0;
    float x = 0, y = 0;
    float scale = (rc->bottom - rc->top) / 200.0f; /* FiveM logo is ~200 units high */
    int offset_x = rc->left + (rc->right - rc->left) / 2 - (int)(85.0f * scale);
    int offset_y = rc->top;
    DWORD color = node->has_fill ? node->fill_color : RGB(255, 255, 255);
    HBRUSH br = CreateSolidBrush(color);
    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HGDIOBJ old_br = SelectObject(hdc, br);
    HGDIOBJ old_pen = SelectObject(hdc, pen);
    
    wine_xaml_log("Drawing path logo with color 0x%08lx (has_fill=%d, fill_color=0x%08lx)\n", 
        (unsigned long)color, node->has_fill, (unsigned long)node->fill_color);

    while (*p) {
        while (*p == ' ') p++;
        if (!*p) break;
        
        if (*p == 'M' || *p == 'L') {
            p++;
            if (swscanf(p, L"%f %f", &x, &y) == 2) {
                pts[count].x = offset_x + (int)(x * scale);
                pts[count].y = offset_y + (int)(y * scale);
                count++;
            }
            while (*p && (iswdigit(*p) || *p == '.' || *p == '-' || *p == ' ')) p++;
        } else if (*p == 'H') {
            p++;
            if (swscanf(p, L"%f", &x) == 1) {
                pts[count].x = offset_x + (int)(x * scale);
                pts[count].y = count > 0 ? pts[count-1].y : offset_y;
                count++;
            }
            while (*p && (iswdigit(*p) || *p == '.' || *p == '-' || *p == ' ')) p++;
        } else if (*p == 'Z') {
            if (count > 2) Polygon(hdc, pts, count);
            count = 0;
            p++;
        } else p++;
    }
    if (count > 2) Polygon(hdc, pts, count);

    SelectObject(hdc, old_br);
    SelectObject(hdc, old_pen);
    DeleteObject(br);
    DeleteObject(pen);
}

static void draw_progress_bar(HDC hdc, struct wine_xaml_node *node, RECT *rc)
{
    RECT inner = *rc;
    HBRUSH bg = CreateSolidBrush(RGB(40, 44, 52));
    HBRUSH fg = CreateSolidBrush(RGB(255, 255, 255));
    int width = (rc->right - rc->left) / 2;
    int x_off = (rc->right - rc->left - width) / 2;

    inner.left += x_off;
    inner.right = inner.left + width;
    inner.top += (rc->bottom - rc->top) / 2 - 5;
    inner.bottom = inner.top + 10;
    
    FillRect(hdc, &inner, bg);
    
    if (node->progress_max > 0) {
        int fill_w = (int)(width * (node->progress_value / node->progress_max));
        if (fill_w < 0) fill_w = 0;
        if (fill_w > width) fill_w = width;
        inner.right = inner.left + fill_w;
        FillRect(hdc, &inner, fg);
    }
    
    DeleteObject(bg);
    DeleteObject(fg);
}

static void draw_xaml_node(HDC hdc, struct wine_xaml_node *node, RECT *rc)
{
    RECT child_rc;
    int i, stack_y;
    if (!node) return;
    
    if (node->has_bg) {
        HBRUSH br = CreateSolidBrush(node->bg_color);
        FillRect(hdc, rc, br);
        DeleteObject(br);
    }
    
    if (node->type == XAML_NODE_TEXTBLOCK) {
        const WCHAR *t = (wcslen(node->text) > 0 && node->text[0] != ' ') ? node->text : wine_xaml_stub_text;
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, node->has_fg ? node->fg_color : RGB(255, 255, 255));
        DrawTextW(hdc, t, -1, rc, DT_CENTER | DT_VCENTER | DT_WORDBREAK);
    }
    
    if (node->type == XAML_NODE_IMAGE && wcslen(node->text) > 0) {
        HBRUSH im_br = CreateSolidBrush(RGB(60, 60, 80)); /* Bluish gray placeholder */
        FillRect(hdc, rc, im_br);
        DeleteObject(im_br);
        
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 100)); /* Yellow text for image URL */
        DrawTextW(hdc, node->text, -1, rc, DT_CENTER | DT_VCENTER | DT_WORDBREAK);
    }

    if (node->type == XAML_NODE_PATH) {
        draw_path_logo(hdc, node, rc);
    }

    if (node->type == XAML_NODE_PROGRESSBAR) {
        draw_progress_bar(hdc, node, rc);
    }

    if (node->type == XAML_NODE_SWAPCHAINPANEL) {
        /* This is the Overlay. Blue placeholder. */
        HBRUSH sw_br = CreateSolidBrush(RGB(22, 25, 35)); /* #161923 dark blue */
        FillRect(hdc, rc, sw_br);
        DeleteObject(sw_br);
    }
    
    if (node->child_count > 0) {
        child_rc = *rc;
        
        if (node->type == XAML_NODE_STACKPANEL) {
            stack_y = rc->top;
            for (i = 0; i < node->child_count; i++) {
                int child_h = (int)node->children[i]->height;
                if (child_h <= 0) {
                    if (node->children[i]->type == XAML_NODE_TEXTBLOCK) child_h = 30;
                    else if (node->children[i]->type == XAML_NODE_PROGRESSBAR) child_h = 40;
                    else child_h = (rc->bottom - stack_y) / (node->child_count - i);
                }
                
                child_rc.top = stack_y;
                child_rc.bottom = stack_y + child_h;
                if (child_rc.bottom > rc->bottom) child_rc.bottom = rc->bottom;
                stack_y = child_rc.bottom;
                
                draw_xaml_node(hdc, node->children[i], &child_rc);
            }
        } else {
            /* Default: Overlap for Grid and other containers */
            for (i = 0; i < node->child_count; i++) {
                draw_xaml_node(hdc, node->children[i], &child_rc);
            }
        }
    }
}

static LRESULT CALLBACK XamlStubWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_CREATE)
    {
        wine_xaml_stub_hwnd = hwnd;
        wine_xaml_log("WM_CREATE hwnd=%p\n", hwnd);
    }
    else if (msg == WM_SIZE)
    {
        wine_xaml_log("WM_SIZE hwnd=%p size=%dx%d\n", hwnd, LOWORD(lp), HIWORD(lp));
        InvalidateRect(hwnd, NULL, TRUE);
    }
    else if (msg == WM_PAINT)
    {
        PAINTSTRUCT ps;
        HDC hdc;
        RECT rc;

        hdc = BeginPaint(hwnd, &ps);
        GetClientRect(hwnd, &rc);
        wine_xaml_log("WM_PAINT hwnd=%p root_node=%p rc=(%ld,%ld,%ld,%ld) text='%ls'\n", 
            hwnd, wine_xaml_root_node, rc.left, rc.top, rc.right, rc.bottom, wine_xaml_stub_text);
        
        if (wine_xaml_root_node) {
            draw_xaml_node(hdc, wine_xaml_root_node, &rc);
        } else {
            HBRUSH bgBrush = CreateSolidBrush(RGB(20, 20, 20)); /* Very dark gray */
            FillRect(hdc, &rc, bgBrush);
            DeleteObject(bgBrush);
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(255, 255, 255));
            DrawTextW(hdc, wine_xaml_stub_text, -1, &rc, DT_CENTER | DT_VCENTER | DT_WORDBREAK);
        }
        
        EndPaint(hwnd, &ps);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

BOOL WINAPI DllMain( HINSTANCE inst, DWORD reason, void *reserved )
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        WNDCLASSW wc = {0};
        DisableThreadLibraryCalls( inst );
        wc.style = CS_GLOBALCLASS;
        wc.lpfnWndProc = XamlStubWndProc;
        wc.hInstance = inst;
        wc.hCursor = LoadCursorW( NULL, (LPCWSTR)IDC_ARROW );
        wc.hbrBackground = CreateSolidBrush(RGB(20, 20, 20));
        wc.lpszClassName = L"WineXamlStubClass";
        RegisterClassW( &wc );
        wine_xaml_log("DllMain: Registered WineXamlStubClass\n");
    }
    return TRUE;
}

HRESULT WINAPI CreateXamlUIPresenter(void *site, void **presenter)
{
    FIXME("site %p, presenter %p - returning S_OK fake\n", site, presenter);
    if (!presenter) return E_POINTER;
    *presenter = (void*)0xdeadbeef; /* Return a non-null dummy pointer */
    return S_OK;
}
