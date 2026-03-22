/* WinRT Windows.UI.Xaml.Hosting.DesktopWindowXamlSource implementation */

#include "private.h"

WINE_DEFAULT_DEBUG_CHANNEL( xaml );

static const IID IID_IDesktopWindowXamlSource =
    {0xd585bfe1, 0x00ff, 0x51be, {0xba, 0x1d, 0xa1, 0x32, 0x99, 0x56, 0xea, 0x0a}};
static const IID IID_IDesktopWindowXamlSourceFactory =
    {0x5cd61dc0, 0x2561, 0x56e1, {0x8e, 0x75, 0x6e, 0x44, 0x17, 0x38, 0x05, 0xe3}};

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

struct desktop_window_xaml_source
{
    IDesktopWindowXamlSource IDesktopWindowXamlSource_iface;
    IClosable IClosable_iface;
    IDesktopWindowXamlSourceNative IDesktopWindowXamlSourceNative_iface;
    IDesktopWindowXamlSourceNative2 IDesktopWindowXamlSourceNative2_iface;
    LONG ref;
    IInspectable *content;
    HWND window;
};

struct desktop_window_xaml_source_statics
{
    IActivationFactory IActivationFactory_iface;
    IDesktopWindowXamlSourceFactory IDesktopWindowXamlSourceFactory_iface;
    LONG ref;
};

static const WCHAR desktop_window_xaml_source_name[] = L"Windows.UI.Xaml.Hosting.DesktopWindowXamlSource";
static EventRegistrationToken dummy_token = {0xdeadbeef};

static inline struct desktop_window_xaml_source *impl_from_IDesktopWindowXamlSource( IDesktopWindowXamlSource *iface )
{
    return CONTAINING_RECORD( iface, struct desktop_window_xaml_source, IDesktopWindowXamlSource_iface );
}

static inline struct desktop_window_xaml_source_statics *impl_from_xaml_source_IActivationFactory( IActivationFactory *iface )
{
    return CONTAINING_RECORD( iface, struct desktop_window_xaml_source_statics, IActivationFactory_iface );
}

static HRESULT WINAPI desktop_window_xaml_source_QueryInterface( IDesktopWindowXamlSource *iface, REFIID iid, void **out )
{
    struct desktop_window_xaml_source *impl = impl_from_IDesktopWindowXamlSource( iface );

    TRACE( "iface %p, iid %s, out %p.\n", iface, debugstr_guid( iid ), out );

    if (!out) return E_POINTER;

    if (IsEqualGUID( iid, &IID_IUnknown ) ||
        IsEqualGUID( iid, &IID_IInspectable ) ||
        IsEqualGUID( iid, &IID_IAgileObject ) ||
        IsEqualGUID( iid, &IID_IDesktopWindowXamlSource ))
    {
        *out = &impl->IDesktopWindowXamlSource_iface;
        IInspectable_AddRef( (IInspectable *)*out );
        return S_OK;
    }
    if (IsEqualGUID( iid, &IID_IClosable ))
    {
        *out = &impl->IClosable_iface;
        IInspectable_AddRef( (IInspectable *)*out );
        return S_OK;
    }
    if (IsEqualGUID( iid, &IID_IDesktopWindowXamlSourceNative ))
    {
        *out = &impl->IDesktopWindowXamlSourceNative_iface;
        IInspectable_AddRef( (IInspectable *)&impl->IDesktopWindowXamlSource_iface );
        return S_OK;
    }
    if (IsEqualGUID( iid, &IID_IDesktopWindowXamlSourceNative2 ))
    {
        *out = &impl->IDesktopWindowXamlSourceNative2_iface;
        IInspectable_AddRef( (IInspectable *)&impl->IDesktopWindowXamlSource_iface );
        return S_OK;
    }

    FIXME( "%s not implemented, returning E_NOINTERFACE.\n", debugstr_guid( iid ) );
    *out = NULL;
    return E_NOINTERFACE;
}

static ULONG WINAPI desktop_window_xaml_source_AddRef( IDesktopWindowXamlSource *iface )
{
    struct desktop_window_xaml_source *impl = impl_from_IDesktopWindowXamlSource( iface );
    ULONG ref = InterlockedIncrement( &impl->ref );
    TRACE( "iface %p increasing refcount to %lu.\n", iface, ref );
    return ref;
}

static ULONG WINAPI desktop_window_xaml_source_Release( IDesktopWindowXamlSource *iface )
{
    struct desktop_window_xaml_source *impl = impl_from_IDesktopWindowXamlSource( iface );
    ULONG ref = InterlockedDecrement( &impl->ref );
    TRACE( "iface %p decreasing refcount to %lu.\n", iface, ref );

    if (!ref)
    {
        if (impl->content) IInspectable_Release( impl->content );
        if (impl->window) DestroyWindow( impl->window );
        free( impl );
    }
    return ref;
}

static HRESULT WINAPI desktop_window_xaml_source_GetIids( IDesktopWindowXamlSource *iface, ULONG *iid_count, IID **iids )
{
    FIXME( "iface %p, iid_count %p, iids %p stub!\n", iface, iid_count, iids );
    return E_NOTIMPL;
}

static HRESULT WINAPI desktop_window_xaml_source_GetRuntimeClassName( IDesktopWindowXamlSource *iface, HSTRING *class_name )
{
    if (!class_name) return E_POINTER;
    return WindowsCreateString( desktop_window_xaml_source_name, ARRAYSIZE( desktop_window_xaml_source_name ) - 1, class_name );
}

static HRESULT WINAPI desktop_window_xaml_source_GetTrustLevel( IDesktopWindowXamlSource *iface, TrustLevel *trust_level )
{
    if (!trust_level) return E_POINTER;
    *trust_level = BaseTrust;
    return S_OK;
}

static HRESULT WINAPI desktop_window_xaml_source_get_Content( IDesktopWindowXamlSource *iface, IInspectable **value )
{
    struct desktop_window_xaml_source *impl = impl_from_IDesktopWindowXamlSource( iface );
    if (!value) return E_POINTER;
    *value = impl->content;
    if (*value) IInspectable_AddRef( *value );
    return S_OK;
}

static HRESULT WINAPI desktop_window_xaml_source_put_Content( IDesktopWindowXamlSource *iface, IInspectable *value )
{
    struct desktop_window_xaml_source *impl = impl_from_IDesktopWindowXamlSource( iface );
    if (value) IInspectable_AddRef( value );
    if (impl->content) IInspectable_Release( impl->content );
    impl->content = value;
    return S_OK;
}

static HRESULT WINAPI desktop_window_xaml_source_get_HasFocus( IDesktopWindowXamlSource *iface, boolean *value )
{
    struct desktop_window_xaml_source *impl = impl_from_IDesktopWindowXamlSource( iface );
    if (!value) return E_POINTER;
    *value = impl->window && GetFocus() == impl->window;
    return S_OK;
}

static HRESULT WINAPI desktop_window_xaml_source_add_TakeFocusRequested( IDesktopWindowXamlSource *iface, IUnknown *handler, EventRegistrationToken *token )
{
    if (!token) return E_POINTER;
    TRACE( "iface %p, handler %p, token %p.\n", iface, handler, token );
    *token = dummy_token;
    return S_OK;
}

static HRESULT WINAPI desktop_window_xaml_source_remove_TakeFocusRequested( IDesktopWindowXamlSource *iface, EventRegistrationToken token )
{
    TRACE( "iface %p, token %#I64x.\n", iface, token.value );
    return S_OK;
}

static HRESULT WINAPI desktop_window_xaml_source_add_GotFocus( IDesktopWindowXamlSource *iface, IUnknown *handler, EventRegistrationToken *token )
{
    if (!token) return E_POINTER;
    TRACE( "iface %p, handler %p, token %p.\n", iface, handler, token );
    *token = dummy_token;
    return S_OK;
}

static HRESULT WINAPI desktop_window_xaml_source_remove_GotFocus( IDesktopWindowXamlSource *iface, EventRegistrationToken token )
{
    TRACE( "iface %p, token %#I64x.\n", iface, token.value );
    return S_OK;
}

static HRESULT WINAPI desktop_window_xaml_source_NavigateFocus( IDesktopWindowXamlSource *iface, IInspectable *request, IInspectable **result )
{
    TRACE( "iface %p, request %p, result %p.\n", iface, request, result );
    if (result) *result = NULL;
    return E_NOTIMPL;
}

static const IDesktopWindowXamlSourceVtbl desktop_window_xaml_source_vtbl =
{
    desktop_window_xaml_source_QueryInterface,
    desktop_window_xaml_source_AddRef,
    desktop_window_xaml_source_Release,
    desktop_window_xaml_source_GetIids,
    desktop_window_xaml_source_GetRuntimeClassName,
    desktop_window_xaml_source_GetTrustLevel,
    desktop_window_xaml_source_get_Content,
    desktop_window_xaml_source_put_Content,
    desktop_window_xaml_source_get_HasFocus,
    desktop_window_xaml_source_add_TakeFocusRequested,
    desktop_window_xaml_source_remove_TakeFocusRequested,
    desktop_window_xaml_source_add_GotFocus,
    desktop_window_xaml_source_remove_GotFocus,
    desktop_window_xaml_source_NavigateFocus,
};

DEFINE_IINSPECTABLE( desktop_window_xaml_source_closable, IClosable, struct desktop_window_xaml_source, IDesktopWindowXamlSource_iface )

static HRESULT WINAPI desktop_window_xaml_source_closable_Close( IClosable *iface )
{
    struct desktop_window_xaml_source *impl = impl_from_IClosable( iface );
    TRACE( "iface %p.\n", iface );
    if (impl->window)
    {
        DestroyWindow( impl->window );
        impl->window = NULL;
    }
    return S_OK;
}

static const struct IClosableVtbl desktop_window_xaml_source_closable_vtbl =
{
    desktop_window_xaml_source_closable_QueryInterface,
    desktop_window_xaml_source_closable_AddRef,
    desktop_window_xaml_source_closable_Release,
    desktop_window_xaml_source_closable_GetIids,
    desktop_window_xaml_source_closable_GetRuntimeClassName,
    desktop_window_xaml_source_closable_GetTrustLevel,
    desktop_window_xaml_source_closable_Close,
};

DEFINE_IINSPECTABLE_( desktop_window_xaml_source_native, IDesktopWindowXamlSourceNative, struct desktop_window_xaml_source,
                      impl_from_IDesktopWindowXamlSourceNative, IDesktopWindowXamlSourceNative_iface,
                      &impl->IDesktopWindowXamlSource_iface )

static HRESULT WINAPI desktop_window_xaml_source_native_AttachToWindow( IDesktopWindowXamlSourceNative *iface, HWND parent_wnd )
{
    struct desktop_window_xaml_source *impl = impl_from_IDesktopWindowXamlSourceNative( iface );

    {
        FILE *f = fopen("/tmp/xaml_debug.log", "a");
        if (f) {
            WCHAR class_name[256] = {0}, window_name[256] = {0};
            GetClassNameW(parent_wnd, class_name, 256);
            GetWindowTextW(parent_wnd, window_name, 256);
            fprintf(f, "AttachToWindow parent=%p class='%ls' title='%ls'\n", parent_wnd, class_name, window_name);
            fclose(f);
        }
    }

    if (!parent_wnd) return E_INVALIDARG;
    if (impl->window) return S_OK;

    {
        RECT parent_rc;
        GetClientRect( parent_wnd, &parent_rc );
        impl->window = CreateWindowExW( 0, L"WineXamlStubClass", NULL, WS_CHILD | WS_VISIBLE,
                                        0, 0, parent_rc.right, parent_rc.bottom, parent_wnd, 0,
                                        NULL, NULL );
    }

    if (!impl->window) return HRESULT_FROM_WIN32( GetLastError() );

    return S_OK;
}

static HRESULT WINAPI desktop_window_xaml_source_native_get_WindowHandle( IDesktopWindowXamlSourceNative *iface, HWND *wnd )
{
    struct desktop_window_xaml_source *impl = impl_from_IDesktopWindowXamlSourceNative( iface );
    if (!wnd) return E_POINTER;
    *wnd = impl->window;
    return S_OK;
}

static const struct IDesktopWindowXamlSourceNativeVtbl desktop_window_xaml_source_native_vtbl =
{
    desktop_window_xaml_source_native_QueryInterface,
    desktop_window_xaml_source_native_AddRef,
    desktop_window_xaml_source_native_Release,
    desktop_window_xaml_source_native_AttachToWindow,
    desktop_window_xaml_source_native_get_WindowHandle,
};

DEFINE_IINSPECTABLE_( desktop_window_xaml_source_native2, IDesktopWindowXamlSourceNative2, struct desktop_window_xaml_source,
                      impl_from_IDesktopWindowXamlSourceNative2, IDesktopWindowXamlSourceNative2_iface,
                      &impl->IDesktopWindowXamlSource_iface )

static HRESULT WINAPI desktop_window_xaml_source_native2_AttachToWindow( IDesktopWindowXamlSourceNative2 *iface, HWND parent_wnd )
{
    return IDesktopWindowXamlSourceNative_AttachToWindow( &impl_from_IDesktopWindowXamlSourceNative2( iface )->IDesktopWindowXamlSourceNative_iface, parent_wnd );
}

static HRESULT WINAPI desktop_window_xaml_source_native2_get_WindowHandle( IDesktopWindowXamlSourceNative2 *iface, HWND *wnd )
{
    return IDesktopWindowXamlSourceNative_get_WindowHandle( &impl_from_IDesktopWindowXamlSourceNative2( iface )->IDesktopWindowXamlSourceNative_iface, wnd );
}

static HRESULT WINAPI desktop_window_xaml_source_native2_PreTranslateMessage( IDesktopWindowXamlSourceNative2 *iface,
                                                                               const MSG *message, BOOL *result )
{
    TRACE( "iface %p, message %p, result %p.\n", iface, message, result );
    if (!result) return E_POINTER;
    *result = FALSE;
    return S_OK;
}

static const struct IDesktopWindowXamlSourceNative2Vtbl desktop_window_xaml_source_native2_vtbl =
{
    desktop_window_xaml_source_native2_QueryInterface,
    desktop_window_xaml_source_native2_AddRef,
    desktop_window_xaml_source_native2_Release,
    desktop_window_xaml_source_native2_AttachToWindow,
    desktop_window_xaml_source_native2_get_WindowHandle,
    desktop_window_xaml_source_native2_PreTranslateMessage,
};

static HRESULT desktop_window_xaml_source_create( IDesktopWindowXamlSource **value )
{
    struct desktop_window_xaml_source *impl;

    if (!value) return E_POINTER;
    if (!(impl = calloc( 1, sizeof(*impl) ))) return E_OUTOFMEMORY;

    impl->IDesktopWindowXamlSource_iface.lpVtbl = &desktop_window_xaml_source_vtbl;
    impl->IClosable_iface.lpVtbl = &desktop_window_xaml_source_closable_vtbl;
    impl->IDesktopWindowXamlSourceNative_iface.lpVtbl = &desktop_window_xaml_source_native_vtbl;
    impl->IDesktopWindowXamlSourceNative2_iface.lpVtbl = &desktop_window_xaml_source_native2_vtbl;
    impl->ref = 1;

    *value = &impl->IDesktopWindowXamlSource_iface;
    return S_OK;
}

static HRESULT WINAPI desktop_factory_QueryInterface( IActivationFactory *iface, REFIID iid, void **out )
{
    struct desktop_window_xaml_source_statics *impl = impl_from_xaml_source_IActivationFactory( iface );

    TRACE( "iface %p, iid %s, out %p.\n", iface, debugstr_guid( iid ), out );

    if (!out) return E_POINTER;

    if (IsEqualGUID( iid, &IID_IUnknown ) ||
        IsEqualGUID( iid, &IID_IInspectable ) ||
        IsEqualGUID( iid, &IID_IAgileObject ) ||
        IsEqualGUID( iid, &IID_IActivationFactory ))
    {
        *out = &impl->IActivationFactory_iface;
        IInspectable_AddRef( (IInspectable *)*out );
        return S_OK;
    }
    if (IsEqualGUID( iid, &IID_IDesktopWindowXamlSourceFactory ))
    {
        *out = &impl->IDesktopWindowXamlSourceFactory_iface;
        IInspectable_AddRef( (IInspectable *)*out );
        return S_OK;
    }

    FIXME( "%s not implemented, returning E_NOINTERFACE.\n", debugstr_guid( iid ) );
    *out = NULL;
    return E_NOINTERFACE;
}

static ULONG WINAPI desktop_factory_AddRef( IActivationFactory *iface )
{
    struct desktop_window_xaml_source_statics *impl = impl_from_xaml_source_IActivationFactory( iface );
    ULONG ref = InterlockedIncrement( &impl->ref );
    TRACE( "iface %p increasing refcount to %lu.\n", iface, ref );
    return ref;
}

static ULONG WINAPI desktop_factory_Release( IActivationFactory *iface )
{
    struct desktop_window_xaml_source_statics *impl = impl_from_xaml_source_IActivationFactory( iface );
    ULONG ref = InterlockedDecrement( &impl->ref );
    TRACE( "iface %p decreasing refcount to %lu.\n", iface, ref );
    return ref;
}

static HRESULT WINAPI desktop_factory_GetIids( IActivationFactory *iface, ULONG *iid_count, IID **iids )
{
    FIXME( "iface %p, iid_count %p, iids %p stub!\n", iface, iid_count, iids );
    return E_NOTIMPL;
}

static HRESULT WINAPI desktop_factory_GetRuntimeClassName( IActivationFactory *iface, HSTRING *class_name )
{
    if (!class_name) return E_POINTER;
    return WindowsCreateString( desktop_window_xaml_source_name, ARRAYSIZE( desktop_window_xaml_source_name ) - 1, class_name );
}

static HRESULT WINAPI desktop_factory_GetTrustLevel( IActivationFactory *iface, TrustLevel *trust_level )
{
    if (!trust_level) return E_POINTER;
    *trust_level = BaseTrust;
    return S_OK;
}

static HRESULT WINAPI desktop_factory_ActivateInstance( IActivationFactory *iface, IInspectable **instance )
{
    return desktop_window_xaml_source_create( (IDesktopWindowXamlSource **)instance );
}

static const struct IActivationFactoryVtbl desktop_factory_vtbl =
{
    desktop_factory_QueryInterface,
    desktop_factory_AddRef,
    desktop_factory_Release,
    desktop_factory_GetIids,
    desktop_factory_GetRuntimeClassName,
    desktop_factory_GetTrustLevel,
    desktop_factory_ActivateInstance,
};

DEFINE_IINSPECTABLE_( desktop_window_xaml_source_factory, IDesktopWindowXamlSourceFactory,
                      struct desktop_window_xaml_source_statics, impl_from_IDesktopWindowXamlSourceFactory,
                      IDesktopWindowXamlSourceFactory_iface, &impl->IActivationFactory_iface )

static HRESULT WINAPI desktop_window_xaml_source_factory_CreateInstance( IDesktopWindowXamlSourceFactory *iface,
                                                                         IInspectable *base_interface,
                                                                         IInspectable **inner_interface,
                                                                         IDesktopWindowXamlSource **value )
{
    TRACE( "iface %p, base_interface %p, inner_interface %p, value %p.\n", iface, base_interface, inner_interface, value );
    if (inner_interface) *inner_interface = NULL;
    return desktop_window_xaml_source_create( value );
}

static const IDesktopWindowXamlSourceFactoryVtbl desktop_window_xaml_source_factory_vtbl =
{
    desktop_window_xaml_source_factory_QueryInterface,
    desktop_window_xaml_source_factory_AddRef,
    desktop_window_xaml_source_factory_Release,
    desktop_window_xaml_source_factory_GetIids,
    desktop_window_xaml_source_factory_GetRuntimeClassName,
    desktop_window_xaml_source_factory_GetTrustLevel,
    desktop_window_xaml_source_factory_CreateInstance,
};

static struct desktop_window_xaml_source_statics desktop_window_xaml_source_statics =
{
    {&desktop_factory_vtbl},
    {&desktop_window_xaml_source_factory_vtbl},
    1,
};

IActivationFactory *desktop_window_xaml_source_factory = &desktop_window_xaml_source_statics.IActivationFactory_iface;
