/* WinRT Windows.UI.Xaml.Hosting.WindowsXamlManager implementation */

#include "private.h"

WINE_DEFAULT_DEBUG_CHANNEL( xaml );

static const IID IID_IWindowsXamlManager =
    {0x56096c31, 0x1aa0, 0x5288, {0x88, 0x18, 0x6e, 0x74, 0xa2, 0xdc, 0xaf, 0xf5}};
static const IID IID_IWindowsXamlManagerStatics =
    {0x28258a12, 0x7d82, 0x505b, {0xb2, 0x10, 0x71, 0x2b, 0x04, 0xa5, 0x88, 0x82}};

typedef IInspectable IWindowsXamlManager;
typedef IInspectableVtbl IWindowsXamlManagerVtbl;

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

struct windows_xaml_manager
{
    IWindowsXamlManager IWindowsXamlManager_iface;
    IClosable IClosable_iface;
    LONG ref;
};

struct windows_xaml_manager_statics
{
    IActivationFactory IActivationFactory_iface;
    IWindowsXamlManagerStatics IWindowsXamlManagerStatics_iface;
    LONG ref;
};

static const WCHAR windows_xaml_manager_name[] = L"Windows.UI.Xaml.Hosting.WindowsXamlManager";

static inline struct windows_xaml_manager *impl_from_IWindowsXamlManager( IWindowsXamlManager *iface )
{
    return CONTAINING_RECORD( iface, struct windows_xaml_manager, IWindowsXamlManager_iface );
}

static inline struct windows_xaml_manager_statics *impl_from_IActivationFactory( IActivationFactory *iface )
{
    return CONTAINING_RECORD( iface, struct windows_xaml_manager_statics, IActivationFactory_iface );
}

static HRESULT WINAPI windows_xaml_manager_QueryInterface( IWindowsXamlManager *iface, REFIID iid, void **out )
{
    struct windows_xaml_manager *impl = impl_from_IWindowsXamlManager( iface );

    TRACE( "iface %p, iid %s, out %p.\n", iface, debugstr_guid( iid ), out );

    if (!out) return E_POINTER;

    if (IsEqualGUID( iid, &IID_IUnknown ) ||
        IsEqualGUID( iid, &IID_IInspectable ) ||
        IsEqualGUID( iid, &IID_IAgileObject ) ||
        IsEqualGUID( iid, &IID_IWindowsXamlManager ))
    {
        *out = &impl->IWindowsXamlManager_iface;
        IInspectable_AddRef( (IInspectable *)*out );
        return S_OK;
    }

    if (IsEqualGUID( iid, &IID_IClosable ))
    {
        *out = &impl->IClosable_iface;
        IInspectable_AddRef( (IInspectable *)*out );
        return S_OK;
    }

    FIXME( "%s not implemented, returning E_NOINTERFACE.\n", debugstr_guid( iid ) );
    *out = NULL;
    return E_NOINTERFACE;
}

static ULONG WINAPI windows_xaml_manager_AddRef( IWindowsXamlManager *iface )
{
    struct windows_xaml_manager *impl = impl_from_IWindowsXamlManager( iface );
    ULONG ref = InterlockedIncrement( &impl->ref );

    TRACE( "iface %p increasing refcount to %lu.\n", iface, ref );
    return ref;
}

static ULONG WINAPI windows_xaml_manager_Release( IWindowsXamlManager *iface )
{
    struct windows_xaml_manager *impl = impl_from_IWindowsXamlManager( iface );
    ULONG ref = InterlockedDecrement( &impl->ref );

    TRACE( "iface %p decreasing refcount to %lu.\n", iface, ref );

    if (!ref)
        free( impl );

    return ref;
}

static HRESULT WINAPI windows_xaml_manager_GetIids( IWindowsXamlManager *iface, ULONG *iid_count, IID **iids )
{
    FIXME( "iface %p, iid_count %p, iids %p stub!\n", iface, iid_count, iids );
    return E_NOTIMPL;
}

static HRESULT WINAPI windows_xaml_manager_GetRuntimeClassName( IWindowsXamlManager *iface, HSTRING *class_name )
{
    TRACE( "iface %p, class_name %p.\n", iface, class_name );

    if (!class_name) return E_POINTER;

    return WindowsCreateString( windows_xaml_manager_name, ARRAYSIZE( windows_xaml_manager_name ) - 1, class_name );
}

static HRESULT WINAPI windows_xaml_manager_GetTrustLevel( IWindowsXamlManager *iface, TrustLevel *trust_level )
{
    TRACE( "iface %p, trust_level %p.\n", iface, trust_level );

    if (!trust_level) return E_POINTER;
    *trust_level = BaseTrust;
    return S_OK;
}

static const IWindowsXamlManagerVtbl windows_xaml_manager_vtbl =
{
    windows_xaml_manager_QueryInterface,
    windows_xaml_manager_AddRef,
    windows_xaml_manager_Release,
    windows_xaml_manager_GetIids,
    windows_xaml_manager_GetRuntimeClassName,
    windows_xaml_manager_GetTrustLevel,
};

DEFINE_IINSPECTABLE( closable, IClosable, struct windows_xaml_manager, IWindowsXamlManager_iface )

static HRESULT WINAPI closable_Close( IClosable *iface )
{
    TRACE( "iface %p.\n", iface );
    return S_OK;
}

static const struct IClosableVtbl closable_vtbl =
{
    closable_QueryInterface,
    closable_AddRef,
    closable_Release,
    closable_GetIids,
    closable_GetRuntimeClassName,
    closable_GetTrustLevel,
    closable_Close,
};

static HRESULT windows_xaml_manager_create( IWindowsXamlManager **manager )
{
    struct windows_xaml_manager *impl;

    if (!manager) return E_POINTER;

    if (!(impl = calloc( 1, sizeof(*impl) )))
        return E_OUTOFMEMORY;

    impl->IWindowsXamlManager_iface.lpVtbl = &windows_xaml_manager_vtbl;
    impl->IClosable_iface.lpVtbl = &closable_vtbl;
    impl->ref = 1;

    *manager = &impl->IWindowsXamlManager_iface;
    return S_OK;
}

static HRESULT WINAPI factory_QueryInterface( IActivationFactory *iface, REFIID iid, void **out )
{
    struct windows_xaml_manager_statics *impl = impl_from_IActivationFactory( iface );

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

    if (IsEqualGUID( iid, &IID_IWindowsXamlManagerStatics ))
    {
        *out = &impl->IWindowsXamlManagerStatics_iface;
        IInspectable_AddRef( (IInspectable *)*out );
        return S_OK;
    }

    FIXME( "%s not implemented, returning E_NOINTERFACE.\n", debugstr_guid( iid ) );
    *out = NULL;
    return E_NOINTERFACE;
}

static ULONG WINAPI factory_AddRef( IActivationFactory *iface )
{
    struct windows_xaml_manager_statics *impl = impl_from_IActivationFactory( iface );
    ULONG ref = InterlockedIncrement( &impl->ref );

    TRACE( "iface %p increasing refcount to %lu.\n", iface, ref );
    return ref;
}

static ULONG WINAPI factory_Release( IActivationFactory *iface )
{
    struct windows_xaml_manager_statics *impl = impl_from_IActivationFactory( iface );
    ULONG ref = InterlockedDecrement( &impl->ref );

    TRACE( "iface %p decreasing refcount to %lu.\n", iface, ref );
    return ref;
}

static HRESULT WINAPI factory_GetIids( IActivationFactory *iface, ULONG *iid_count, IID **iids )
{
    FIXME( "iface %p, iid_count %p, iids %p stub!\n", iface, iid_count, iids );
    return E_NOTIMPL;
}

static HRESULT WINAPI factory_GetRuntimeClassName( IActivationFactory *iface, HSTRING *class_name )
{
    TRACE( "iface %p, class_name %p.\n", iface, class_name );

    if (!class_name) return E_POINTER;

    return WindowsCreateString( windows_xaml_manager_name, ARRAYSIZE( windows_xaml_manager_name ) - 1, class_name );
}

static HRESULT WINAPI factory_GetTrustLevel( IActivationFactory *iface, TrustLevel *trust_level )
{
    TRACE( "iface %p, trust_level %p.\n", iface, trust_level );

    if (!trust_level) return E_POINTER;
    *trust_level = BaseTrust;
    return S_OK;
}

static HRESULT WINAPI factory_ActivateInstance( IActivationFactory *iface, IInspectable **instance )
{
    TRACE( "iface %p, instance %p.\n", iface, instance );
    return E_NOTIMPL;
}

static const struct IActivationFactoryVtbl factory_vtbl =
{
    factory_QueryInterface,
    factory_AddRef,
    factory_Release,
    factory_GetIids,
    factory_GetRuntimeClassName,
    factory_GetTrustLevel,
    factory_ActivateInstance,
};

DEFINE_IINSPECTABLE_( statics, IWindowsXamlManagerStatics, struct windows_xaml_manager_statics,
                      impl_from_IWindowsXamlManagerStatics, IWindowsXamlManagerStatics_iface,
                      &impl->IActivationFactory_iface )

static HRESULT WINAPI statics_InitializeForCurrentThread( IWindowsXamlManagerStatics *iface,
                                                          IWindowsXamlManager **manager )
{
    TRACE( "iface %p, manager %p.\n", iface, manager );
    return windows_xaml_manager_create( manager );
}

static const IWindowsXamlManagerStaticsVtbl statics_vtbl =
{
    statics_QueryInterface,
    statics_AddRef,
    statics_Release,
    statics_GetIids,
    statics_GetRuntimeClassName,
    statics_GetTrustLevel,
    statics_InitializeForCurrentThread,
};

static struct windows_xaml_manager_statics windows_xaml_manager_statics =
{
    {&factory_vtbl},
    {&statics_vtbl},
    1,
};

IActivationFactory *windows_xaml_manager_factory = &windows_xaml_manager_statics.IActivationFactory_iface;
