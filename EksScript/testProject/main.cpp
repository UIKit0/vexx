#include "v8.h"
#include "QMetaType"
#include "QHash"

using namespace v8;

class SomeClass
{
public:

  int getNonStatic()
  {
    return nonStatic;
  }

  void setNonStatic(const int &inA)
  {
    nonStatic = inA;
  }

  static int getA()
  {
    return a;
  }

  static void setA(const int &inA)
  {
    a = inA;
  }

  int nonStatic;
  static int a;
};

int SomeClass::a = 0;

template <typename T> Handle<Value> Pack(T t);
template <typename T> T Unpack(Handle<Value>);

template <> Handle<Value> Pack(int i)
{
  return Integer::New(i);
}

template <> int Unpack(Handle<Value> v)
{
  return v->Int32Value();
}

struct WrappedStaticProperty
{
  void *get;
  void *set;

  template <typename T>
  static Handle<Value> StaticGetter(Local<String>, const AccessorInfo& info)
  {
    Local<Value> v = info.Data();
    WrappedStaticProperty *p = (WrappedStaticProperty *)External::Unwrap(v);

    typedef T (*GET)() ;
    GET g = (GET)p->get;

    T val = g();

    return Pack<T>(val);
  }

  template <typename T>
  static void StaticSetter(Local<String>, Local<Value> value, const AccessorInfo& info)
  {
    Local<Value> v = info.Data();
    WrappedStaticProperty *p = (WrappedStaticProperty *)External::Unwrap(v);

    typedef void (*SET)(const T &);
    SET g = (SET)p->set;

    g(Unpack<T>(value));
  }
};

struct WrappedProperty
{
  void *get[2];
  void *set[2];

  template <typename CLASS, typename T>
  static Handle<Value> GetHelper(Local<String>, const AccessorInfo& info)
  {
    Local<Value> v = info.Data();
    WrappedProperty *p = (WrappedProperty *)External::Unwrap(v);

    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    typedef T (CLASS::*GET)();
    GET g = *(GET*)p->get;

    T val = (static_cast<CLASS*>(ptr)->*g)();
    return Pack<T>(val);
  }

  template <typename CLASS, typename T>
  static void SetHelper(Local<String>, Local<Value> value, const AccessorInfo& info)
  {
    Local<Value> v = info.Data();
    WrappedProperty *p = (WrappedProperty *)External::Unwrap(v);

    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();

    typedef void (CLASS::*SET)(const T &);
    SET s = *(SET*)p->set;

    (static_cast<CLASS*>(ptr)->*s)(Unpack<T>(value));
  }
};

class TypeWrapper
{
public:
  void *staticGet;
  void *staticSet;

  template <typename T> static void wrapType()
  {
    v8::AccessorGetter sG = WrappedStaticProperty::StaticGetter<T>;
    v8::AccessorSetter sS = WrappedStaticProperty::StaticSetter<T>;
    TypeWrapper t = { (void*)sG, (void*)sS };
    _wrappedTypes.insert(QMetaTypeId2<int>::qt_metatype_id(), t);
      
  }
  static const TypeWrapper findType(int i)
  {
    _ASSERT(_wrappedTypes.contains(i));
    return _wrappedTypes[i];
  }

  static QHash<int, TypeWrapper> _wrappedTypes;
};
QHash<int, TypeWrapper> TypeWrapper::_wrappedTypes;

void addStaticAccessor(Handle<ObjectTemplate>& tmpl, const char *name, int id, void *get, void *set)
{
  WrappedStaticProperty* wrappedData = new WrappedStaticProperty;
  wrappedData->get = get;
  wrappedData->set = set;

  Handle<Value> data = External::Wrap(wrappedData);

  const TypeWrapper wrap = TypeWrapper::findType(id);
  tmpl->SetAccessor(String::New(name), (v8::AccessorGetter)wrap.staticGet, (v8::AccessorSetter)wrap.staticSet, data);
}

void addAccessor(Handle<ObjectTemplate>& tmpl, const char *name, int id, void *get, void *set, void *gMem[2], void *sMem[2])
{
  WrappedProperty* wrappedData = new WrappedProperty;
  memcpy(wrappedData->get, gMem, sizeof(void*)*2);
  memcpy(wrappedData->set, sMem, sizeof(void*)*2);

  Handle<Value> data = External::Wrap(wrappedData);

  tmpl->SetAccessor(String::New(name), (v8::AccessorGetter)get, (v8::AccessorSetter)set, data);
}

template <typename T, class CLASS, typename GETFUNC, typename SETFUNC>
void addAccessor(Handle<ObjectTemplate>& tmpl, const char *name, GETFUNC g,  SETFUNC s)
{
  _ASSERT(sizeof(g) <= (sizeof(void *)*2));
  _ASSERT(sizeof(s) <= (sizeof(void *)*2));
  v8::AccessorGetter getNoClass = WrappedProperty::GetHelper<CLASS, T>;
  v8::AccessorSetter setNoClass = WrappedProperty::SetHelper<CLASS, T>;

  void *gV[2];
  void *sV[2];
  memcpy(gV, &g, sizeof(void*)*2);
  memcpy(sV, &s, sizeof(void*)*2);

  addAccessor(tmpl, name, 0, getNoClass, setNoClass, gV, sV);
}

Handle<Value> GetPointX(Local<String>,
                        const AccessorInfo &info)
{
  Local<Object> self = info.Holder();
  Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
  void* ptr = wrap->Value();
  int value = static_cast<SomeClass*>(ptr)->getNonStatic();
  return Integer::New(value);
}

void SetPointX(Local<String>, Local<Value> value,
               const AccessorInfo& info)
{
  Local<Object> self = info.Holder();
  Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
  void* ptr = wrap->Value();
  static_cast<SomeClass*>(ptr)->setNonStatic(value->Int32Value());
}

int main(int, char*[])
{
  // Create a stack-allocated handle scope.
  HandleScope handle_scope;

  Handle<ObjectTemplate> global_templ = ObjectTemplate::New();

  Handle<ObjectTemplate> point_templ = ObjectTemplate::New();
  point_templ->SetAccessor(String::New("x"), GetPointX, SetPointX);
  point_templ->SetInternalFieldCount(1);

  TypeWrapper::wrapType<int>();
  addStaticAccessor(point_templ, "a", QMetaTypeId2<int>::qt_metatype_id(), SomeClass::getA, SomeClass::setA);
  addAccessor<int, SomeClass>(point_templ, "x", &SomeClass::getNonStatic, &SomeClass::setNonStatic);

  // Create a new context.
  Persistent<Context> context = Context::New(NULL, global_templ);

  // Enter the created context for compiling and
  // running the hello world script.
  Context::Scope context_scope(context);

  SomeClass c;
  Local<Object> obj = point_templ->NewInstance();
  obj->SetInternalField(0, External::New(&c));

  Handle<String> propName = String::New("someClass");
  context->Global()->Set(propName, obj);

  // Create a string containing the JavaScript source code.
  Handle<String> source = String::New("someClass.a = 5; someClass.x = 1; 'Hello' + someClass.a.toString() + someClass.x.toString();");

  // Compile the source code.
  Handle<Script> script = Script::Compile(source);

  TryCatch trycatch;
  // Run the script to get the result.
  Handle<Value> result = script->Run();

  if (result.IsEmpty()) {  
    Handle<Value> exception = trycatch.Exception();
    String::AsciiValue exception_str(exception);
    printf("Exception: %s\n", *exception_str);
    // ...
  }

  // Dispose the persistent context.
  context.Dispose();

  // Convert the result to an ASCII string and print it.
  String::AsciiValue ascii(result);
  printf("%s\n", *ascii);
  return 0;
}
