/*
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
#define COBJMACROS
#include "initguid.h"
#include <stdarg.h>

#include "windef.h"
#include "winbase.h"
#include "winstring.h"

#include "roapi.h"

#define WIDL_using_Windows_Foundation
#define WIDL_using_Windows_Foundation_Collections
#include "windows.foundation.h"
#define WIDL_using_Windows_UI
#include "windows.ui.h"
#include "windows.ui.xaml.hosting.desktopwindowxamlsource.h"

#include "wine/test.h"

DEFINE_GUID( IID_IWindowsXamlManager, 0x56096c31, 0x1aa0, 0x5288, 0x88, 0x18, 0x6e, 0x74, 0xa2, 0xdc, 0xaf, 0xf5 );
DEFINE_GUID( IID_IWindowsXamlManagerStatics, 0x28258a12, 0x7d82, 0x505b, 0xb2, 0x10, 0x71, 0x2b, 0x04, 0xa5, 0x88, 0x82 );

typedef IInspectable IWindowsXamlManager;

typedef struct IWindowsXamlManagerStatics IWindowsXamlManagerStatics;
typedef struct IWindowsXamlManagerStaticsVtbl
{
    BEGIN_INTERFACE

    HRESULT (WINAPI *QueryInterface)( IWindowsXamlManagerStatics *iface, REFIID iid, void **out );
    ULONG (WINAPI *AddRef)( IWindowsXamlManagerStatics *iface );
    ULONG (WINAPI *Release)( IWindowsXamlManagerStatics *iface );
    HRESULT (WINAPI *GetIids)( IWindowsXamlManagerStatics *iface, ULONG *iid_count, IID **iids );
    HRESULT (WINAPI *GetRuntimeClassName)( IWindowsXamlManagerStatics *iface, HSTRING *class_name );
    HRESULT (WINAPI *GetTrustLevel)( IWindowsXamlManagerStatics *iface, TrustLevel *trust_level );
    HRESULT (WINAPI *InitializeForCurrentThread)( IWindowsXamlManagerStatics *iface, IWindowsXamlManager **manager );

    END_INTERFACE
} IWindowsXamlManagerStaticsVtbl;

struct IWindowsXamlManagerStatics
{
    const IWindowsXamlManagerStaticsVtbl *lpVtbl;
};

#define IWindowsXamlManagerStatics_InitializeForCurrentThread( iface, manager ) \
    ((iface)->lpVtbl->InitializeForCurrentThread( iface, manager ))

DEFINE_GUID( IID_IDesktopWindowXamlSource, 0xd585bfe1, 0x00ff, 0x51be, 0xba, 0x1d, 0xa1, 0x32, 0x99, 0x56, 0xea, 0x0a );
DEFINE_GUID( IID_IDesktopWindowXamlSourceFactory, 0x5cd61dc0, 0x2561, 0x56e1, 0x8e, 0x75, 0x6e, 0x44, 0x17, 0x38, 0x05, 0xe3 );

DEFINE_GUID( IID_IXamlReaderStatics, 0x9891c6bd, 0x534f, 0x4955, 0xb8, 0x5a, 0x8a, 0x8d, 0xc0, 0xdc, 0xa6, 0x02 );
DEFINE_GUID( IID_IFrameworkElement, 0xa391d09b, 0x4a99, 0x4b7c, 0x9d, 0x8d, 0x6f, 0xa5, 0xd0, 0x1f, 0x6f, 0xbf );
DEFINE_GUID( IID_IUIElement, 0x676d0be9, 0xb65c, 0x41c6, 0xba, 0x40, 0x58, 0xcf, 0x87, 0xf2, 0x01, 0xc1 );
DEFINE_GUID( IID_IProgressBar, 0xae752c89, 0x0067, 0x4963, 0xbf, 0x4c, 0x29, 0xdb, 0x0c, 0x4a, 0x50, 0x7e );
DEFINE_GUID( IID_IRangeBase, 0xfa002c1a, 0x494e, 0x46cf, 0x91, 0xd4, 0xe1, 0x4a, 0x8d, 0x79, 0x86, 0x75 );
DEFINE_GUID( IID_ITextBlock, 0xae2d9271, 0x3b4a, 0x45fc, 0x84, 0x68, 0xf7, 0x94, 0x95, 0x48, 0xf4, 0xd5 );
DEFINE_GUID( IID_ISwapChainPanel, 0xc589644f, 0xeba8, 0x427a, 0xb7, 0x5a, 0x9f, 0x1f, 0x93, 0xa1, 0x1a, 0xe9 );
DEFINE_GUID( IID_ISwapChainPanelNative, 0xf92f19d2, 0x3ade, 0x45a6, 0xa2, 0x0c, 0xf6, 0xf1, 0xea, 0x90, 0x55, 0x4b );

typedef struct IXamlReaderStatics IXamlReaderStatics;
typedef struct IXamlReaderStaticsVtbl
{
    BEGIN_INTERFACE
    HRESULT (WINAPI *QueryInterface)( IXamlReaderStatics *iface, REFIID iid, void **out );
    ULONG (WINAPI *AddRef)( IXamlReaderStatics *iface );
    ULONG (WINAPI *Release)( IXamlReaderStatics *iface );
    HRESULT (WINAPI *GetIids)( IXamlReaderStatics *iface, ULONG *iid_count, IID **iids );
    HRESULT (WINAPI *GetRuntimeClassName)( IXamlReaderStatics *iface, HSTRING *class_name );
    HRESULT (WINAPI *GetTrustLevel)( IXamlReaderStatics *iface, TrustLevel *trust_level );
    HRESULT (WINAPI *Load)( IXamlReaderStatics *iface, HSTRING xaml, IInspectable **value );
    HRESULT (WINAPI *LoadWithInitialTemplateValidation)( IXamlReaderStatics *iface, HSTRING xaml, IInspectable **value );
    END_INTERFACE
} IXamlReaderStaticsVtbl;

struct IXamlReaderStatics
{
    const IXamlReaderStaticsVtbl *lpVtbl;
};

#define IXamlReaderStatics_Load( iface, xaml, value ) ((iface)->lpVtbl->Load( iface, xaml, value ))

typedef struct IFrameworkElement IFrameworkElement;
typedef struct IFrameworkElementVtbl
{
    BEGIN_INTERFACE
    HRESULT (WINAPI *QueryInterface)( IFrameworkElement *iface, REFIID iid, void **out );
    ULONG (WINAPI *AddRef)( IFrameworkElement *iface );
    ULONG (WINAPI *Release)( IFrameworkElement *iface );
    HRESULT (WINAPI *GetIids)( IFrameworkElement *iface, ULONG *iid_count, IID **iids );
    HRESULT (WINAPI *GetRuntimeClassName)( IFrameworkElement *iface, HSTRING *class_name );
    HRESULT (WINAPI *GetTrustLevel)( IFrameworkElement *iface, TrustLevel *trust_level );
    HRESULT (WINAPI *get_Triggers)( IFrameworkElement *iface, IInspectable **value );
    HRESULT (WINAPI *get_Resources)( IFrameworkElement *iface, IInspectable **value );
    HRESULT (WINAPI *put_Resources)( IFrameworkElement *iface, IInspectable *value );
    HRESULT (WINAPI *get_Tag)( IFrameworkElement *iface, IInspectable **value );
    HRESULT (WINAPI *put_Tag)( IFrameworkElement *iface, IInspectable *value );
    HRESULT (WINAPI *get_Language)( IFrameworkElement *iface, HSTRING *value );
    HRESULT (WINAPI *put_Language)( IFrameworkElement *iface, HSTRING value );
    HRESULT (WINAPI *get_ActualWidth)( IFrameworkElement *iface, DOUBLE *value );
    HRESULT (WINAPI *get_ActualHeight)( IFrameworkElement *iface, DOUBLE *value );
    HRESULT (WINAPI *get_Width)( IFrameworkElement *iface, DOUBLE *value );
    HRESULT (WINAPI *put_Width)( IFrameworkElement *iface, DOUBLE value );
    HRESULT (WINAPI *get_Height)( IFrameworkElement *iface, DOUBLE *value );
    HRESULT (WINAPI *put_Height)( IFrameworkElement *iface, DOUBLE value );
    HRESULT (WINAPI *get_MinWidth)( IFrameworkElement *iface, DOUBLE *value );
    HRESULT (WINAPI *put_MinWidth)( IFrameworkElement *iface, DOUBLE value );
    HRESULT (WINAPI *get_MaxWidth)( IFrameworkElement *iface, DOUBLE *value );
    HRESULT (WINAPI *put_MaxWidth)( IFrameworkElement *iface, DOUBLE value );
    HRESULT (WINAPI *get_MinHeight)( IFrameworkElement *iface, DOUBLE *value );
    HRESULT (WINAPI *put_MinHeight)( IFrameworkElement *iface, DOUBLE value );
    HRESULT (WINAPI *get_MaxHeight)( IFrameworkElement *iface, DOUBLE *value );
    HRESULT (WINAPI *put_MaxHeight)( IFrameworkElement *iface, DOUBLE value );
    HRESULT (WINAPI *get_HorizontalAlignment)( IFrameworkElement *iface, INT32 *value );
    HRESULT (WINAPI *put_HorizontalAlignment)( IFrameworkElement *iface, INT32 value );
    HRESULT (WINAPI *get_VerticalAlignment)( IFrameworkElement *iface, INT32 *value );
    HRESULT (WINAPI *put_VerticalAlignment)( IFrameworkElement *iface, INT32 value );
    HRESULT (WINAPI *get_Margin)( IFrameworkElement *iface, void *value );
    HRESULT (WINAPI *put_Margin)( IFrameworkElement *iface, const void *value );
    HRESULT (WINAPI *get_Name)( IFrameworkElement *iface, HSTRING *value );
    HRESULT (WINAPI *put_Name)( IFrameworkElement *iface, HSTRING value );
    HRESULT (WINAPI *get_BaseUri)( IFrameworkElement *iface, IInspectable **value );
    HRESULT (WINAPI *get_DataContext)( IFrameworkElement *iface, IInspectable **value );
    HRESULT (WINAPI *put_DataContext)( IFrameworkElement *iface, IInspectable *value );
    HRESULT (WINAPI *get_Style)( IFrameworkElement *iface, IInspectable **value );
    HRESULT (WINAPI *put_Style)( IFrameworkElement *iface, IInspectable *value );
    HRESULT (WINAPI *get_Parent)( IFrameworkElement *iface, IInspectable **value );
    HRESULT (WINAPI *get_FlowDirection)( IFrameworkElement *iface, INT32 *value );
    HRESULT (WINAPI *put_FlowDirection)( IFrameworkElement *iface, INT32 value );
    HRESULT (WINAPI *add_Loaded)( IFrameworkElement *iface, IInspectable *handler, EventRegistrationToken *token );
    HRESULT (WINAPI *remove_Loaded)( IFrameworkElement *iface, EventRegistrationToken token );
    HRESULT (WINAPI *add_Unloaded)( IFrameworkElement *iface, IInspectable *handler, EventRegistrationToken *token );
    HRESULT (WINAPI *remove_Unloaded)( IFrameworkElement *iface, EventRegistrationToken token );
    HRESULT (WINAPI *add_SizeChanged)( IFrameworkElement *iface, IInspectable *handler, EventRegistrationToken *token );
    HRESULT (WINAPI *remove_SizeChanged)( IFrameworkElement *iface, EventRegistrationToken token );
    HRESULT (WINAPI *add_LayoutUpdated)( IFrameworkElement *iface, IInspectable *handler, EventRegistrationToken *token );
    HRESULT (WINAPI *remove_LayoutUpdated)( IFrameworkElement *iface, EventRegistrationToken token );
    HRESULT (WINAPI *FindName)( IFrameworkElement *iface, HSTRING name, IInspectable **result );
    HRESULT (WINAPI *SetBinding)( IFrameworkElement *iface, IInspectable *dp, IInspectable *binding, IInspectable **result );
    END_INTERFACE
} IFrameworkElementVtbl;

struct IFrameworkElement
{
    const IFrameworkElementVtbl *lpVtbl;
};

#define IFrameworkElement_FindName( iface, name, result ) ((iface)->lpVtbl->FindName( iface, name, result ))

typedef struct ITextBlock ITextBlock;
typedef struct ITextBlockVtbl
{
    BEGIN_INTERFACE
    HRESULT (WINAPI *QueryInterface)( ITextBlock *iface, REFIID iid, void **out );
    ULONG (WINAPI *AddRef)( ITextBlock *iface );
    ULONG (WINAPI *Release)( ITextBlock *iface );
    HRESULT (WINAPI *GetIids)( ITextBlock *iface, ULONG *iid_count, IID **iids );
    HRESULT (WINAPI *GetRuntimeClassName)( ITextBlock *iface, HSTRING *class_name );
    HRESULT (WINAPI *GetTrustLevel)( ITextBlock *iface, TrustLevel *trust_level );
    HRESULT (WINAPI *get_FontSize)( ITextBlock *iface, DOUBLE *value );
    HRESULT (WINAPI *put_FontSize)( ITextBlock *iface, DOUBLE value );
    HRESULT (WINAPI *get_FontFamily)( ITextBlock *iface, IInspectable **value );
    HRESULT (WINAPI *put_FontFamily)( ITextBlock *iface, IInspectable *value );
    HRESULT (WINAPI *get_FontWeight)( ITextBlock *iface, UINT32 *value );
    HRESULT (WINAPI *put_FontWeight)( ITextBlock *iface, UINT32 value );
    HRESULT (WINAPI *get_FontStyle)( ITextBlock *iface, INT32 *value );
    HRESULT (WINAPI *put_FontStyle)( ITextBlock *iface, INT32 value );
    HRESULT (WINAPI *get_FontStretch)( ITextBlock *iface, INT32 *value );
    HRESULT (WINAPI *put_FontStretch)( ITextBlock *iface, INT32 value );
    HRESULT (WINAPI *get_CharacterSpacing)( ITextBlock *iface, INT32 *value );
    HRESULT (WINAPI *put_CharacterSpacing)( ITextBlock *iface, INT32 value );
    HRESULT (WINAPI *get_Foreground)( ITextBlock *iface, IInspectable **value );
    HRESULT (WINAPI *put_Foreground)( ITextBlock *iface, IInspectable *value );
    HRESULT (WINAPI *get_TextWrapping)( ITextBlock *iface, INT32 *value );
    HRESULT (WINAPI *put_TextWrapping)( ITextBlock *iface, INT32 value );
    HRESULT (WINAPI *get_TextTrimming)( ITextBlock *iface, INT32 *value );
    HRESULT (WINAPI *put_TextTrimming)( ITextBlock *iface, INT32 value );
    HRESULT (WINAPI *get_TextAlignment)( ITextBlock *iface, INT32 *value );
    HRESULT (WINAPI *put_TextAlignment)( ITextBlock *iface, INT32 value );
    HRESULT (WINAPI *get_Text)( ITextBlock *iface, HSTRING *value );
    HRESULT (WINAPI *put_Text)( ITextBlock *iface, HSTRING value );
    END_INTERFACE
} ITextBlockVtbl;

struct ITextBlock
{
    const ITextBlockVtbl *lpVtbl;
};

#define ITextBlock_Release( iface ) ((iface)->lpVtbl->Release( iface ))
#define ITextBlock_put_Text( iface, value ) ((iface)->lpVtbl->put_Text( iface, value ))
typedef struct IRangeBase IRangeBase;
typedef struct IRangeBaseVtbl
{
    BEGIN_INTERFACE
    HRESULT (WINAPI *QueryInterface)( IRangeBase *iface, REFIID iid, void **out );
    ULONG (WINAPI *AddRef)( IRangeBase *iface );
    ULONG (WINAPI *Release)( IRangeBase *iface );
    HRESULT (WINAPI *GetIids)( IRangeBase *iface, ULONG *iid_count, IID **iids );
    HRESULT (WINAPI *GetRuntimeClassName)( IRangeBase *iface, HSTRING *class_name );
    HRESULT (WINAPI *GetTrustLevel)( IRangeBase *iface, TrustLevel *trust_level );
    HRESULT (WINAPI *get_Minimum)( IRangeBase *iface, DOUBLE *value );
    HRESULT (WINAPI *put_Minimum)( IRangeBase *iface, DOUBLE value );
    HRESULT (WINAPI *get_Maximum)( IRangeBase *iface, DOUBLE *value );
    HRESULT (WINAPI *put_Maximum)( IRangeBase *iface, DOUBLE value );
    HRESULT (WINAPI *get_SmallChange)( IRangeBase *iface, DOUBLE *value );
    HRESULT (WINAPI *put_SmallChange)( IRangeBase *iface, DOUBLE value );
    HRESULT (WINAPI *get_LargeChange)( IRangeBase *iface, DOUBLE *value );
    HRESULT (WINAPI *put_LargeChange)( IRangeBase *iface, DOUBLE value );
    HRESULT (WINAPI *get_Value)( IRangeBase *iface, DOUBLE *value );
    HRESULT (WINAPI *put_Value)( IRangeBase *iface, DOUBLE value );
    END_INTERFACE
} IRangeBaseVtbl;

struct IRangeBase
{
    const IRangeBaseVtbl *lpVtbl;
};

#define IRangeBase_Release( iface ) ((iface)->lpVtbl->Release( iface ))
#define IRangeBase_put_Maximum( iface, value ) ((iface)->lpVtbl->put_Maximum( iface, value ))
#define IRangeBase_put_Value( iface, value ) ((iface)->lpVtbl->put_Value( iface, value ))
typedef struct IProgressBar IProgressBar;
typedef struct IProgressBarVtbl
{
    BEGIN_INTERFACE
    HRESULT (WINAPI *QueryInterface)( IProgressBar *iface, REFIID iid, void **out );
    ULONG (WINAPI *AddRef)( IProgressBar *iface );
    ULONG (WINAPI *Release)( IProgressBar *iface );
    HRESULT (WINAPI *GetIids)( IProgressBar *iface, ULONG *iid_count, IID **iids );
    HRESULT (WINAPI *GetRuntimeClassName)( IProgressBar *iface, HSTRING *class_name );
    HRESULT (WINAPI *GetTrustLevel)( IProgressBar *iface, TrustLevel *trust_level );
    HRESULT (WINAPI *get_IsIndeterminate)( IProgressBar *iface, boolean *value );
    HRESULT (WINAPI *put_IsIndeterminate)( IProgressBar *iface, boolean value );
    HRESULT (WINAPI *get_ShowError)( IProgressBar *iface, boolean *value );
    HRESULT (WINAPI *put_ShowError)( IProgressBar *iface, boolean value );
    HRESULT (WINAPI *get_ShowPaused)( IProgressBar *iface, boolean *value );
    HRESULT (WINAPI *put_ShowPaused)( IProgressBar *iface, boolean value );
    HRESULT (WINAPI *get_TemplateSettings)( IProgressBar *iface, IInspectable **value );
    END_INTERFACE
} IProgressBarVtbl;

struct IProgressBar
{
    const IProgressBarVtbl *lpVtbl;
};

#define IProgressBar_Release( iface ) ((iface)->lpVtbl->Release( iface ))
#define IProgressBar_put_IsIndeterminate( iface, value ) ((iface)->lpVtbl->put_IsIndeterminate( iface, value ))



typedef struct IDesktopWindowXamlSource IDesktopWindowXamlSource;
typedef struct IDesktopWindowXamlSourceFactory IDesktopWindowXamlSourceFactory;

typedef struct IDesktopWindowXamlSourceVtbl
{
    BEGIN_INTERFACE
    HRESULT (WINAPI *QueryInterface)( IDesktopWindowXamlSource *iface, REFIID iid, void **out );
    ULONG (WINAPI *AddRef)( IDesktopWindowXamlSource *iface );
    ULONG (WINAPI *Release)( IDesktopWindowXamlSource *iface );
    HRESULT (WINAPI *GetIids)( IDesktopWindowXamlSource *iface, ULONG *iid_count, IID **iids );
    HRESULT (WINAPI *GetRuntimeClassName)( IDesktopWindowXamlSource *iface, HSTRING *class_name );
    HRESULT (WINAPI *GetTrustLevel)( IDesktopWindowXamlSource *iface, TrustLevel *trust_level );
    HRESULT (WINAPI *get_Content)( IDesktopWindowXamlSource *iface, IInspectable **value );
    HRESULT (WINAPI *put_Content)( IDesktopWindowXamlSource *iface, IInspectable *value );
    HRESULT (WINAPI *get_HasFocus)( IDesktopWindowXamlSource *iface, boolean *value );
    HRESULT (WINAPI *add_TakeFocusRequested)( IDesktopWindowXamlSource *iface, IUnknown *handler, EventRegistrationToken *token );
    HRESULT (WINAPI *remove_TakeFocusRequested)( IDesktopWindowXamlSource *iface, EventRegistrationToken token );
    HRESULT (WINAPI *add_GotFocus)( IDesktopWindowXamlSource *iface, IUnknown *handler, EventRegistrationToken *token );
    HRESULT (WINAPI *remove_GotFocus)( IDesktopWindowXamlSource *iface, EventRegistrationToken token );
    HRESULT (WINAPI *NavigateFocus)( IDesktopWindowXamlSource *iface, IInspectable *request, IInspectable **result );
    END_INTERFACE
} IDesktopWindowXamlSourceVtbl;

struct IDesktopWindowXamlSource
{
    const IDesktopWindowXamlSourceVtbl *lpVtbl;
};

typedef struct IDesktopWindowXamlSourceFactoryVtbl
{
    BEGIN_INTERFACE
    HRESULT (WINAPI *QueryInterface)( IDesktopWindowXamlSourceFactory *iface, REFIID iid, void **out );
    ULONG (WINAPI *AddRef)( IDesktopWindowXamlSourceFactory *iface );
    ULONG (WINAPI *Release)( IDesktopWindowXamlSourceFactory *iface );
    HRESULT (WINAPI *GetIids)( IDesktopWindowXamlSourceFactory *iface, ULONG *iid_count, IID **iids );
    HRESULT (WINAPI *GetRuntimeClassName)( IDesktopWindowXamlSourceFactory *iface, HSTRING *class_name );
    HRESULT (WINAPI *GetTrustLevel)( IDesktopWindowXamlSourceFactory *iface, TrustLevel *trust_level );
    HRESULT (WINAPI *CreateInstance)( IDesktopWindowXamlSourceFactory *iface, IInspectable *base_interface,
                                      IInspectable **inner_interface, IDesktopWindowXamlSource **value );
    END_INTERFACE
} IDesktopWindowXamlSourceFactoryVtbl;

struct IDesktopWindowXamlSourceFactory
{
    const IDesktopWindowXamlSourceFactoryVtbl *lpVtbl;
};

#define IDesktopWindowXamlSourceFactory_CreateInstance( iface, base_interface, inner_interface, value ) \
    ((iface)->lpVtbl->CreateInstance( iface, base_interface, inner_interface, value ))

#define check_interface( obj, iid, is_broken ) check_interface_( __LINE__, obj, iid, is_broken )
static void check_interface_( unsigned int line, void *obj, const IID *iid, BOOL is_broken )
{
    IUnknown *iface = obj;
    IUnknown *unk;
    HRESULT hr;

    hr = IUnknown_QueryInterface( iface, iid, (void **)&unk );
    ok_(__FILE__, line)( hr == S_OK || broken( is_broken && hr == E_NOINTERFACE ), "got hr %#lx.\n", hr );
    if (SUCCEEDED(hr))
        IUnknown_Release( unk );
}

static void test_ColorHelper(void)
{
    static const WCHAR *color_helper_statics_name = L"Windows.UI.ColorHelper";
    IColorHelperStatics *color_helper_statics = (void *)0xdeadbeef;
    IActivationFactory *factory = (void *)0xdeadbeef;
    Color color;
    HSTRING str;
    HRESULT hr;

    hr = WindowsCreateString( color_helper_statics_name, wcslen( color_helper_statics_name ), &str );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    hr = RoGetActivationFactory( str, &IID_IActivationFactory, (void **)&factory );
    WindowsDeleteString( str );
    ok( hr == S_OK || broken( hr == REGDB_E_CLASSNOTREG ), "got hr %#lx.\n", hr );
    if (hr == REGDB_E_CLASSNOTREG)
    {
        win_skip( "%s runtimeclass not registered, skipping tests.\n", wine_dbgstr_w( color_helper_statics_name ) );
        return;
    }

    check_interface( factory, &IID_IUnknown, FALSE );
    check_interface( factory, &IID_IInspectable, FALSE );
    check_interface( factory, &IID_IAgileObject, TRUE /* Missing on Windows older than 1809v2 */ );

    hr = IActivationFactory_QueryInterface( factory, &IID_IColorHelperStatics, (void **)&color_helper_statics );
    ok( hr == S_OK, "got hr %#lx.\n", hr );

    memset( &color, 0, sizeof( color ) );
    hr = IColorHelperStatics_FromArgb( color_helper_statics, 255, 255, 255, 255, NULL );
    ok( hr == E_POINTER, "got hr %#lx.\n", hr );
    hr = IColorHelperStatics_FromArgb( color_helper_statics, 255, 255, 255, 255, &color );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    ok( color.A == 255 && color.R == 255 && color.G == 255 && color.B == 255,
        "got color.A = %u, color.R = %u, color.G = %u, color.B = %u,\n", color.A, color.R, color.G, color.B );

    IColorHelperStatics_Release( color_helper_statics );
    IActivationFactory_Release( factory );
}

static void test_WindowsXamlManager(void)
{
    static const WCHAR *windows_xaml_manager_name = L"Windows.UI.Xaml.Hosting.WindowsXamlManager";
    IWindowsXamlManagerStatics *windows_xaml_manager_statics = (void *)0xdeadbeef;
    IWindowsXamlManager *windows_xaml_manager = (void *)0xdeadbeef;
    IActivationFactory *factory = (void *)0xdeadbeef;
    IClosable *closable = NULL;
    HSTRING str;
    HRESULT hr;

    hr = WindowsCreateString( windows_xaml_manager_name, wcslen( windows_xaml_manager_name ), &str );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    hr = RoGetActivationFactory( str, &IID_IActivationFactory, (void **)&factory );
    WindowsDeleteString( str );
    ok( hr == S_OK || broken( hr == REGDB_E_CLASSNOTREG ), "got hr %#lx.\n", hr );
    if (hr == REGDB_E_CLASSNOTREG)
    {
        win_skip( "%s runtimeclass not registered, skipping tests.\n", wine_dbgstr_w( windows_xaml_manager_name ) );
        return;
    }

    hr = IActivationFactory_QueryInterface( factory, &IID_IWindowsXamlManagerStatics,
                                            (void **)&windows_xaml_manager_statics );
    ok( hr == S_OK, "got hr %#lx.\n", hr );

    hr = IWindowsXamlManagerStatics_InitializeForCurrentThread( windows_xaml_manager_statics,
                                                                &windows_xaml_manager );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    ok( !!windows_xaml_manager, "got manager %p.\n", windows_xaml_manager );

    check_interface( windows_xaml_manager, &IID_IUnknown, FALSE );
    check_interface( windows_xaml_manager, &IID_IInspectable, FALSE );
    check_interface( windows_xaml_manager, &IID_IWindowsXamlManager, FALSE );
    check_interface( windows_xaml_manager, &IID_IClosable, FALSE );

    hr = IInspectable_QueryInterface( (IInspectable *)windows_xaml_manager, &IID_IClosable, (void **)&closable );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    hr = IClosable_Close( closable );
    ok( hr == S_OK, "got hr %#lx.\n", hr );

    IClosable_Release( closable );
    IInspectable_Release( (IInspectable *)windows_xaml_manager );
    IInspectable_Release( (IInspectable *)windows_xaml_manager_statics );
    IActivationFactory_Release( factory );
}

static void test_DesktopWindowXamlSource(void)
{
    static const WCHAR *desktop_window_xaml_source_name = L"Windows.UI.Xaml.Hosting.DesktopWindowXamlSource";
    IDesktopWindowXamlSourceFactory *desktop_window_xaml_source_factory = (void *)0xdeadbeef;
    IDesktopWindowXamlSource *desktop_window_xaml_source = (void *)0xdeadbeef;
    IDesktopWindowXamlSourceNative *native = NULL;
    IActivationFactory *factory = (void *)0xdeadbeef;
    IInspectable *inner = (void *)0xdeadbeef;
    HSTRING str;
    HRESULT hr;

    hr = WindowsCreateString( desktop_window_xaml_source_name, wcslen( desktop_window_xaml_source_name ), &str );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    hr = RoGetActivationFactory( str, &IID_IActivationFactory, (void **)&factory );
    WindowsDeleteString( str );
    ok( hr == S_OK || broken( hr == REGDB_E_CLASSNOTREG ), "got hr %#lx.\n", hr );
    if (hr == REGDB_E_CLASSNOTREG)
    {
        win_skip( "%s runtimeclass not registered, skipping tests.\n", wine_dbgstr_w( desktop_window_xaml_source_name ) );
        return;
    }

    hr = IActivationFactory_QueryInterface( factory, &IID_IDesktopWindowXamlSourceFactory,
                                            (void **)&desktop_window_xaml_source_factory );
    ok( hr == S_OK, "got hr %#lx.\n", hr );

    hr = IDesktopWindowXamlSourceFactory_CreateInstance( desktop_window_xaml_source_factory, NULL, &inner,
                                                         &desktop_window_xaml_source );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    ok( !inner, "got inner %p.\n", inner );
    ok( !!desktop_window_xaml_source, "got source %p.\n", desktop_window_xaml_source );

    check_interface( desktop_window_xaml_source, &IID_IUnknown, FALSE );
    check_interface( desktop_window_xaml_source, &IID_IInspectable, FALSE );
    check_interface( desktop_window_xaml_source, &IID_IDesktopWindowXamlSource, FALSE );
    check_interface( desktop_window_xaml_source, &IID_IClosable, FALSE );
    check_interface( desktop_window_xaml_source, &IID_IDesktopWindowXamlSourceNative, FALSE );
    check_interface( desktop_window_xaml_source, &IID_IDesktopWindowXamlSourceNative2, FALSE );

    hr = IInspectable_QueryInterface( (IInspectable *)desktop_window_xaml_source, &IID_IDesktopWindowXamlSourceNative, (void **)&native );
    ok( hr == S_OK, "got hr %#lx.\n", hr );

    IDesktopWindowXamlSourceNative_Release( native );
    IInspectable_Release( (IInspectable *)desktop_window_xaml_source );
    IInspectable_Release( (IInspectable *)desktop_window_xaml_source_factory );
    IActivationFactory_Release( factory );
}

static void test_XamlReader(void)
{
    static const WCHAR *xaml_reader_name = L"Windows.UI.Xaml.Markup.XamlReader";
    static const WCHAR *xaml = L"<Grid xmlns=\"http://schemas.microsoft.com/winfx/2006/xaml/presentation\"/>";
    IXamlReaderStatics *xaml_reader_statics = (void *)0xdeadbeef;
    IActivationFactory *factory = (void *)0xdeadbeef;
    IInspectable *value = NULL, *loaded = NULL, *named = NULL;
    ITextBlock *text_block = NULL;
    IRangeBase *range_base = NULL;
    IProgressBar *progress_bar = NULL;
    HSTRING str, xaml_str, text_str;
    HRESULT hr;

    hr = WindowsCreateString( xaml_reader_name, wcslen( xaml_reader_name ), &str );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    hr = RoGetActivationFactory( str, &IID_IActivationFactory, (void **)&factory );
    WindowsDeleteString( str );
    ok( hr == S_OK || broken( hr == REGDB_E_CLASSNOTREG ), "got hr %#lx.\n", hr );
    if (hr == REGDB_E_CLASSNOTREG)
    {
        win_skip( "%s runtimeclass not registered, skipping tests.\n", wine_dbgstr_w( xaml_reader_name ) );
        return;
    }

    hr = IActivationFactory_QueryInterface( factory, &IID_IXamlReaderStatics, (void **)&xaml_reader_statics );
    ok( hr == S_OK, "got hr %#lx.\n", hr );

    hr = WindowsCreateString( xaml, wcslen( xaml ), &xaml_str );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    hr = IXamlReaderStatics_Load( xaml_reader_statics, xaml_str, &value );
    WindowsDeleteString( xaml_str );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    ok( !!value, "got value %p.\n", value );

    hr = IInspectable_QueryInterface( value, &IID_IFrameworkElement, (void **)&loaded );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    ok( !!loaded, "got loaded %p.\n", loaded );

    hr = WindowsCreateString( L"Overlay", 7, &xaml_str );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    hr = IFrameworkElement_FindName( (IFrameworkElement *)loaded, xaml_str, &named );
    WindowsDeleteString( xaml_str );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    ok( !!named, "got named %p.\n", named );
    check_interface( named, &IID_ISwapChainPanel, FALSE );
    check_interface( named, &IID_ISwapChainPanelNative, FALSE );
    IInspectable_Release( named );
    named = NULL;

    hr = WindowsCreateString( L"static1", 7, &xaml_str );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    hr = IFrameworkElement_FindName( (IFrameworkElement *)loaded, xaml_str, &named );
    WindowsDeleteString( xaml_str );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    ok( !!named, "got named %p.\n", named );
    check_interface( named, &IID_ITextBlock, FALSE );
    hr = IInspectable_QueryInterface( named, &IID_ITextBlock, (void **)&text_block );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    hr = WindowsCreateString( L"hello", 5, &text_str );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    hr = ITextBlock_put_Text( text_block, text_str );
    WindowsDeleteString( text_str );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    ITextBlock_Release( text_block );
    text_block = NULL;
    IInspectable_Release( named );
    named = NULL;

    hr = WindowsCreateString( L"static2", 7, &xaml_str );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    hr = IFrameworkElement_FindName( (IFrameworkElement *)loaded, xaml_str, &named );
    WindowsDeleteString( xaml_str );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    ok( !!named, "got named %p.\n", named );
    check_interface( named, &IID_ITextBlock, FALSE );
    IInspectable_Release( named );
    named = NULL;

    hr = WindowsCreateString( L"progressBar", 11, &xaml_str );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    hr = IFrameworkElement_FindName( (IFrameworkElement *)loaded, xaml_str, &named );
    WindowsDeleteString( xaml_str );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    ok( !!named, "got named %p.\n", named );
    check_interface( named, &IID_IProgressBar, FALSE );
    check_interface( named, &IID_IRangeBase, FALSE );
    check_interface( loaded, &IID_IUIElement, FALSE );
    hr = IInspectable_QueryInterface( named, &IID_IProgressBar, (void **)&progress_bar );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    hr = IProgressBar_put_IsIndeterminate( progress_bar, 1 );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    IProgressBar_Release( progress_bar );
    progress_bar = NULL;
    hr = IInspectable_QueryInterface( named, &IID_IRangeBase, (void **)&range_base );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    hr = IRangeBase_put_Maximum( range_base, 100.0 );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    hr = IRangeBase_put_Value( range_base, 50.0 );
    ok( hr == S_OK, "got hr %#lx.\n", hr );
    IRangeBase_Release( range_base );
    range_base = NULL;

    IInspectable_Release( named );
    IInspectable_Release( loaded );
    IInspectable_Release( value );
    IInspectable_Release( (IInspectable *)xaml_reader_statics );
    IActivationFactory_Release( factory );
}


START_TEST(xaml)
{
    HRESULT hr;

    hr = RoInitialize( RO_INIT_MULTITHREADED );
    ok( hr == S_OK, "RoInitialize failed, hr %#lx\n", hr );

    test_ColorHelper();
    test_WindowsXamlManager();
    test_DesktopWindowXamlSource();
    test_XamlReader();

    RoUninitialize();
}
