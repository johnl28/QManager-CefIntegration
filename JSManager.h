#ifndef _JS_MGR_H_
#define _JS_MGR_H_

#include <functional>


#define JS_FUNC(name) void name(CefRefPtr<CefV8Value>& retVal, const CefV8ValueList& args, CefRefPtr<CefBrowser> browser)

typedef struct jsf
{
    enum
    {
        FUNC_GLOBAL,
        FUNC_CONFIG
    };

    std::function<void(CefRefPtr<CefV8Value>&,
        const CefV8ValueList&,
        CefRefPtr<CefBrowser>)> function;

    int type;

} TJSfunction;

typedef std::map<std::string, TJSfunction> func_container;

class JSManager : public CefV8Handler
{
public:
    JSManager();
    void Initialize(CefRefPtr<CefBrowser> browser, CefRefPtr<CefV8Context> context);

    // handle javascript calls
    virtual bool Execute(const CefString& name,
        CefRefPtr<CefV8Value> object,
        const CefV8ValueList& arguments,
        CefRefPtr<CefV8Value>& retval,
        CefString& exception) OVERRIDE;


private:
    static func_container functions;


    IMPLEMENT_REFCOUNTING(JSManager);
};

#endif
