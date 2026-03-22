#include <stdio.h>
#include <wchar.h>
#include "private.h"

WINE_DEFAULT_DEBUG_CHANNEL( xaml );

extern WCHAR wine_xaml_stub_text[4096];
extern HWND wine_xaml_stub_hwnd;
struct wine_xaml_node *wine_xaml_root_node = NULL;

static void free_wine_xaml_node(struct wine_xaml_node *node)
{
    int i;
    if (!node) return;
    for (i = 0; i < node->child_count; i++)
        free_wine_xaml_node(node->children[i]);
    free(node->children);
    free(node);
}

static unsigned char hex_to_byte(const WCHAR *hex)
{
    unsigned char val = 0;
    int i;
    for (i = 0; i < 2; i++) {
        val <<= 4;
        if (hex[i] >= '0' && hex[i] <= '9') val += hex[i] - '0';
        else if (hex[i] >= 'a' && hex[i] <= 'f') val += hex[i] - 'a' + 10;
        else if (hex[i] >= 'A' && hex[i] <= 'F') val += hex[i] - 'A' + 10;
    }
    return val;
}

static DWORD parse_color(const WCHAR *hex)
{
    DWORD res = 0;
    const WCHAR *end;
    size_t len;
    if (hex[0] == '#') hex++;
    
    end = wcschr(hex, '"');
    len = end ? (size_t)(end - hex) : wcslen(hex);
    if (len >= 8) { hex += 2; len -= 2; } /* skip alpha if present */
    
    if (len >= 6) {
        unsigned char r = hex_to_byte(hex);
        unsigned char g = hex_to_byte(hex + 2);
        unsigned char b = hex_to_byte(hex + 4);
        res = RGB(r, g, b);
    } else if (wcsnicmp(hex, L"White", 5) == 0) res = RGB(255, 255, 255);
    
    {
        FILE *f = fopen("/tmp/xaml_debug.log", "a");
        if (f) { fprintf(f, "Parsed color '%.*ls' (len %d) -> 0x%08lx\n", (int)len, hex, (int)len, (unsigned long)res); fclose(f); }
    }
    return res;
}

static struct wine_xaml_node *find_node_by_name(struct wine_xaml_node *node, const WCHAR *name)
{
    int i;
    if (!node) return NULL;
    if (wcscmp(node->name, name) == 0) return node;
    for (i = 0; i < node->child_count; i++) {
        struct wine_xaml_node *found = find_node_by_name(node->children[i], name);
        if (found) return found;
    }
    return NULL;
}

static struct wine_xaml_node *parse_node(const WCHAR **ptr)
{
    struct wine_xaml_node *node = calloc(1, sizeof(*node));
    const WCHAR *p = *ptr;
    const WCHAR *tag_end;
    const WCHAR *bg, *fill, *fg, *txt;
    int self_closing;
    
    p = wcschr(p, '<');
    if (!p) { free(node); return NULL; }
    p++;
    while (*p == '?' || *p == '!' || *p == '/') {
        p = wcschr(p, '<');
        if (!p) { free(node); return NULL; }
        p++;
    }
    
    if (wcsncmp(p, L"Grid", 4) == 0) node->type = XAML_NODE_GRID;
    else if (wcsncmp(p, L"TextBlock", 9) == 0) node->type = XAML_NODE_TEXTBLOCK;
    else if (wcsncmp(p, L"Rectangle", 9) == 0) node->type = XAML_NODE_RECTANGLE;
    else if (wcsncmp(p, L"Border", 6) == 0) node->type = XAML_NODE_BORDER;
    else if (wcsncmp(p, L"StackPanel", 10) == 0) node->type = XAML_NODE_STACKPANEL;
    else if (wcsncmp(p, L"Image", 5) == 0) node->type = XAML_NODE_IMAGE;
    else if (wcsncmp(p, L"Viewbox", 7) == 0) node->type = XAML_NODE_VIEWBOX;
    else if (wcsncmp(p, L"Path", 4) == 0) node->type = XAML_NODE_PATH;
    else if (wcsncmp(p, L"ProgressBar", 11) == 0) node->type = XAML_NODE_PROGRESSBAR;
    else if (wcsncmp(p, L"SwapChainPanel", 14) == 0) node->type = XAML_NODE_SWAPCHAINPANEL;
    else node->type = XAML_NODE_UNKNOWN;
    
    tag_end = wcschr(p, '>');
    if (!tag_end) { free(node); return NULL; }

    /* Parse Name */
    {
        const WCHAR *name_attr = wcsstr(p, L"x:Name=\"");
        if (!name_attr || name_attr > tag_end) name_attr = wcsstr(p, L"Name=\"");
        if (name_attr && name_attr < tag_end) {
            int skip = (wcsncmp(name_attr, L"x:", 2) == 0) ? 8 : 6;
            const WCHAR *name_end = wcschr(name_attr + skip, '"');
            if (name_end) {
            size_t l = name_end - (name_attr + skip);
            if (l > 127) l = 127;
            memcpy(node->name, name_attr + skip, l * sizeof(WCHAR));
            node->name[l] = 0;
            {
                FILE *f = fopen("/tmp/xaml_debug.log", "a");
                if (f) { fprintf(f, "Parsed node type %d with name '%ls'\n", node->type, node->name); fclose(f); }
            }
        }
        }
    }
    
    bg = wcsstr(p, L"Background=\"");
    if (bg && bg < tag_end) {
        node->has_bg = 1;
        node->bg_color = parse_color(bg + 12);
    }
    fill = wcsstr(p, L"Fill=\"");
    if (fill && fill < tag_end) {
        node->has_fill = 1;
        node->fill_color = parse_color(fill + 6);
    }
    fg = wcsstr(p, L"Foreground=\"");
    if (fg && fg < tag_end) {
        node->has_fg = 1;
        node->fg_color = parse_color(fg + 12);
    }
    {
        const WCHAR *w, *h, *v, *m;
        w = wcsstr(p, L"Width=\"");
        if (w && w < tag_end) swscanf(w + 7, L"%f", &node->width);
        h = wcsstr(p, L"Height=\"");
        if (h && h < tag_end) swscanf(h + 8, L"%f", &node->height);
        v = wcsstr(p, L"Value=\"");
        if (v && v < tag_end) swscanf(v + 7, L"%lf", &node->progress_value);
        m = wcsstr(p, L"Maximum=\"");
        if (m && m < tag_end) swscanf(m + 9, L"%lf", &node->progress_max);
    }
    
    /* Parse Text or Image Source or Path Data */
    txt = wcsstr(p, L"Text=\"");
    if (!txt || txt > tag_end) txt = wcsstr(p, L"Source=\"");
    if (!txt || txt > tag_end) txt = wcsstr(p, L"Data=\"");
    
    if (txt && txt < tag_end) {
        const WCHAR *search_attr;
        if (wcsncmp(txt, L"Text", 4) == 0) search_attr = L"Text=\"";
        else if (wcsncmp(txt, L"Source", 6) == 0) search_attr = L"Source=\"";
        else search_attr = L"Data=\"";
        
        int skip = wcslen(search_attr);
        const WCHAR *txt_end = wcschr(txt + skip, '"');
        if (txt_end) {
            size_t l = txt_end - (txt + skip);
            if (l > 500) l = 500;
            size_t copy_len = l * sizeof(WCHAR);
            memcpy(node->text, txt + skip, copy_len);
            node->text[l] = 0;
            if (wcslen(wine_xaml_stub_text) < 100 && wcsncmp(txt, L"Text", 4) == 0) {
                memcpy(wine_xaml_stub_text, txt + skip, copy_len);
                wine_xaml_stub_text[l] = 0;
            }
        }
    }
    
    self_closing = (*(tag_end - 1) == '/');
    *ptr = tag_end + 1;
    
    if (!self_closing) {
        while (1) {
            struct wine_xaml_node *child;
            p = *ptr;
            p = wcschr(p, '<');
            if (!p) break;
            if (p[1] == '/') {
                *ptr = wcschr(p, '>') + 1;
                break;
            }
            child = parse_node(ptr);
            if (child) {
                if (node->child_count >= node->child_capacity) {
                    node->child_capacity = node->child_capacity ? node->child_capacity * 2 : 4;
                    node->children = realloc(node->children, node->child_capacity * sizeof(void*));
                }
                node->children[node->child_count++] = child;
            }
        }
    }
    return node;
}

void parse_wine_xaml(const WCHAR *xml)
{
    {
        FILE *f = fopen("/tmp/xaml_debug.log", "a");
        if (f) {
            fprintf(f, "==== FIVEM XAML INTERCEPT ====\nPayload Length: %d\nPayload Content:\n%ls\n==============================\n", 
                (int)wcslen(xml), xml);
            fclose(f);
        }
    }

    if (wine_xaml_root_node) free_wine_xaml_node(wine_xaml_root_node);
    wine_xaml_root_node = parse_node(&xml);
    if (wine_xaml_stub_hwnd) InvalidateRect(wine_xaml_stub_hwnd, NULL, TRUE);
}

static const IID IID_IXamlReaderStatics =
    {0x9891c6bd, 0x534f, 0x4955, {0xb8, 0x5a, 0x8a, 0x8d, 0xc0, 0xdc, 0xa6, 0x02}};
static const IID IID_IFrameworkElement =
    {0xa391d09b, 0x4a99, 0x4b7c, {0x9d, 0x8d, 0x6f, 0xa5, 0xd0, 0x1f, 0x6f, 0xbf}};
static const IID IID_IUIElement =
    {0x676d0be9, 0xb65c, 0x41c6, {0xba, 0x40, 0x58, 0xcf, 0x87, 0xf2, 0x01, 0xc1}};
static const IID IID_IProgressBar =
    {0xae752c89, 0x0067, 0x4963, {0xbf, 0x4c, 0x29, 0xdb, 0x0c, 0x4a, 0x50, 0x7e}};
static const IID IID_IRangeBase =
    {0xfa002c1a, 0x494e, 0x46cf, {0x91, 0xd4, 0xe1, 0x4a, 0x8d, 0x79, 0x86, 0x75}};
static const IID IID_ISwapChainPanel =
    {0xc589644f, 0xeba8, 0x427a, {0xb7, 0x5a, 0x9f, 0x1f, 0x93, 0xa1, 0x1a, 0xe9}};
static const IID IID_ITextBlock =
    {0xae2d9271, 0x3b4a, 0x45fc, {0x84, 0x68, 0xf7, 0x94, 0x95, 0x48, 0xf4, 0xd5}};
static const IID IID_ISwapChainPanelNative =
    {0xf92f19d2, 0x3ade, 0x45a6, {0xa2, 0x0c, 0xf6, 0xf1, 0xea, 0x90, 0x55, 0x4b}};

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

typedef struct IUIElement IUIElement;
typedef struct IUIElementVtbl
{
    BEGIN_INTERFACE
    HRESULT (WINAPI *QueryInterface)( IUIElement *iface, REFIID iid, void **out );
    ULONG (WINAPI *AddRef)( IUIElement *iface );
    ULONG (WINAPI *Release)( IUIElement *iface );
    HRESULT (WINAPI *GetIids)( IUIElement *iface, ULONG *iid_count, IID **iids );
    HRESULT (WINAPI *GetRuntimeClassName)( IUIElement *iface, HSTRING *class_name );
    HRESULT (WINAPI *GetTrustLevel)( IUIElement *iface, TrustLevel *trust_level );
    void *methods[64];
    END_INTERFACE
} IUIElementVtbl;

struct IUIElement
{
    const IUIElementVtbl *lpVtbl;
};

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
    void *methods[2];
    END_INTERFACE
} IRangeBaseVtbl;

struct IRangeBase
{
    const IRangeBaseVtbl *lpVtbl;
};

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
    void *methods[16];
    END_INTERFACE
} ITextBlockVtbl;

struct ITextBlock
{
    const ITextBlockVtbl *lpVtbl;
};

typedef struct ISwapChainPanel ISwapChainPanel;
typedef struct ISwapChainPanelVtbl
{
    BEGIN_INTERFACE
    HRESULT (WINAPI *QueryInterface)( ISwapChainPanel *iface, REFIID iid, void **out );
    ULONG (WINAPI *AddRef)( ISwapChainPanel *iface );
    ULONG (WINAPI *Release)( ISwapChainPanel *iface );
    HRESULT (WINAPI *GetIids)( ISwapChainPanel *iface, ULONG *iid_count, IID **iids );
    HRESULT (WINAPI *GetRuntimeClassName)( ISwapChainPanel *iface, HSTRING *class_name );
    HRESULT (WINAPI *GetTrustLevel)( ISwapChainPanel *iface, TrustLevel *trust_level );
    HRESULT (WINAPI *get_CompositionScaleX)( ISwapChainPanel *iface, FLOAT *value );
    HRESULT (WINAPI *get_CompositionScaleY)( ISwapChainPanel *iface, FLOAT *value );
    HRESULT (WINAPI *add_CompositionScaleChanged)( ISwapChainPanel *iface, IUnknown *handler, EventRegistrationToken *token );
    HRESULT (WINAPI *remove_CompositionScaleChanged)( ISwapChainPanel *iface, EventRegistrationToken token );
    HRESULT (WINAPI *CreateCoreIndependentInputSource)( ISwapChainPanel *iface, INT32 device_types, IInspectable **value );
    END_INTERFACE
} ISwapChainPanelVtbl;

struct ISwapChainPanel
{
    const ISwapChainPanelVtbl *lpVtbl;
};

typedef struct ISwapChainPanelNative ISwapChainPanelNative;
typedef struct ISwapChainPanelNativeVtbl
{
    BEGIN_INTERFACE
    HRESULT (WINAPI *QueryInterface)( ISwapChainPanelNative *iface, REFIID iid, void **out );
    ULONG (WINAPI *AddRef)( ISwapChainPanelNative *iface );
    ULONG (WINAPI *Release)( ISwapChainPanelNative *iface );
    HRESULT (WINAPI *SetSwapChain)( ISwapChainPanelNative *iface, IUnknown *swapchain );
    END_INTERFACE
} ISwapChainPanelNativeVtbl;

struct ISwapChainPanelNative
{
    const ISwapChainPanelNativeVtbl *lpVtbl;
};

enum xaml_reader_object_kind
{
    XAML_READER_OBJECT_FRAMEWORK_ELEMENT,
    XAML_READER_OBJECT_PROGRESS_BAR,
    XAML_READER_OBJECT_SWAPCHAIN_PANEL,
    XAML_READER_OBJECT_TEXT_BLOCK,
};

struct xaml_reader_object
{
    IFrameworkElement IFrameworkElement_iface;
    IUIElement IUIElement_iface;
    IProgressBar IProgressBar_iface;
    IRangeBase IRangeBase_iface;
    ISwapChainPanel ISwapChainPanel_iface;
    ISwapChainPanelNative ISwapChainPanelNative_iface;
    ITextBlock ITextBlock_iface;
    LONG ref;
    enum xaml_reader_object_kind kind;
    const WCHAR *runtime_class_name;
    IUnknown *swapchain;
    HSTRING text;
    DOUBLE minimum, maximum, small_change, large_change, value;
    boolean is_indeterminate, show_error, show_paused;
    struct wine_xaml_node *node;
};

struct xaml_reader_statics
{
    IActivationFactory IActivationFactory_iface;
    IXamlReaderStatics IXamlReaderStatics_iface;
    LONG ref;
};

static const WCHAR xaml_reader_name[] = L"Windows.UI.Xaml.Markup.XamlReader";
static const WCHAR framework_element_name[] = L"Windows.UI.Xaml.FrameworkElement";
static const WCHAR grid_name[] = L"Windows.UI.Xaml.Controls.Grid";
static const WCHAR progress_bar_name[] = L"Windows.UI.Xaml.Controls.ProgressBar";
static const WCHAR static1_name[] = L"static1";
static const WCHAR static2_name[] = L"static2";
static const WCHAR swap_chain_panel_name[] = L"Windows.UI.Xaml.Controls.SwapChainPanel";
static const WCHAR text_block_name[] = L"Windows.UI.Xaml.Controls.TextBlock";
static const WCHAR progress_bar_element_name[] = L"progressBar";
static const WCHAR overlay_name[] = L"Overlay";

static inline struct xaml_reader_object *impl_from_IFrameworkElement( IFrameworkElement *iface )
{
    return CONTAINING_RECORD( iface, struct xaml_reader_object, IFrameworkElement_iface );
}

static inline struct xaml_reader_object *impl_from_IUIElement( IUIElement *iface )
{
    return CONTAINING_RECORD( iface, struct xaml_reader_object, IUIElement_iface );
}

static inline struct xaml_reader_object *impl_from_IProgressBar( IProgressBar *iface )
{
    return CONTAINING_RECORD( iface, struct xaml_reader_object, IProgressBar_iface );
}

static inline struct xaml_reader_object *impl_from_IRangeBase( IRangeBase *iface )
{
    return CONTAINING_RECORD( iface, struct xaml_reader_object, IRangeBase_iface );
}

static inline struct xaml_reader_object *impl_from_ISwapChainPanel( ISwapChainPanel *iface )
{
    return CONTAINING_RECORD( iface, struct xaml_reader_object, ISwapChainPanel_iface );
}

static inline struct xaml_reader_object *impl_from_ISwapChainPanelNative( ISwapChainPanelNative *iface )
{
    return CONTAINING_RECORD( iface, struct xaml_reader_object, ISwapChainPanelNative_iface );
}

static inline struct xaml_reader_object *impl_from_ITextBlock( ITextBlock *iface )
{
    return CONTAINING_RECORD( iface, struct xaml_reader_object, ITextBlock_iface );
}

static inline struct xaml_reader_statics *impl_from_xaml_reader_IActivationFactory( IActivationFactory *iface )
{
    return CONTAINING_RECORD( iface, struct xaml_reader_statics, IActivationFactory_iface );
}

static HRESULT xaml_reader_object_create( enum xaml_reader_object_kind kind, const WCHAR *runtime_class_name,
                                          IInspectable **value );

static ULONG xaml_reader_object_addref( struct xaml_reader_object *impl )
{
    return InterlockedIncrement( &impl->ref );
}

static ULONG xaml_reader_object_release( struct xaml_reader_object *impl )
{
    ULONG ref = InterlockedDecrement( &impl->ref );
    if (!ref)
    {
        if (impl->swapchain) IUnknown_Release( impl->swapchain );
        if (impl->text) WindowsDeleteString( impl->text );
        free( impl );
    }
    return ref;
}

static HRESULT xaml_reader_named_object_create( HSTRING name, IInspectable **value )
{
    UINT32 len = 0;
    const WCHAR *buffer;

    if (!value) return E_POINTER;

    buffer = WindowsGetStringRawBuffer( name, &len );
    if (buffer && len == ARRAYSIZE( overlay_name ) - 1 && !wcsncmp( buffer, overlay_name, len ))
        return xaml_reader_object_create( XAML_READER_OBJECT_SWAPCHAIN_PANEL, swap_chain_panel_name, value );
    if (buffer && len == ARRAYSIZE( static1_name ) - 1 && !wcsncmp( buffer, static1_name, len ))
        return xaml_reader_object_create( XAML_READER_OBJECT_TEXT_BLOCK, text_block_name, value );
    if (buffer && len == ARRAYSIZE( static2_name ) - 1 && !wcsncmp( buffer, static2_name, len ))
        return xaml_reader_object_create( XAML_READER_OBJECT_TEXT_BLOCK, text_block_name, value );
    if (buffer && len == ARRAYSIZE( progress_bar_element_name ) - 1 && !wcsncmp( buffer, progress_bar_element_name, len ))
        return xaml_reader_object_create( XAML_READER_OBJECT_PROGRESS_BAR, progress_bar_name, value );
    return xaml_reader_object_create( XAML_READER_OBJECT_FRAMEWORK_ELEMENT, framework_element_name, value );
}

static HRESULT WINAPI framework_element_QueryInterface( IFrameworkElement *iface, REFIID iid, void **out )
{
    struct xaml_reader_object *impl = impl_from_IFrameworkElement( iface );

    TRACE( "iface %p, iid %s, out %p.\n", iface, debugstr_guid( iid ), out );

    if (!out) return E_POINTER;
    if (IsEqualGUID( iid, &IID_IUnknown ) || IsEqualGUID( iid, &IID_IInspectable ) ||
        IsEqualGUID( iid, &IID_IAgileObject ) || IsEqualGUID( iid, &IID_IFrameworkElement ))
    {
        *out = &impl->IFrameworkElement_iface;
    }
    else if (IsEqualGUID( iid, &IID_IUIElement ))
    {
        *out = &impl->IUIElement_iface;
    }
    else if (impl->kind == XAML_READER_OBJECT_PROGRESS_BAR && IsEqualGUID( iid, &IID_IProgressBar ))
    {
        *out = &impl->IProgressBar_iface;
    }
    else if (impl->kind == XAML_READER_OBJECT_PROGRESS_BAR && IsEqualGUID( iid, &IID_IRangeBase ))
    {
        *out = &impl->IRangeBase_iface;
    }
    else if (impl->kind == XAML_READER_OBJECT_SWAPCHAIN_PANEL && IsEqualGUID( iid, &IID_ISwapChainPanel ))
    {
        *out = &impl->ISwapChainPanel_iface;
    }
    else if (impl->kind == XAML_READER_OBJECT_SWAPCHAIN_PANEL && IsEqualGUID( iid, &IID_ISwapChainPanelNative ))
    {
        *out = &impl->ISwapChainPanelNative_iface;
    }
    else if (impl->kind == XAML_READER_OBJECT_TEXT_BLOCK && IsEqualGUID( iid, &IID_ITextBlock ))
    {
        *out = &impl->ITextBlock_iface;
    }
    else
    {
        *out = NULL;
        return E_NOINTERFACE;
    }

    xaml_reader_object_addref( impl );
    return S_OK;
}

static ULONG WINAPI framework_element_AddRef( IFrameworkElement *iface )
{
    return xaml_reader_object_addref( impl_from_IFrameworkElement( iface ) );
}

static ULONG WINAPI framework_element_Release( IFrameworkElement *iface )
{
    return xaml_reader_object_release( impl_from_IFrameworkElement( iface ) );
}

static HRESULT WINAPI framework_element_GetIids( IFrameworkElement *iface, ULONG *iid_count, IID **iids )
{
    return E_NOTIMPL;
}

static HRESULT WINAPI framework_element_GetRuntimeClassName( IFrameworkElement *iface, HSTRING *class_name )
{
    struct xaml_reader_object *impl = impl_from_IFrameworkElement( iface );

    if (!class_name) return E_POINTER;
    return WindowsCreateString( impl->runtime_class_name, wcslen( impl->runtime_class_name ), class_name );
}

static HRESULT WINAPI framework_element_GetTrustLevel( IFrameworkElement *iface, TrustLevel *trust_level )
{
    if (!trust_level) return E_POINTER;
    *trust_level = BaseTrust;
    return S_OK;
}

#define DEFINE_GET_PTR_STUB(name) \
    static HRESULT WINAPI framework_element_##name( IFrameworkElement *iface, IInspectable **value ) \
    { if (!value) return E_POINTER; *value = NULL; return S_OK; }
#define DEFINE_PUT_PTR_STUB(name) \
    static HRESULT WINAPI framework_element_##name( IFrameworkElement *iface, IInspectable *value ) \
    { return S_OK; }
#define DEFINE_GET_DBL_STUB(name) \
    static HRESULT WINAPI framework_element_##name( IFrameworkElement *iface, DOUBLE *value ) \
    { if (!value) return E_POINTER; *value = 0.0; return S_OK; }
#define DEFINE_PUT_DBL_STUB(name) \
    static HRESULT WINAPI framework_element_##name( IFrameworkElement *iface, DOUBLE value ) \
    { return S_OK; }
#define DEFINE_GET_I32_STUB(name) \
    static HRESULT WINAPI framework_element_##name( IFrameworkElement *iface, INT32 *value ) \
    { if (!value) return E_POINTER; *value = 0; return S_OK; }
#define DEFINE_PUT_I32_STUB(name) \
    static HRESULT WINAPI framework_element_##name( IFrameworkElement *iface, INT32 value ) \
    { return S_OK; }
#define DEFINE_ADD_EVENT_STUB(name) \
    static HRESULT WINAPI framework_element_##name( IFrameworkElement *iface, IInspectable *handler, EventRegistrationToken *token ) \
    { if (!token) return E_POINTER; token->value = 0; return S_OK; }
#define DEFINE_REMOVE_EVENT_STUB(name) \
    static HRESULT WINAPI framework_element_##name( IFrameworkElement *iface, EventRegistrationToken token ) \
    { return S_OK; }

DEFINE_GET_PTR_STUB( get_Triggers )
DEFINE_GET_PTR_STUB( get_Resources )
DEFINE_PUT_PTR_STUB( put_Resources )
DEFINE_GET_PTR_STUB( get_Tag )
DEFINE_PUT_PTR_STUB( put_Tag )
DEFINE_GET_DBL_STUB( get_ActualWidth )
DEFINE_GET_DBL_STUB( get_ActualHeight )
DEFINE_GET_DBL_STUB( get_Width )
DEFINE_PUT_DBL_STUB( put_Width )
DEFINE_GET_DBL_STUB( get_Height )
DEFINE_PUT_DBL_STUB( put_Height )
DEFINE_GET_DBL_STUB( get_MinWidth )
DEFINE_PUT_DBL_STUB( put_MinWidth )
DEFINE_GET_DBL_STUB( get_MaxWidth )
DEFINE_PUT_DBL_STUB( put_MaxWidth )
DEFINE_GET_DBL_STUB( get_MinHeight )
DEFINE_PUT_DBL_STUB( put_MinHeight )
DEFINE_GET_DBL_STUB( get_MaxHeight )
DEFINE_PUT_DBL_STUB( put_MaxHeight )
DEFINE_GET_I32_STUB( get_HorizontalAlignment )
DEFINE_PUT_I32_STUB( put_HorizontalAlignment )
DEFINE_GET_I32_STUB( get_VerticalAlignment )
DEFINE_PUT_I32_STUB( put_VerticalAlignment )
DEFINE_GET_PTR_STUB( get_BaseUri )
DEFINE_GET_PTR_STUB( get_DataContext )
DEFINE_PUT_PTR_STUB( put_DataContext )
DEFINE_GET_PTR_STUB( get_Style )
DEFINE_PUT_PTR_STUB( put_Style )
DEFINE_GET_PTR_STUB( get_Parent )
DEFINE_GET_I32_STUB( get_FlowDirection )
DEFINE_PUT_I32_STUB( put_FlowDirection )
DEFINE_ADD_EVENT_STUB( add_Loaded )
DEFINE_REMOVE_EVENT_STUB( remove_Loaded )
DEFINE_ADD_EVENT_STUB( add_Unloaded )
DEFINE_REMOVE_EVENT_STUB( remove_Unloaded )
DEFINE_ADD_EVENT_STUB( add_SizeChanged )
DEFINE_REMOVE_EVENT_STUB( remove_SizeChanged )
DEFINE_ADD_EVENT_STUB( add_LayoutUpdated )
DEFINE_REMOVE_EVENT_STUB( remove_LayoutUpdated )

static HRESULT WINAPI framework_element_get_Language( IFrameworkElement *iface, HSTRING *value )
{
    if (!value) return E_POINTER;
    *value = NULL;
    return S_OK;
}

static HRESULT WINAPI framework_element_put_Language( IFrameworkElement *iface, HSTRING value )
{
    return S_OK;
}

static HRESULT WINAPI framework_element_get_Margin( IFrameworkElement *iface, void *value )
{
    if (!value) return E_POINTER;
    memset( value, 0, sizeof(DOUBLE) * 4 );
    return S_OK;
}

static HRESULT WINAPI framework_element_put_Margin( IFrameworkElement *iface, const void *value )
{
    return S_OK;
}

static HRESULT WINAPI framework_element_get_Name( IFrameworkElement *iface, HSTRING *value )
{
    if (!value) return E_POINTER;
    *value = NULL;
    return S_OK;
}

static HRESULT WINAPI framework_element_put_Name( IFrameworkElement *iface, HSTRING value )
{
    return S_OK;
}

static HRESULT WINAPI framework_element_FindName( IFrameworkElement *iface, HSTRING name, IInspectable **result )
    {
        const WCHAR *name_str = WindowsGetStringRawBuffer( name, NULL );
        struct wine_xaml_node *found = find_node_by_name(wine_xaml_root_node, name_str);
        struct xaml_reader_object *obj;
    
        ERR( "==== XAML FindName ==== iface %p, name %s, found_node %p.\n", iface, debugstr_hstring( name ), found );
        
        if (found) {
            enum xaml_reader_object_kind kind = XAML_READER_OBJECT_FRAMEWORK_ELEMENT;
            if (found->type == XAML_NODE_TEXTBLOCK) kind = XAML_READER_OBJECT_TEXT_BLOCK;
            else if (found->type == XAML_NODE_PROGRESSBAR) kind = XAML_READER_OBJECT_PROGRESS_BAR;
            else if (found->type == XAML_NODE_SWAPCHAINPANEL) kind = XAML_READER_OBJECT_SWAPCHAIN_PANEL;
            
            xaml_reader_object_create(kind, name_str, result);
            obj = impl_from_IFrameworkElement((IFrameworkElement *)*result);
            obj->node = found;

            if (found->type == XAML_NODE_PROGRESSBAR) {
                obj->value = found->progress_value;
                obj->maximum = found->progress_max;
            }
            return S_OK;
        }
    
        return xaml_reader_named_object_create( name, result );
    }

static HRESULT WINAPI framework_element_SetBinding( IFrameworkElement *iface, IInspectable *dp, IInspectable *binding,
                                                     IInspectable **result )
{
    if (!result) return E_POINTER;
    *result = NULL;
    return S_OK;
}

static const IFrameworkElementVtbl framework_element_vtbl =
{
    framework_element_QueryInterface,
    framework_element_AddRef,
    framework_element_Release,
    framework_element_GetIids,
    framework_element_GetRuntimeClassName,
    framework_element_GetTrustLevel,
    framework_element_get_Triggers,
    framework_element_get_Resources,
    framework_element_put_Resources,
    framework_element_get_Tag,
    framework_element_put_Tag,
    framework_element_get_Language,
    framework_element_put_Language,
    framework_element_get_ActualWidth,
    framework_element_get_ActualHeight,
    framework_element_get_Width,
    framework_element_put_Width,
    framework_element_get_Height,
    framework_element_put_Height,
    framework_element_get_MinWidth,
    framework_element_put_MinWidth,
    framework_element_get_MaxWidth,
    framework_element_put_MaxWidth,
    framework_element_get_MinHeight,
    framework_element_put_MinHeight,
    framework_element_get_MaxHeight,
    framework_element_put_MaxHeight,
    framework_element_get_HorizontalAlignment,
    framework_element_put_HorizontalAlignment,
    framework_element_get_VerticalAlignment,
    framework_element_put_VerticalAlignment,
    framework_element_get_Margin,
    framework_element_put_Margin,
    framework_element_get_Name,
    framework_element_put_Name,
    framework_element_get_BaseUri,
    framework_element_get_DataContext,
    framework_element_put_DataContext,
    framework_element_get_Style,
    framework_element_put_Style,
    framework_element_get_Parent,
    framework_element_get_FlowDirection,
    framework_element_put_FlowDirection,
    framework_element_add_Loaded,
    framework_element_remove_Loaded,
    framework_element_add_Unloaded,
    framework_element_remove_Unloaded,
    framework_element_add_SizeChanged,
    framework_element_remove_SizeChanged,
    framework_element_add_LayoutUpdated,
    framework_element_remove_LayoutUpdated,
    framework_element_FindName,
    framework_element_SetBinding,
};

static HRESULT WINAPI ui_element_QueryInterface( IUIElement *iface, REFIID iid, void **out )
{
    return framework_element_QueryInterface( &impl_from_IUIElement( iface )->IFrameworkElement_iface, iid, out );
}

static ULONG WINAPI ui_element_AddRef( IUIElement *iface )
{
    return framework_element_AddRef( &impl_from_IUIElement( iface )->IFrameworkElement_iface );
}

static ULONG WINAPI ui_element_Release( IUIElement *iface )
{
    return framework_element_Release( &impl_from_IUIElement( iface )->IFrameworkElement_iface );
}

static HRESULT WINAPI ui_element_GetIids( IUIElement *iface, ULONG *iid_count, IID **iids )
{
    return framework_element_GetIids( &impl_from_IUIElement( iface )->IFrameworkElement_iface, iid_count, iids );
}

static HRESULT WINAPI ui_element_GetRuntimeClassName( IUIElement *iface, HSTRING *class_name )
{
    return framework_element_GetRuntimeClassName( &impl_from_IUIElement( iface )->IFrameworkElement_iface, class_name );
}

static HRESULT WINAPI ui_element_GetTrustLevel( IUIElement *iface, TrustLevel *trust_level )
{
    return framework_element_GetTrustLevel( &impl_from_IUIElement( iface )->IFrameworkElement_iface, trust_level );
}

static const IUIElementVtbl ui_element_vtbl =
{
    ui_element_QueryInterface,
    ui_element_AddRef,
    ui_element_Release,
    ui_element_GetIids,
    ui_element_GetRuntimeClassName,
    ui_element_GetTrustLevel,
};

static HRESULT WINAPI progress_bar_QueryInterface( IProgressBar *iface, REFIID iid, void **out )
{
    return framework_element_QueryInterface( &impl_from_IProgressBar( iface )->IFrameworkElement_iface, iid, out );
}

static ULONG WINAPI progress_bar_AddRef( IProgressBar *iface )
{
    return framework_element_AddRef( &impl_from_IProgressBar( iface )->IFrameworkElement_iface );
}

static ULONG WINAPI progress_bar_Release( IProgressBar *iface )
{
    return framework_element_Release( &impl_from_IProgressBar( iface )->IFrameworkElement_iface );
}

static HRESULT WINAPI progress_bar_GetIids( IProgressBar *iface, ULONG *iid_count, IID **iids )
{
    return framework_element_GetIids( &impl_from_IProgressBar( iface )->IFrameworkElement_iface, iid_count, iids );
}

static HRESULT WINAPI progress_bar_GetRuntimeClassName( IProgressBar *iface, HSTRING *class_name )
{
    return framework_element_GetRuntimeClassName( &impl_from_IProgressBar( iface )->IFrameworkElement_iface, class_name );
}

static HRESULT WINAPI progress_bar_GetTrustLevel( IProgressBar *iface, TrustLevel *trust_level )
{
    return framework_element_GetTrustLevel( &impl_from_IProgressBar( iface )->IFrameworkElement_iface, trust_level );
}

static HRESULT WINAPI progress_bar_get_IsIndeterminate( IProgressBar *iface, boolean *value )
{
    struct xaml_reader_object *impl = impl_from_IProgressBar( iface );
    if (!value) return E_POINTER;
    *value = impl->is_indeterminate;
    return S_OK;
}

static HRESULT WINAPI progress_bar_put_IsIndeterminate( IProgressBar *iface, boolean value )
{
    TRACE( "iface %p, value %u.\n", iface, value );
    impl_from_IProgressBar( iface )->is_indeterminate = value;
    return S_OK;
}

static HRESULT WINAPI progress_bar_get_ShowError( IProgressBar *iface, boolean *value )
{
    struct xaml_reader_object *impl = impl_from_IProgressBar( iface );
    if (!value) return E_POINTER;
    *value = impl->show_error;
    return S_OK;
}

static HRESULT WINAPI progress_bar_put_ShowError( IProgressBar *iface, boolean value )
{
    impl_from_IProgressBar( iface )->show_error = value;
    return S_OK;
}

static HRESULT WINAPI progress_bar_get_ShowPaused( IProgressBar *iface, boolean *value )
{
    struct xaml_reader_object *impl = impl_from_IProgressBar( iface );
    if (!value) return E_POINTER;
    *value = impl->show_paused;
    return S_OK;
}

static HRESULT WINAPI progress_bar_put_ShowPaused( IProgressBar *iface, boolean value )
{
    impl_from_IProgressBar( iface )->show_paused = value;
    return S_OK;
}

static HRESULT WINAPI progress_bar_get_TemplateSettings( IProgressBar *iface, IInspectable **value )
{
    if (!value) return E_POINTER;
    *value = NULL;
    return S_OK;
}

static const IProgressBarVtbl progress_bar_vtbl =
{
    progress_bar_QueryInterface,
    progress_bar_AddRef,
    progress_bar_Release,
    progress_bar_GetIids,
    progress_bar_GetRuntimeClassName,
    progress_bar_GetTrustLevel,
    progress_bar_get_IsIndeterminate,
    progress_bar_put_IsIndeterminate,
    progress_bar_get_ShowError,
    progress_bar_put_ShowError,
    progress_bar_get_ShowPaused,
    progress_bar_put_ShowPaused,
    progress_bar_get_TemplateSettings,
};


static HRESULT WINAPI range_base_QueryInterface( IRangeBase *iface, REFIID iid, void **out )
{
    return framework_element_QueryInterface( &impl_from_IRangeBase( iface )->IFrameworkElement_iface, iid, out );
}

static ULONG WINAPI range_base_AddRef( IRangeBase *iface )
{
    return framework_element_AddRef( &impl_from_IRangeBase( iface )->IFrameworkElement_iface );
}

static ULONG WINAPI range_base_Release( IRangeBase *iface )
{
    return framework_element_Release( &impl_from_IRangeBase( iface )->IFrameworkElement_iface );
}

static HRESULT WINAPI range_base_GetIids( IRangeBase *iface, ULONG *iid_count, IID **iids )
{
    return framework_element_GetIids( &impl_from_IRangeBase( iface )->IFrameworkElement_iface, iid_count, iids );
}

static HRESULT WINAPI range_base_GetRuntimeClassName( IRangeBase *iface, HSTRING *class_name )
{
    return framework_element_GetRuntimeClassName( &impl_from_IRangeBase( iface )->IFrameworkElement_iface, class_name );
}

static HRESULT WINAPI range_base_GetTrustLevel( IRangeBase *iface, TrustLevel *trust_level )
{
    return framework_element_GetTrustLevel( &impl_from_IRangeBase( iface )->IFrameworkElement_iface, trust_level );
}

static HRESULT WINAPI range_base_get_Minimum( IRangeBase *iface, DOUBLE *value )
{
    struct xaml_reader_object *impl = impl_from_IRangeBase( iface );
    if (!value) return E_POINTER;
    *value = impl->minimum;
    return S_OK;
}

static HRESULT WINAPI range_base_put_Minimum( IRangeBase *iface, DOUBLE value )
{
    impl_from_IRangeBase( iface )->minimum = value;
    return S_OK;
}

static HRESULT WINAPI range_base_get_Maximum( IRangeBase *iface, DOUBLE *value )
{
    struct xaml_reader_object *impl = impl_from_IRangeBase( iface );
    if (!value) return E_POINTER;
    *value = impl->maximum;
    return S_OK;
}

static HRESULT WINAPI range_base_put_Maximum( IRangeBase *iface, DOUBLE value )
{
    struct xaml_reader_object *impl = impl_from_IRangeBase( iface );
    TRACE( "iface %p, value %f.\n", iface, value );
    impl->maximum = value;
    if (impl->node) {
        impl->node->progress_max = value;
        if (wine_xaml_stub_hwnd) InvalidateRect(wine_xaml_stub_hwnd, NULL, TRUE);
    }
    return S_OK;
}

static HRESULT WINAPI range_base_get_SmallChange( IRangeBase *iface, DOUBLE *value )
{
    struct xaml_reader_object *impl = impl_from_IRangeBase( iface );
    if (!value) return E_POINTER;
    *value = impl->small_change;
    return S_OK;
}

static HRESULT WINAPI range_base_put_SmallChange( IRangeBase *iface, DOUBLE value )
{
    impl_from_IRangeBase( iface )->small_change = value;
    return S_OK;
}

static HRESULT WINAPI range_base_get_LargeChange( IRangeBase *iface, DOUBLE *value )
{
    struct xaml_reader_object *impl = impl_from_IRangeBase( iface );
    if (!value) return E_POINTER;
    *value = impl->large_change;
    return S_OK;
}

static HRESULT WINAPI range_base_put_LargeChange( IRangeBase *iface, DOUBLE value )
{
    impl_from_IRangeBase( iface )->large_change = value;
    return S_OK;
}

static HRESULT WINAPI range_base_get_Value( IRangeBase *iface, DOUBLE *value )
{
    struct xaml_reader_object *impl = impl_from_IRangeBase( iface );
    if (!value) return E_POINTER;
    *value = impl->value;
    return S_OK;
}

static HRESULT WINAPI range_base_put_Value( IRangeBase *iface, DOUBLE value )
{
    TRACE( "iface %p, value %f.\n", iface, value );
    impl_from_IRangeBase( iface )->value = value;
    return S_OK;
}

static const IRangeBaseVtbl range_base_vtbl =
{
    range_base_QueryInterface,
    range_base_AddRef,
    range_base_Release,
    range_base_GetIids,
    range_base_GetRuntimeClassName,
    range_base_GetTrustLevel,
    range_base_get_Minimum,
    range_base_put_Minimum,
    range_base_get_Maximum,
    range_base_put_Maximum,
    range_base_get_SmallChange,
    range_base_put_SmallChange,
    range_base_get_LargeChange,
    range_base_put_LargeChange,
    range_base_get_Value,
    range_base_put_Value,
};

static HRESULT WINAPI text_block_QueryInterface( ITextBlock *iface, REFIID iid, void **out )
{
    return framework_element_QueryInterface( &impl_from_ITextBlock( iface )->IFrameworkElement_iface, iid, out );
}

static ULONG WINAPI text_block_AddRef( ITextBlock *iface )
{
    return framework_element_AddRef( &impl_from_ITextBlock( iface )->IFrameworkElement_iface );
}

static ULONG WINAPI text_block_Release( ITextBlock *iface )
{
    return framework_element_Release( &impl_from_ITextBlock( iface )->IFrameworkElement_iface );
}

static HRESULT WINAPI text_block_GetIids( ITextBlock *iface, ULONG *iid_count, IID **iids )
{
    return framework_element_GetIids( &impl_from_ITextBlock( iface )->IFrameworkElement_iface, iid_count, iids );
}

static HRESULT WINAPI text_block_GetRuntimeClassName( ITextBlock *iface, HSTRING *class_name )
{
    return framework_element_GetRuntimeClassName( &impl_from_ITextBlock( iface )->IFrameworkElement_iface, class_name );
}

static HRESULT WINAPI text_block_GetTrustLevel( ITextBlock *iface, TrustLevel *trust_level )
{
    return framework_element_GetTrustLevel( &impl_from_ITextBlock( iface )->IFrameworkElement_iface, trust_level );
}

static HRESULT WINAPI text_block_get_FontSize( ITextBlock *iface, DOUBLE *value )
{
    if (!value) return E_POINTER;
    *value = 0.0;
    return S_OK;
}

static HRESULT WINAPI text_block_put_FontSize( ITextBlock *iface, DOUBLE value )
{
    return S_OK;
}

static HRESULT WINAPI text_block_get_FontFamily( ITextBlock *iface, IInspectable **value )
{
    if (!value) return E_POINTER;
    *value = NULL;
    return S_OK;
}

static HRESULT WINAPI text_block_put_FontFamily( ITextBlock *iface, IInspectable *value )
{
    return S_OK;
}

static HRESULT WINAPI text_block_get_FontWeight( ITextBlock *iface, UINT32 *value )
{
    if (!value) return E_POINTER;
    *value = 0;
    return S_OK;
}

static HRESULT WINAPI text_block_put_FontWeight( ITextBlock *iface, UINT32 value )
{
    return S_OK;
}

static HRESULT WINAPI text_block_get_FontStyle( ITextBlock *iface, INT32 *value )
{
    if (!value) return E_POINTER;
    *value = 0;
    return S_OK;
}

static HRESULT WINAPI text_block_put_FontStyle( ITextBlock *iface, INT32 value )
{
    return S_OK;
}

static HRESULT WINAPI text_block_get_FontStretch( ITextBlock *iface, INT32 *value )
{
    if (!value) return E_POINTER;
    *value = 0;
    return S_OK;
}

static HRESULT WINAPI text_block_put_FontStretch( ITextBlock *iface, INT32 value )
{
    return S_OK;
}

static HRESULT WINAPI text_block_get_CharacterSpacing( ITextBlock *iface, INT32 *value )
{
    if (!value) return E_POINTER;
    *value = 0;
    return S_OK;
}

static HRESULT WINAPI text_block_put_CharacterSpacing( ITextBlock *iface, INT32 value )
{
    return S_OK;
}

static HRESULT WINAPI text_block_get_Foreground( ITextBlock *iface, IInspectable **value )
{
    if (!value) return E_POINTER;
    *value = NULL;
    return S_OK;
}

static HRESULT WINAPI text_block_put_Foreground( ITextBlock *iface, IInspectable *value )
{
    return S_OK;
}

static HRESULT WINAPI text_block_get_TextWrapping( ITextBlock *iface, INT32 *value )
{
    if (!value) return E_POINTER;
    *value = 0;
    return S_OK;
}

static HRESULT WINAPI text_block_put_TextWrapping( ITextBlock *iface, INT32 value )
{
    return S_OK;
}

static HRESULT WINAPI text_block_get_TextTrimming( ITextBlock *iface, INT32 *value )
{
    if (!value) return E_POINTER;
    *value = 0;
    return S_OK;
}

static HRESULT WINAPI text_block_put_TextTrimming( ITextBlock *iface, INT32 value )
{
    return S_OK;
}

static HRESULT WINAPI text_block_get_TextAlignment( ITextBlock *iface, INT32 *value )
{
    if (!value) return E_POINTER;
    *value = 0;
    return S_OK;
}

static HRESULT WINAPI text_block_put_TextAlignment( ITextBlock *iface, INT32 value )
{
    return S_OK;
}

static HRESULT WINAPI text_block_get_Text( ITextBlock *iface, HSTRING *value )
{
    struct xaml_reader_object *impl = impl_from_ITextBlock( iface );

    if (!value) return E_POINTER;
    if (impl->text) return WindowsDuplicateString( impl->text, value );
    return WindowsCreateString( L"", 0, value );
}

static HRESULT WINAPI text_block_put_Text( ITextBlock *iface, HSTRING value )
{
    struct xaml_reader_object *impl = impl_from_ITextBlock( iface );
    HSTRING copy = NULL;
    HRESULT hr;

    TRACE( "iface %p, value %s.\n", iface, debugstr_hstring( value ) );

    if (value && FAILED(hr = WindowsDuplicateString( value, &copy )))
        return hr;

    if (impl->text) WindowsDeleteString( impl->text );
    impl->text = copy;
    
    /* WINE STUB HACK: Capture dynamic text updates into the DOM */
    if (value)
    {
        UINT32 len;
        const WCHAR *str = WindowsGetStringRawBuffer( value, &len );
        if (str && len < 4000)
        {
            {
                FILE *f = fopen("/tmp/xaml_debug.log", "a");
                if (f) {
                    fprintf(f, "==== FIVEM DYNAMIC TEXT UPDATE ====\nNew Text: %ls\n=================================\n", str);
                    fclose(f);
                }
            }
            if (impl->node)
            {
                UINT32 copy_count = len > 1023 ? 1023 : len;
                memcpy(impl->node->text, str, copy_count * sizeof(WCHAR));
                impl->node->text[copy_count] = 0;
            }
            else
            {
                /* Fallback: Update global text if object isn't linked to a node */
                UINT32 copy_count = len > 4095 ? 4095 : len;
                memcpy(wine_xaml_stub_text, str, copy_count * sizeof(WCHAR));
                wine_xaml_stub_text[copy_count] = 0;
            }
            
            if (wine_xaml_stub_hwnd) InvalidateRect(wine_xaml_stub_hwnd, NULL, TRUE);
        }
    }

    return S_OK;
}

static const ITextBlockVtbl text_block_vtbl =
{
    text_block_QueryInterface,
    text_block_AddRef,
    text_block_Release,
    text_block_GetIids,
    text_block_GetRuntimeClassName,
    text_block_GetTrustLevel,
    text_block_get_FontSize,
    text_block_put_FontSize,
    text_block_get_FontFamily,
    text_block_put_FontFamily,
    text_block_get_FontWeight,
    text_block_put_FontWeight,
    text_block_get_FontStyle,
    text_block_put_FontStyle,
    text_block_get_FontStretch,
    text_block_put_FontStretch,
    text_block_get_CharacterSpacing,
    text_block_put_CharacterSpacing,
    text_block_get_Foreground,
    text_block_put_Foreground,
    text_block_get_TextWrapping,
    text_block_put_TextWrapping,
    text_block_get_TextTrimming,
    text_block_put_TextTrimming,
    text_block_get_TextAlignment,
    text_block_put_TextAlignment,
    text_block_get_Text,
    text_block_put_Text,
};


static HRESULT WINAPI swap_chain_panel_QueryInterface( ISwapChainPanel *iface, REFIID iid, void **out )
{
    return framework_element_QueryInterface( &impl_from_ISwapChainPanel( iface )->IFrameworkElement_iface, iid, out );
}

static ULONG WINAPI swap_chain_panel_AddRef( ISwapChainPanel *iface )
{
    return framework_element_AddRef( &impl_from_ISwapChainPanel( iface )->IFrameworkElement_iface );
}

static ULONG WINAPI swap_chain_panel_Release( ISwapChainPanel *iface )
{
    return framework_element_Release( &impl_from_ISwapChainPanel( iface )->IFrameworkElement_iface );
}

static HRESULT WINAPI swap_chain_panel_GetIids( ISwapChainPanel *iface, ULONG *iid_count, IID **iids )
{
    return framework_element_GetIids( &impl_from_ISwapChainPanel( iface )->IFrameworkElement_iface, iid_count, iids );
}

static HRESULT WINAPI swap_chain_panel_GetRuntimeClassName( ISwapChainPanel *iface, HSTRING *class_name )
{
    return framework_element_GetRuntimeClassName( &impl_from_ISwapChainPanel( iface )->IFrameworkElement_iface, class_name );
}

static HRESULT WINAPI swap_chain_panel_GetTrustLevel( ISwapChainPanel *iface, TrustLevel *trust_level )
{
    return framework_element_GetTrustLevel( &impl_from_ISwapChainPanel( iface )->IFrameworkElement_iface, trust_level );
}

static HRESULT WINAPI swap_chain_panel_get_CompositionScaleX( ISwapChainPanel *iface, FLOAT *value )
{
    if (!value) return E_POINTER;
    *value = 1.0f;
    return S_OK;
}

static HRESULT WINAPI swap_chain_panel_get_CompositionScaleY( ISwapChainPanel *iface, FLOAT *value )
{
    if (!value) return E_POINTER;
    *value = 1.0f;
    return S_OK;
}

static HRESULT WINAPI swap_chain_panel_add_CompositionScaleChanged( ISwapChainPanel *iface, IUnknown *handler,
                                                                    EventRegistrationToken *token )
{
    if (!token) return E_POINTER;
    token->value = 0;
    return S_OK;
}

static HRESULT WINAPI swap_chain_panel_remove_CompositionScaleChanged( ISwapChainPanel *iface, EventRegistrationToken token )
{
    return S_OK;
}

static HRESULT WINAPI swap_chain_panel_CreateCoreIndependentInputSource( ISwapChainPanel *iface, INT32 device_types,
                                                                         IInspectable **value )
{
    if (!value) return E_POINTER;
    *value = NULL;
    return E_NOTIMPL;
}

static const ISwapChainPanelVtbl swap_chain_panel_vtbl =
{
    swap_chain_panel_QueryInterface,
    swap_chain_panel_AddRef,
    swap_chain_panel_Release,
    swap_chain_panel_GetIids,
    swap_chain_panel_GetRuntimeClassName,
    swap_chain_panel_GetTrustLevel,
    swap_chain_panel_get_CompositionScaleX,
    swap_chain_panel_get_CompositionScaleY,
    swap_chain_panel_add_CompositionScaleChanged,
    swap_chain_panel_remove_CompositionScaleChanged,
    swap_chain_panel_CreateCoreIndependentInputSource,
};

static HRESULT WINAPI swap_chain_panel_native_QueryInterface( ISwapChainPanelNative *iface, REFIID iid, void **out )
{
    return framework_element_QueryInterface( &impl_from_ISwapChainPanelNative( iface )->IFrameworkElement_iface, iid, out );
}

static ULONG WINAPI swap_chain_panel_native_AddRef( ISwapChainPanelNative *iface )
{
    return framework_element_AddRef( &impl_from_ISwapChainPanelNative( iface )->IFrameworkElement_iface );
}

static ULONG WINAPI swap_chain_panel_native_Release( ISwapChainPanelNative *iface )
{
    return framework_element_Release( &impl_from_ISwapChainPanelNative( iface )->IFrameworkElement_iface );
}

static HRESULT WINAPI swap_chain_panel_native_SetSwapChain( ISwapChainPanelNative *iface, IUnknown *swapchain )
{
    struct xaml_reader_object *impl = impl_from_ISwapChainPanelNative( iface );

    ERR( "==== XAML SetSwapChain ==== iface %p, swapchain %p.\n", iface, swapchain );

    if (swapchain) IUnknown_AddRef( swapchain );
    if (impl->swapchain) IUnknown_Release( impl->swapchain );
    impl->swapchain = swapchain;
    return S_OK;
}

static const ISwapChainPanelNativeVtbl swap_chain_panel_native_vtbl =
{
    swap_chain_panel_native_QueryInterface,
    swap_chain_panel_native_AddRef,
    swap_chain_panel_native_Release,
    swap_chain_panel_native_SetSwapChain,
};

static HRESULT xaml_reader_object_create( enum xaml_reader_object_kind kind, const WCHAR *runtime_class_name,
                                          IInspectable **value )
{
    struct xaml_reader_object *impl;

    if (!value) return E_POINTER;
    if (!(impl = calloc( 1, sizeof(*impl) ))) return E_OUTOFMEMORY;

    impl->IFrameworkElement_iface.lpVtbl = &framework_element_vtbl;
    impl->IUIElement_iface.lpVtbl = &ui_element_vtbl;
    impl->IProgressBar_iface.lpVtbl = &progress_bar_vtbl;
    impl->IRangeBase_iface.lpVtbl = &range_base_vtbl;
    impl->ISwapChainPanel_iface.lpVtbl = &swap_chain_panel_vtbl;
    impl->ISwapChainPanelNative_iface.lpVtbl = &swap_chain_panel_native_vtbl;
    impl->ITextBlock_iface.lpVtbl = &text_block_vtbl;
    impl->ref = 1;
    impl->kind = kind;
    impl->runtime_class_name = runtime_class_name;
    impl->maximum = 100.0;
    *value = (IInspectable *)&impl->IFrameworkElement_iface;
    return S_OK;
}

static HRESULT WINAPI xaml_reader_factory_QueryInterface( IActivationFactory *iface, REFIID iid, void **out )
{
    struct xaml_reader_statics *impl = impl_from_xaml_reader_IActivationFactory( iface );

    TRACE( "iface %p, iid %s, out %p.\n", iface, debugstr_guid( iid ), out );

    if (!out) return E_POINTER;
    if (IsEqualGUID( iid, &IID_IUnknown ) || IsEqualGUID( iid, &IID_IInspectable ) ||
        IsEqualGUID( iid, &IID_IAgileObject ) || IsEqualGUID( iid, &IID_IActivationFactory ))
    {
        *out = &impl->IActivationFactory_iface;
        IInspectable_AddRef( (IInspectable *)*out );
        return S_OK;
    }
    if (IsEqualGUID( iid, &IID_IXamlReaderStatics ))
    {
        *out = &impl->IXamlReaderStatics_iface;
        IInspectable_AddRef( (IInspectable *)*out );
        return S_OK;
    }
    *out = NULL;
    return E_NOINTERFACE;
}

static ULONG WINAPI xaml_reader_factory_AddRef( IActivationFactory *iface )
{
    struct xaml_reader_statics *impl = impl_from_xaml_reader_IActivationFactory( iface );
    return InterlockedIncrement( &impl->ref );
}

static ULONG WINAPI xaml_reader_factory_Release( IActivationFactory *iface )
{
    struct xaml_reader_statics *impl = impl_from_xaml_reader_IActivationFactory( iface );
    return InterlockedDecrement( &impl->ref );
}

static HRESULT WINAPI xaml_reader_factory_GetIids( IActivationFactory *iface, ULONG *iid_count, IID **iids )
{
    return E_NOTIMPL;
}

static HRESULT WINAPI xaml_reader_factory_GetRuntimeClassName( IActivationFactory *iface, HSTRING *class_name )
{
    if (!class_name) return E_POINTER;
    return WindowsCreateString( xaml_reader_name, ARRAYSIZE( xaml_reader_name ) - 1, class_name );
}

static HRESULT WINAPI xaml_reader_factory_GetTrustLevel( IActivationFactory *iface, TrustLevel *trust_level )
{
    if (!trust_level) return E_POINTER;
    *trust_level = BaseTrust;
    return S_OK;
}

static HRESULT WINAPI xaml_reader_factory_ActivateInstance( IActivationFactory *iface, IInspectable **instance )
{
    return E_NOTIMPL;
}

static const struct IActivationFactoryVtbl xaml_reader_factory_vtbl =
{
    xaml_reader_factory_QueryInterface,
    xaml_reader_factory_AddRef,
    xaml_reader_factory_Release,
    xaml_reader_factory_GetIids,
    xaml_reader_factory_GetRuntimeClassName,
    xaml_reader_factory_GetTrustLevel,
    xaml_reader_factory_ActivateInstance,
};

DEFINE_IINSPECTABLE_( xaml_reader_statics, IXamlReaderStatics, struct xaml_reader_statics,
                      impl_from_IXamlReaderStatics, IXamlReaderStatics_iface, &impl->IActivationFactory_iface )

static HRESULT WINAPI xaml_reader_statics_Load( IXamlReaderStatics *iface, HSTRING xaml, IInspectable **value )
{
    TRACE( "iface %p, xaml %s, value %p.\n", iface, debugstr_hstring( xaml ), value );
    if (!value) return E_POINTER;
    if (!xaml) return E_INVALIDARG;
    
    /* WINE STUB HACK: Extract XAML text for our spoofed UI */
    {
        UINT32 len;
        const WCHAR *str = WindowsGetStringRawBuffer( xaml, &len );
        if (str) parse_wine_xaml(str);
    }

    return xaml_reader_object_create( XAML_READER_OBJECT_FRAMEWORK_ELEMENT, grid_name, value );
}

static HRESULT WINAPI xaml_reader_statics_LoadWithInitialTemplateValidation( IXamlReaderStatics *iface, HSTRING xaml,
                                                                             IInspectable **value )
{
    TRACE( "iface %p, xaml %s, value %p.\n", iface, debugstr_hstring( xaml ), value );
    if (!value) return E_POINTER;
    if (!xaml) return E_INVALIDARG;
    return xaml_reader_object_create( XAML_READER_OBJECT_FRAMEWORK_ELEMENT, grid_name, value );
}

static const IXamlReaderStaticsVtbl xaml_reader_statics_vtbl =
{
    xaml_reader_statics_QueryInterface,
    xaml_reader_statics_AddRef,
    xaml_reader_statics_Release,
    xaml_reader_statics_GetIids,
    xaml_reader_statics_GetRuntimeClassName,
    xaml_reader_statics_GetTrustLevel,
    xaml_reader_statics_Load,
    xaml_reader_statics_LoadWithInitialTemplateValidation,
};

static struct xaml_reader_statics xaml_reader_statics_instance =
{
    {&xaml_reader_factory_vtbl},
    {&xaml_reader_statics_vtbl},
    1,
};

IActivationFactory *xaml_reader_factory = &xaml_reader_statics_instance.IActivationFactory_iface;
