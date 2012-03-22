#include "XScriptEngine.h"
#include "XScriptValue.h"
#include "XInterface.h"
#include "XConvertToScript.h"
#include "XScriptValueV8Internals.h"
#include "v8.h"

struct StaticEngine
  {
  StaticEngine() : globalTemplate(v8::ObjectTemplate::New()),
      context(v8::Context::New(NULL, globalTemplate)),
      contextScope(context)
    {
    context->AllowCodeGenerationFromStrings(false);
    }

  v8::HandleScope scope;
  v8::Handle<v8::ObjectTemplate> globalTemplate;
  v8::Persistent<v8::Context> context;
  v8::Context::Scope contextScope;
  };

StaticEngine *g_engine = 0;

void fatal(const char* location, const char* message)
  {
  qFatal("%s: %s", location, message);
  }

void XScriptEngine::initiate()
  {
  if(g_engine)
    {
    return;
    }

  v8::V8::SetFatalErrorHandler(fatal);

  g_engine = new StaticEngine();
  }

void XScriptEngine::terminate()
  {
  g_engine->context.Dispose();
  delete g_engine;
  }

XScriptEngine::XScriptEngine()
  {
  }

XScriptEngine::~XScriptEngine()
  {
  v8::V8::LowMemoryNotification();
  }

void XScriptEngine::set(const QString& in, const XScriptObject& obj)
  {
  XScriptValue propName = XScriptConvert::to(in);
  g_engine->context->Global()->Set(getV8Internal(propName), getV8Internal(obj));
  }

void XScriptEngine::set(const QString &name, Function fn)
  {
  XScriptValue propName = XScriptConvert::to(name);
  v8::Handle<v8::FunctionTemplate> fnTmpl = ::v8::FunctionTemplate::New((v8::InvocationCallback)fn);

  g_engine->context->Global()->Set(getV8Internal(propName), fnTmpl->GetFunction());
  }


void XScriptEngine::addInterface(const XInterfaceBase *i)
  {
  xAssert(i->isSealed());
  i->addClassTo(i->typeName(), fromHandle(g_engine->context->Global()));
  }


void XScriptEngine::adjustAmountOfExternalAllocatedMemory(int in)
  {
  v8::V8::AdjustAmountOfExternalAllocatedMemory(in);
  }

v8::Handle<v8::ObjectTemplate> getGlobalTemplate(XScriptEngine *e)
  {
  return g_engine->globalTemplate;
  }
