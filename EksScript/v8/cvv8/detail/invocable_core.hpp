#if !defined(CODE_GOOGLE_COM_V8_CONVERT_INVOCABLE_V8_HPP_INCLUDED)
#define CODE_GOOGLE_COM_V8_CONVERT_INVOCABLE_V8_HPP_INCLUDED 1

/** @file invocable_core.hpp

This file houses the core-most "invocation-related" parts of the
v8-convert API. These types and functions are for converting
free and member functions to v8::InvocationCallback functions
so that they can be tied to JS objects. It relies on the
CastToJS() and CastFromJS() functions to do non-function type
conversion.

*/

#include "convert_core.hpp"
#include "XSignature.h"
#include "XSignatureSpecialisations.h"
#include "XScriptException.h"

namespace cvv8 {

/**
    A concept class, primarily for documentation and tag-type purposes.
*/
struct InCa
{
    /**
        Matches the v8::InvocationCallback interface. All function bindings
        created by this framework use Call() as their class-level
        InvocationCallback interface. Some bindings provide additional
        overloads as well.
    */
    static v8::Handle<v8::Value> Call( v8::Arguments const & );
};

/**
    A tag type for use with FunctionTo and MethodTo.
*/
struct InCaVoid : InCa {};


/**
   Partial specialization for v8::InvocationCallback-like functions
   (differing only in their return type) with an Arity value of -1.
*/
template <typename RV>
struct XFunctionSignature< RV (v8::Arguments const &) > : XSignature<RV (v8::Arguments const &)>
{
    typedef RV (*FunctionType)(v8::Arguments const &);
};

/**
   Partial specialization for v8::InvocationCallback-like non-const
   member functions (differing only in their return type) with an
   Arity value of -1.
*/
template <typename T, typename RV >
struct XMethodSignature< T, RV (v8::Arguments const &) > : XSignature< RV (v8::Arguments const &) >
{
    typedef T Type;
    typedef RV (T::*FunctionType)(v8::Arguments const &);
};

/**
   Partial specialization for v8::InvocationCallback-like const member
   functions (differing only in their return type) with an Arity value
   of -1.
*/
template <typename T, typename RV >
struct XConstMethodSignature< T, RV (v8::Arguments const &) > : XSignature< RV (v8::Arguments const &) >
{
    typedef T Type;
    typedef RV (T::*FunctionType)(v8::Arguments const &) const;
};

/**
   Full specialization for InvocationCallback and
   InvocationCallback-like functions (differing only by their return
   type), which uses an Arity value of -1.
*/
template <typename RV, RV (*FuncPtr)(v8::Arguments const &) >
struct XFunctionPtr<RV (v8::Arguments const &),FuncPtr>
    : XFunctionSignature<RV (v8::Arguments const &)>
{
    public:
    typedef XFunctionSignature<RV (v8::Arguments const &)> XSignatureType;
    typedef typename XSignatureType::ReturnType ReturnType;
    typedef typename XSignatureType::FunctionType FunctionType;
    static FunctionType GetFunction()
     {
         return FuncPtr;
     }
};

/**
   Full specialization for InvocationCallback and
   InvocationCallback-like functions (differing only by their return
   type), which uses an Arity value of -1.
*/
template <typename T,typename RV, RV (T::*FuncPtr)(v8::Arguments const &) >
struct XMethodPtr<T, RV (T::*)(v8::Arguments const &),FuncPtr>
    : XMethodSignature<T, RV (T::*)(v8::Arguments const &)>
{
    typedef XMethodSignature<T, RV (T::*)(v8::Arguments const &)> XSignatureType;
    typedef typename XSignatureType::ReturnType ReturnType;
    typedef typename XSignatureType::FunctionType FunctionType;
    static FunctionType GetFunction()
     {
         return FuncPtr;
     }
};

/**
   Full specialization for InvocationCallback and
   InvocationCallback-like functions (differing only by their return
   type), which uses an Arity value of -1.
*/
template <typename T,typename RV, RV (T::*FuncPtr)(v8::Arguments const &) const >
struct XConstMethodPtr<T, RV (T::*)(v8::Arguments const &) const,FuncPtr>
    : XConstMethodSignature<T, RV (T::*)(v8::Arguments const &) const>
{
    typedef XConstMethodSignature<T, RV (T::*)(v8::Arguments const &) const> XSignatureType;
    typedef typename XSignatureType::ReturnType ReturnType;
    typedef typename XSignatureType::FunctionType FunctionType;
    static FunctionType GetFunction()
     {
         return FuncPtr;
     }
};

#if !defined(DOXYGEN)
namespace Detail {
    /**
        A sentry class which instantiates a v8::Unlocker
        if the boolean value is true or is a no-op if it is false.
    */
    template <bool> struct V8Unlocker {};

    /**
        Equivalent to v8::Unlocker.
    */
    template <>
    struct V8Unlocker<true> : v8::Unlocker
    {
    };


}
#endif

/**
    A metatemplate which we can use to determine if a given type
    is "safe" to use in conjunction with v8::Unlock.

    We optimistically assume that most types are safe and
    add specializations for types we know are not safe, e.g.
    v8::Handle<Anything> and v8::Arguments.

    Specializations of this type basically make up a collective
    "blacklist" of types which we know are not legal to use unless
    v8 is locked by our current thread. As new problematic types are
    discovered, they can be added to the blacklist by introducing a
    specialization of this class.
*/
template <typename T>
struct IsUnlockable : XBoolVal<true> {};
template <typename T>
struct IsUnlockable<T const> : IsUnlockable<T> {};
template <typename T>
struct IsUnlockable<T const &> : IsUnlockable<T> {};
template <typename T>
struct IsUnlockable<T *> : IsUnlockable<T> {};
template <typename T>
struct IsUnlockable<T const *> : IsUnlockable<T> {};

/**
    Explicit instantiation to make damned sure that nobody
    re-sets this one. We rely on these semantics for
    handling XFunctionSignature like Const/XMethodSignature
    in some cases.
*/
template <>
struct IsUnlockable<void> : XBoolVal<true> {};

/*
    Todo?: find a mechanism to cause certain highly illegal types to
    always trigger an assertion... We could probably do it by
    declaring but not definiting JSToNative/NativeToJS
    specialializations.
*/
template <>
struct IsUnlockable<void *> : XBoolVal<false> {};

template <>
struct IsUnlockable<void const *> : XBoolVal<false> {};

template <typename T>
struct IsUnlockable< v8::Handle<T> > : XBoolVal<false> {};

template <typename T>
struct IsUnlockable< v8::Persistent<T> > : XBoolVal<false> {};

template <typename T>
struct IsUnlockable< v8::Local<T> > : XBoolVal<false> {};

template <>
struct IsUnlockable<v8::Arguments> : XBoolVal<false> {};

/**
    Given a tmp::TypeList-compatible type list, this metafunction's
    Value member evalues to true if IsUnlockable<T>::Value is
    true for all types in TList, else Value evaluates to false.
    As a special case, an empty list evaluates to true because in this
    API an empty list will refer to a function taking no arguments.
*/
template <typename TList>
struct TypeListIsUnlockable : XBoolVal<
    IsUnlockable<typename TList::Head>::Value && TypeListIsUnlockable<typename TList::Tail>::Value
    >
{
};

//! End-of-typelist specialization.
template <>
struct TypeListIsUnlockable< XNilType > : XBoolVal<true>
{};
/**
    Given a XSignature, this metafunction's Value member
    evaluates to true if:

    - IsUnlockable<SigTList::ReturnType>::Value is true and...

    - IsUnlockable<SigTList::Context>::Value is true (only relavent
    for Const/XMethodSignature, not XFunctionSignature) and...

    - TypeListIsUnlockable< SigTList >::Value is true.

    If the value is true, the function XSignature has passed the most
    basic check for whether or not it is legal to use v8::Unlocker
    to unlock v8 before calling a function with this XSignature. Note
    that this is a best guess, but this code cannot know
    app-specific conditions which might make this guess invalid.
    e.g. if a bound function itself uses v8 and does not explicitly
    acquire a lock then the results are undefined (and will very
    possibly trigger an assertion in v8, killing the app). Such
    things can and do happen. If you're lucky, you're building
    against a debug version of libv8 and its assertion text will
    point you directly to the JS code which caused the assertion,
    then you can disable unlocking support for that binding.

    If you want to disable unlocking for all bound members of a given
    class, create an IsUnlockable<T> specialization whos Value
    member evaluates to false. Then no functions/methods using that
    class will cause this metafunction to evaluate to true.

    Note that FunctionToInCa, Const/MethodToInCa, etc., are all
    XSignature subclasses, and can be used directly with
    this template.

    Example:

    @code
    // This one can be unlocked:
    typedef XFunctionSignature< int (int) > F1;
    // XSignatureIsUnlockable<F1>::Value == true

    // This one cannot because it contains a v8 type in the arguments:
    typedef XFunctionSignature< int (v8::Handle<v8::Value>) > F2;
    // XSignatureIsUnlockable<F2>::Value == false
    @endcode

    For Const/MethodToInCa types, this check will also fail if
    IsUnlockable< SigTList::Context >::Value is false.
*/
template <typename SigTList>
struct XSignatureIsUnlockable : XBoolVal<
        IsUnlockable<typename SigTList::Context>::Value &&
        IsUnlockable<typename SigTList::ReturnType>::Value &&
        IsUnlockable<typename SigTList::Head>::Value &&
        XSignatureIsUnlockable<typename SigTList::Tail>::Value
        > {};
//! End-of-list specialization.
template <>
struct XSignatureIsUnlockable<XNilType> : XBoolVal<true> {};



/**
    This special-case convenience form of Toss() triggers a JS-side
    exception containing an English-language message explaining that
    CastFromJS<T>() failed, i.e. the native 'this' pointer for a bound
    native T object could not be found. It uses TypeName<T>::Value as the
    class' name.

    This is primarily intended for use in two cases:

    - Internally in the cvv8 binding mechanisms.

    - In client code when binding non-member functions as JS-side methods of
    native objects.

    Returns the result of v8::ThrowException(...).

    Example:

    @code
    v8::Handle<v8::Value> my_bound_func( v8::Arguments const & argv ) {
        T * self = CastFromJS<T>( argv.This() );
        if( ! self ) {
            return TossMissingThis<T>();
        }
        ...
    }
    @endcode

    BUG: TypeName<T>::Value is NOT used here because... If we instantiate
    TypeName<> from here then we require a value for TypeName<>::Value or we
    break FunctorTo and friends (because TypeName'ing functors isn't always
    possible). The problem with that is that i want to use TypeName::Value's
    address as a type key and that can't work cross-DLL on Windows (and
    possibly other platforms) if we have a default value for TypeName::Value.
    i hope to eventually find a solution which is both cross-platform and
    allows us to invoke TypeName<T>::Value from here.
*/
template <typename T>
v8::Handle<v8::Value> TossMissingThis()
{
    return Toss(StringBuffer()<<"CastFromJS<"
#if 0
        <<TypeName<T>::Value
#else
        <<'T'
#endif
        <<">() returned NULL! Cannot find 'this' pointer!");
}

#if !defined(DOXYGEN)
namespace Detail {
/** Temporary internal macro. Undef'd at the end of this file. */
#define HANDLE_PROPAGATE_EXCEPTION catch( MissingThisException const & ){ return TossMissingThis<T>(); } \
            catch(...){ throw; } (void)0/* (void)0 is a hack to help emacs' indentation out!*/

    /**
        A MissingThisException type holding generic
        message text which references TypeName<T>::Value
        as being the problematic class.
    */
    template <typename T>
    struct MissingThisExceptionT : MissingThisException
    {
        MissingThisExceptionT()
        {
            this->init<T>();
        }
    };

/**
    Temporary internal macro to trigger a static assertion if unlocking
    support is requested but cannot be implemented for the given
    wrapper due to constraints on our use of v8 when it is unlocked.

    This differs from ASSERT_UNLOCK_SANITY_CHECK in that this is intended
    for use with functions which we _know_ (while coding, as opposed to
    finding out at compile time) are not legally unlockable.
*/
#define ASSERT_UNLOCKV8_IS_FALSE typedef char ThisSpecializationCannotUseV8Unlocker[!UnlockV8 ? 1 : -1]
/**
    Temporary internal macro to trigger a static assertion if:
    UnlockV8 is true and XSignatureIsUnlockable<Sig>::Value is false.
    This is to prohibit that someone accidentally enables locking
    when using a function type which we know (based on its
    XSignature's types) cannot obey the (un)locking rules.
*/
#define ASSERT_UNLOCK_SANITY_CHECK typedef char AssertCanEnableUnlock[ \
    !UnlockV8 ? 1 : (XSignatureIsUnlockable< XSignatureType >::Value ?  1 : -1) \
    ]

    template <int Arity_, typename Sig,
            bool UnlockV8 = XSignatureIsUnlockable< XSignature<Sig> >::Value >
    struct FunctionForwarder;

    template <int Arity, typename RV, bool UnlockV8>
    struct FunctionForwarder<Arity,RV (v8::Arguments const &), UnlockV8>
        : XFunctionSignature<RV (v8::Arguments const &)>
    {
    public:
        typedef XFunctionSignature<RV (v8::Arguments const &)> XSignatureType;
        typedef typename XSignatureType::FunctionType FunctionType;
        typedef typename XSignatureType::ReturnType ReturnType;
        static ReturnType CallNative( FunctionType func, v8::Arguments const & argv )
        {
            return (RV) func(argv);
        }

        static v8::Handle<v8::Value> Call( FunctionType func, v8::Arguments const & argv )
        {
            return CastToJS( CallNative( func, argv ) );
        }

        ASSERT_UNLOCKV8_IS_FALSE;
        typedef char AssertArity[ sl::Arity<XSignatureType>::Value == -1 ? 1 : -1];
    };

    //! Reminder to self: we really do need this specialization for some cases.
    template <int Arity, typename RV, bool UnlockV8>
    struct FunctionForwarder<Arity,RV (*)(v8::Arguments const &), UnlockV8>
        : FunctionForwarder<Arity,RV (v8::Arguments const &), UnlockV8>
    {};

    template <typename Sig, bool UnlockV8>
    struct FunctionForwarder<0,Sig, UnlockV8> : XFunctionSignature<Sig>
    {
        typedef XFunctionSignature<Sig> XSignatureType;
        typedef typename XSignatureType::ReturnType ReturnType;
        typedef typename XSignatureType::FunctionType FunctionType;
        static ReturnType CallNative( FunctionType func, v8::Arguments const & )
        {
            V8Unlocker<UnlockV8> const & unlocker( V8Unlocker<UnlockV8>() );
            return func();
        }
        static v8::Handle<v8::Value> Call( FunctionType func, v8::Arguments const & argv )
        {
            return CastToJS( CallNative( func, argv ) );
        }
        ASSERT_UNLOCK_SANITY_CHECK;
        typedef char AssertArity[ sl::Arity<XSignatureType>::Value == 0 ? 1 : -1];
    };

    template <int Arity_, typename Sig,
                bool UnlockV8 = XSignatureIsUnlockable< XSignature<Sig> >::Value>
    struct FunctionForwarderVoid;

    template <typename Sig, bool UnlockV8>
    struct FunctionForwarderVoid<0,Sig, UnlockV8> : XFunctionSignature<Sig>
    {
        typedef XFunctionSignature<Sig> XSignatureType;
        typedef typename XSignatureType::ReturnType ReturnType;
        typedef Sig FunctionType;
        static ReturnType CallNative( FunctionType func, v8::Arguments const & )
        {
            V8Unlocker<UnlockV8> const & unlocker( V8Unlocker<UnlockV8>() );
            return (ReturnType)func()
            /* the explicit cast there is a workaround for the RV==void
               case. It is a no-op for other cases, since the return value
               is already RV. Some compilers (MSVC) don't allow an explicit
               return of a void expression without the cast.
            */;
        }
        static v8::Handle<v8::Value> Call( FunctionType func, v8::Arguments const & argv )
        {
            CallNative( func, argv );
            return v8::Undefined();
        }
        ASSERT_UNLOCK_SANITY_CHECK;
        typedef char AssertArity[ sl::Arity<XSignatureType>::Value == 0 ? 1 : -1];
    };

    template <int Arity, typename RV, bool UnlockV8>
    struct FunctionForwarderVoid<Arity,RV (v8::Arguments const &), UnlockV8>
        : XFunctionSignature<RV (v8::Arguments const &)>
    {
    public:
        typedef XFunctionSignature<RV (v8::Arguments const &)> XSignatureType;
        typedef typename XSignatureType::FunctionType FunctionType;
        typedef typename XSignatureType::ReturnType ReturnType;
        static ReturnType CallNative( FunctionType func, v8::Arguments const & argv )
        {
            V8Unlocker<UnlockV8> const & unlocker( V8Unlocker<UnlockV8>() );
            return (ReturnType)func(argv);
        }
        static v8::Handle<v8::Value> Call( FunctionType func, v8::Arguments const & argv )
        {
            CallNative( func, argv );
            return v8::Undefined();
        }
        ASSERT_UNLOCKV8_IS_FALSE;
        typedef char AssertArity[ sl::Arity<XSignatureType>::Value == -1 ? 1 : -1];
    };

    /**
        Internal impl for cvv8::XConstMethodForwarder.
    */
    template <typename T, int Arity_, typename Sig,
             bool UnlockV8 = XSignatureIsUnlockable< XMethodSignature<T, Sig> >::Value
     >
    struct XMethodForwarder;


    template <typename T, typename Sig, bool UnlockV8>
    struct XMethodForwarder<T, 0, Sig, UnlockV8> : XMethodSignature<T,Sig>
    {
    public:
        typedef XMethodSignature<T,Sig> XSignatureType;
        typedef typename XSignatureType::ReturnType ReturnType;
        typedef typename XSignatureType::FunctionType FunctionType;
        typedef typename TypeInfo<T>::Type Type;
        static ReturnType CallNative( T & self, FunctionType func, v8::Arguments const & argv )
        {
            V8Unlocker<UnlockV8> const & unlocker( V8Unlocker<UnlockV8>() );
            return (self.*func)();
        }
        static v8::Handle<v8::Value> Call( T & self, FunctionType func, v8::Arguments const & argv )
        {
            try
            {
                return CastToJS( CallNative( self, func, argv ) );
            }
            HANDLE_PROPAGATE_EXCEPTION;
        }
        static ReturnType CallNative( FunctionType func, v8::Arguments const & argv )
        {
            T * self = CastFromJS<T>(argv.This());
            if( ! self ) throw MissingThisExceptionT<T>();
            return (ReturnType)CallNative(*self, func, argv);
        }
        static v8::Handle<v8::Value> Call( FunctionType func, v8::Arguments const & argv )
        {
            try
            {
                return CastToJS( CallNative( func, argv ) );
            }
            HANDLE_PROPAGATE_EXCEPTION;
        }

        ASSERT_UNLOCK_SANITY_CHECK;
    };

    template <typename T, int Arity, typename RV, bool UnlockV8>
    struct XMethodForwarder<T, Arity, RV (v8::Arguments const &), UnlockV8>
        : XMethodSignature<T, RV (v8::Arguments const &)>
    {
    public:
        typedef XMethodSignature<T, RV (v8::Arguments const &)> XSignatureType;
        typedef char AssertArity[ sl::Arity<XSignatureType>::Value == -1 ? 1 : -1];
        typedef typename XSignatureType::FunctionType FunctionType;
        typedef typename XSignatureType::ReturnType ReturnType;
        static ReturnType CallNative( T & self, FunctionType func, v8::Arguments const & argv )
        {
            return (self.*func)(argv);
        }
        static v8::Handle<v8::Value> Call( T & self, FunctionType func, v8::Arguments const & argv )
        {
            try
            {
                return CastToJS( CallNative( self, func, argv ) );
            }
            HANDLE_PROPAGATE_EXCEPTION;
        }
        static ReturnType CallNative( FunctionType func, v8::Arguments const & argv )
        {
            T * self = CastFromJS<T>(argv.This());
            if( ! self ) throw MissingThisExceptionT<T>();
            return (ReturnType)CallNative(*self, func, argv);
        }
        static v8::Handle<v8::Value> Call( FunctionType func, v8::Arguments const & argv )
        {
            try
            {
                return CastToJS( CallNative(func, argv) );
            }
            HANDLE_PROPAGATE_EXCEPTION;
        }
        ASSERT_UNLOCKV8_IS_FALSE;
    };

    template <typename T, typename RV, bool UnlockV8, int _Arity>
    struct XMethodForwarder<T,_Arity, RV (T::*)(v8::Arguments const &), UnlockV8> :
            XMethodForwarder<T, _Arity, RV (v8::Arguments const &), UnlockV8>
    {};

    template <typename T, int Arity_, typename Sig,
        bool UnlockV8 = XSignatureIsUnlockable< XMethodSignature<T, Sig> >::Value
    >
    struct XMethodForwarderVoid ;

    template <typename T, typename Sig, bool UnlockV8>
    struct XMethodForwarderVoid<T,0,Sig, UnlockV8>
        : XMethodSignature<T,Sig>
    {
    public:
        typedef XMethodSignature<T,Sig> XSignatureType;
        typedef typename XSignatureType::FunctionType FunctionType;
        typedef T Type;
        typedef typename XSignatureType::ReturnType ReturnType;
        static ReturnType CallNative( Type & self, FunctionType func, v8::Arguments const & )
        {
            V8Unlocker<UnlockV8> const & unlocker( V8Unlocker<UnlockV8>() );
            return (ReturnType)(self.*func)();
        }
        static v8::Handle<v8::Value> Call( Type & self, FunctionType func, v8::Arguments const & argv )
        {
            try
            {
                CallNative( self, func, argv );
                return v8::Undefined();
            }
            HANDLE_PROPAGATE_EXCEPTION;
        }
        static ReturnType CallNative( FunctionType func, v8::Arguments const & argv )
        {
            T * self = CastFromJS<T>(argv.This());
            if( ! self ) throw MissingThisExceptionT<T>();
            return (ReturnType)CallNative(*self, func, argv);
        }
        static v8::Handle<v8::Value> Call( FunctionType func, v8::Arguments const & argv )
        {
            try
            {
                CallNative(func, argv);
                return v8::Undefined();
            }
            HANDLE_PROPAGATE_EXCEPTION;
        }
    };

    template <typename T, int Arity, typename RV, bool UnlockV8>
    struct XMethodForwarderVoid<T,Arity, RV (v8::Arguments const &), UnlockV8>
        : XMethodSignature<T,RV (v8::Arguments const &)>
    {
    public:
        typedef XMethodSignature<T,RV (v8::Arguments const &)> XSignatureType;
        typedef typename XSignatureType::FunctionType FunctionType;
        typedef typename TypeInfo<T>::Type Type;
        typedef typename XSignatureType::ReturnType ReturnType;
        static ReturnType CallNative( Type & self, FunctionType func, v8::Arguments const & argv )
        {
            return (ReturnType)(self.*func)(argv);
        }
        static v8::Handle<v8::Value> Call( Type & self, FunctionType func, v8::Arguments const & argv )
        {
            try
            {
                CallNative( self, func, argv );
                return v8::Undefined();
            }
            HANDLE_PROPAGATE_EXCEPTION;

        }
        static ReturnType CallNative( FunctionType func, v8::Arguments const & argv )
        {
            T * self = CastFromJS<T>(argv.This());
            if( ! self ) throw MissingThisExceptionT<T>();
            return (ReturnType)CallNative(*self, func, argv);
        }
        static v8::Handle<v8::Value> Call( FunctionType func, v8::Arguments const & argv )
        {
            try
            {
                CallNative(func, argv);
                return v8::Undefined();
            }
            HANDLE_PROPAGATE_EXCEPTION;
        }
        ASSERT_UNLOCKV8_IS_FALSE;
    };

    template <typename T, int Arity, typename RV, bool UnlockV8>
    struct XMethodForwarderVoid<T,Arity, RV (T::*)(v8::Arguments const &), UnlockV8>
        : XMethodForwarderVoid<T,Arity, RV (v8::Arguments const &), UnlockV8>
    {};

    /**
        Internal impl for cvv8::XConstMethodForwarder.
    */
    template <typename T, int Arity_, typename Sig,
            bool UnlockV8 = XSignatureIsUnlockable< XConstMethodSignature<T, Sig> >::Value
    >
    struct XConstMethodForwarder ;

    template <typename T, typename Sig, bool UnlockV8>
    struct XConstMethodForwarder<T,0,Sig, UnlockV8> : XConstMethodSignature<T,Sig>
    {
    public:
        typedef XConstMethodSignature<T,Sig> XSignatureType;
        typedef typename XSignatureType::FunctionType FunctionType;
        typedef typename XSignatureType::ReturnType ReturnType;
        static ReturnType CallNative( T const & self, FunctionType func, v8::Arguments const & )
        {
            V8Unlocker<UnlockV8> const & unlocker( V8Unlocker<UnlockV8>() );
            return (self.*func)();
        }

        static v8::Handle<v8::Value> Call( T const & self, FunctionType func, v8::Arguments const & argv )
        {
            try
            {
                return CastToJS( CallNative( self, func, argv ) );
            }
            HANDLE_PROPAGATE_EXCEPTION;
        }
        static ReturnType CallNative( FunctionType func, v8::Arguments const & argv )
        {
            T const * self = CastFromJS<T>(argv.This());
            if( ! self ) throw MissingThisExceptionT<T>();
            return (ReturnType)CallNative(*self, func, argv);
        }
        static v8::Handle<v8::Value> Call( FunctionType func, v8::Arguments const & argv )
        {
            try
            {
                return CastToJS( CallNative(func, argv) );
            }
            HANDLE_PROPAGATE_EXCEPTION;
        }
    };

    template <typename T, int Arity, typename RV, bool UnlockV8>
    struct XConstMethodForwarder<T, Arity, RV (v8::Arguments const &), UnlockV8>
        : XConstMethodSignature<T, RV (v8::Arguments const &)>
    {
    public:
        typedef XConstMethodSignature<T, RV (v8::Arguments const &)> XSignatureType;
        typedef char AssertArity[ sl::Arity<XSignatureType>::Value == -1 ? 1 : -1];
        typedef typename XSignatureType::FunctionType FunctionType;
        typedef typename XSignatureType::ReturnType ReturnType;
        static ReturnType CallNative( T const & self, FunctionType func, v8::Arguments const & argv )
        {
            return (ReturnType)(self.*func)(argv);
        }
        static v8::Handle<v8::Value> Call( T const & self, FunctionType func, v8::Arguments const & argv )
        {
            try
            {
                return CastToJS( CallNative( self, func, argv ) );
            }
            HANDLE_PROPAGATE_EXCEPTION;
        }
        static ReturnType CallNative( FunctionType func, v8::Arguments const & argv )
        {
            T const * self = CastFromJS<T>(argv.This());
            if( ! self ) throw MissingThisExceptionT<T>();
            return (ReturnType)CallNative(*self, func, argv);
        }
        static v8::Handle<v8::Value> Call( FunctionType func, v8::Arguments const & argv )
        {
            try
            {
                return CastToJS( CallNative(func, argv) );
            }
            HANDLE_PROPAGATE_EXCEPTION;
        }
        ASSERT_UNLOCKV8_IS_FALSE;
    };

    template <typename T, int Arity, typename RV, bool UnlockV8>
    struct XConstMethodForwarder<T, Arity, RV (T::*)(v8::Arguments const &) const, UnlockV8>
        : XConstMethodForwarder<T, Arity, RV (v8::Arguments const &), UnlockV8>
    {};

    template <typename T, int Arity_, typename Sig,
            bool UnlockV8 = XSignatureIsUnlockable< XConstMethodSignature<T, Sig> >::Value
    >
    struct XConstMethodForwarderVoid;

    template <typename T, typename Sig, bool UnlockV8>
    struct XConstMethodForwarderVoid<T,0,Sig, UnlockV8> : XConstMethodSignature<T,Sig>
    {
    public:
        typedef XConstMethodSignature<T,Sig> XSignatureType;
        typedef typename XSignatureType::FunctionType FunctionType;
        typedef typename XSignatureType::ReturnType ReturnType;
        typedef typename TypeInfo<T>::Type Type;
        static ReturnType CallNative( Type const & self, FunctionType func, v8::Arguments const & )
        {
            V8Unlocker<UnlockV8> const & unlocker( V8Unlocker<UnlockV8>() );
            return (ReturnType)(self.*func)();
        }

        static v8::Handle<v8::Value> Call( Type const & self, FunctionType func, v8::Arguments const & argv )
        {
            try
            {
                CallNative( self, func, argv );
                return v8::Undefined();
            }
            HANDLE_PROPAGATE_EXCEPTION;
        }
        static ReturnType CallNative( FunctionType func, v8::Arguments const & argv )
        {
            T const * self = CastFromJS<T>(argv.This());
            if( ! self ) throw MissingThisExceptionT<T>();
            return (ReturnType)CallNative(*self, func, argv);
        }
        static v8::Handle<v8::Value> Call( FunctionType func, v8::Arguments const & argv )
        {
            try
            {
                CallNative(func, argv);
                return v8::Undefined();
            }
            HANDLE_PROPAGATE_EXCEPTION;
        }
        ASSERT_UNLOCK_SANITY_CHECK;
    };

    template <typename T, int Arity, typename RV, bool UnlockV8>
    struct XConstMethodForwarderVoid<T, Arity, RV (v8::Arguments const &), UnlockV8>
        : XConstMethodSignature<T, RV (v8::Arguments const &)>
    {
    public:
        typedef XConstMethodSignature<T, RV (v8::Arguments const &)> XSignatureType;
        typedef char AssertArity[ sl::Arity<XSignatureType>::Value == -1 ? 1 : -1];
        typedef typename XSignatureType::FunctionType FunctionType;
        typedef typename XSignatureType::ReturnType ReturnType;
        typedef typename TypeInfo<T>::Type Type;
        static ReturnType CallNative( Type const & self, FunctionType func, v8::Arguments const & argv )
        {
            V8Unlocker<UnlockV8> const & unlocker( V8Unlocker<UnlockV8>() );
            return (ReturnType)(self.*func)();
        }
        static v8::Handle<v8::Value> Call( Type const & self, FunctionType func, v8::Arguments const & argv )
        {
            try
            {
                CallNative( self, func, argv );
            }
            HANDLE_PROPAGATE_EXCEPTION;
        }
        static ReturnType CallNative( FunctionType func, v8::Arguments const & argv )
        {
            T const * self = CastFromJS<T>(argv.This());
            if( ! self ) throw MissingThisExceptionT<T>();
            return CallNative(*self, func, argv);
        }
        static v8::Handle<v8::Value> Call( FunctionType func, v8::Arguments const & argv )
        {
            try
            {
                return CastToJS( CallNative( func, argv ) );
            }
            HANDLE_PROPAGATE_EXCEPTION;
        }
        ASSERT_UNLOCKV8_IS_FALSE;
    };

    template <typename T, int Arity, typename RV, bool UnlockV8>
    struct XConstMethodForwarderVoid<T, Arity, RV (T::*)(v8::Arguments const &) const, UnlockV8>
    : XConstMethodForwarderVoid<T, Arity, RV (v8::Arguments const &), UnlockV8>
    {};

}
#endif // DOXYGEN

/**
    A helper type for passing v8::Arguments lists to native non-member
    functions.

    Sig must be a function-signature-like argument. e.g. <double (int,double)>,
    and the members of this class expect functions matching that XSignature.

    If UnlockV8 is true then v8::Unlocker will be used to unlock v8
    for the duration of the call to Func(). HOWEVER... (the rest of
    these docs are about the caveats)...

    A UnlockV8 value of XSignatureIsUnlockable<Sig>::Value uses a
    reasonably sound heuristic but it cannot predict certain
    app-dependent conditions which render its guess semantically
    invalid. See XSignatureIsUnlockable for more information.

    It is illegal for UnlockV8 to be true if ANY of the following
    applies:

    - The callback itself will "use" v8 but does not explicitly use
    v8::Locker. If it uses v8::Locker then it may safely enable
    unlocking support. We cannot determine via templates  whether
    or not a function "uses" v8 except by guessing based on the
    return and arguments types.

    - Any of the return or argument types for the function are v8
    types, e.g. v8::Handle<Anything> or v8::Arguments.
    XSignatureIsUnlockable<Sig>::Value will evaluate to false
    if any of the "known bad" types is contained in the function's
    XSignature. If this function is given a true value but
    XSignatureIsUnlockable<Sig>::Value is false then
    a compile-time assertion will be triggered.

    - Certain callback XSignatures cannot have unlocking support
    enabled because if we unlock then we cannot legally access the data
    we need to convert. These will cause a compile-time assertion
    if UnlockV8 is true. All such bits are incidentally covered by
    XSignatureIsUnlockable's check, so this assertion can
    only happen if the client explicitly sets UnlockV8 to true for
    those few cases.

    - There might be other, as yet unknown/undocumented, corner cases
    where unlocking v8 is semantically illegal at this level of the
    API.

    Violating any of these leads to undefined behaviour. The library
    tries to fail at compile time for invalid combinations when it
    can, but (as described aboved) it can be fooled into thinking that
    unlocking is safe.

    Reminder to self: we can implement this completely via inheritance of
    the internal Proxy type, but i REALLY want the API docs out here at this
    level instead of in the Detail namespace (which i filter out of doxygen
    for the most part).
*/
template <typename Sig,
        bool UnlockV8 = XSignatureIsUnlockable< XSignature<Sig> >::Value
>
struct FunctionForwarder
{
private:
    typedef typename XIfElse< XSameType<void ,typename XSignature<Sig>::ReturnType>::Value,
                                Detail::FunctionForwarderVoid< sl::Arity< XSignature<Sig> >::Value, Sig, UnlockV8 >,
                                Detail::FunctionForwarder< sl::Arity< XSignature<Sig> >::Value, Sig, UnlockV8 >
        >::Type
    Proxy;
public:
    typedef typename Proxy::SignatureType XSignatureType;
    typedef typename Proxy::ReturnType ReturnType;
    typedef typename Proxy::FunctionType FunctionType;
    /**
       Passes the given arguments to func(), converting them to the appropriate
       types. If argv.Length() is less than sl::Arity< XSignatureType >::Value then
       a JS exception is thrown, with one exception: if the function has "-1 arity"
       (i.e. it is InvocationCallback-like) then argv is passed on to it regardless
       of the value of argv.Length().

       The native return value of the call is returned to the caller.
    */
    static ReturnType CallNative( FunctionType func, v8::Arguments const & argv )
    {
        return Proxy::CallNative( func, argv );
    }
    /**
        Equivalent to CastToJS( CallNative(func,argv) ) unless ReturnType
        is void, in which case CastToJS() is not invoked and v8::Undefined()
        will be returned.
    */
    static v8::Handle<v8::Value> Call( FunctionType func, v8::Arguments const & argv )
    {
        return Proxy::Call( func, argv );
    }
};

/**
    CallForwarder basically does the opposite of FunctionForwarder: it
    converts native arguments to JS and calls a JS function.

    The default implementation is useless - it must be specialized
    for each arity.
*/
template <int Arity>
struct CallForwarder
{
    /**
        Implementations must be templates taking Arity arguments in addition
        to the first two. All argument types must legal for use with
        CastToJS().

        If either self or func.IsEmpty() then a JS exception must be thrown,
        else implementations must return func->Call(self, N, ARGS), where
        ARGS is an array of v8::Handle<v8::Value> and N is the number of
        items in that array (and, not coincidentally, is the same value as
        Arity). The ARGS array must be populated by calling CastToJS(ARG_N)
        for each argument, where ARG_N is the Nth argument.
    */
    static v8::Handle<v8::Value> Call( v8::Handle<v8::Object> const & self,
                                       v8::Handle<v8::Function> const & func,
                                       ... );
    /**
        Convenience form of Call() which must be equivalent to
        Call(func,func,...).
    */
    static v8::Handle<v8::Value> Call( v8::Handle<v8::Function> const & func, ... );
};

//! Specialization for 0-arity calls.
template <>
struct CallForwarder<0>
{
    static v8::Handle<v8::Value> Call( v8::Handle<v8::Object> const & self,
                                       v8::Handle<v8::Function> const & func )
    {
        return (self.IsEmpty() || func.IsEmpty())
            ? Toss("Illegal argument: empty v8::Handle<>.")
            : func->Call(self, 0, NULL);
    }
    static v8::Handle<v8::Value> Call( v8::Handle<v8::Function> const & func )
    {
        return Call( func, func );
    }
};


#if !defined(DOXYGEN)
namespace Detail {

    /**
        Base internal implementation of cvv8::FunctionToInCa.
    */
    template <typename Sig,
              typename XFunctionSignature<Sig>::FunctionType Func,
              bool UnlockV8 = XSignatureIsUnlockable< XFunctionSignature<Sig> >::Value >
    struct FunctionToInCa : XFunctionPtr<Sig,Func>, InCa
    {

        typedef XFunctionSignature<Sig> XSignatureType;
        typedef FunctionForwarder< sl::Arity<XSignatureType>::Value, Sig, UnlockV8 > Proxy;
        typedef typename XSignatureType::ReturnType ReturnType;
        static ReturnType CallNative( v8::Arguments const & argv )
        {
            return (ReturnType)Proxy::CallNative( Func, argv );
            /**
                The (ReturnType) cast is effectively a no-op for all types
                except void, where it is used to get around a limitation in
                some compilers which does not allow us to explicitly return
                foo() when foo() returns void.
            */
        }
        static v8::Handle<v8::Value> Call( v8::Arguments const & argv )
        {
            return Proxy::Call( Func, argv );
        }
        ASSERT_UNLOCK_SANITY_CHECK;
    };

    /** Almost identical to FunctionToInCa, but expressly does not
        instantiate NativeToJS< XSignature<Sig>::ReturnType >, meaning
        that:

        a) it can proxy functions which have non-convertible return types.

        b) JS always gets the 'undefined' JS value as the return value.
    */
    template <typename Sig,
              typename XFunctionSignature<Sig>::FunctionType Func,
              bool UnlockV8 = XSignatureIsUnlockable< XFunctionSignature<Sig> >::Value >
    struct FunctionToInCaVoid : XFunctionPtr<Sig,Func>, InCa
    {
        typedef XFunctionSignature<Sig> XSignatureType;
        typedef FunctionForwarderVoid< sl::Arity<XSignatureType>::Value, Sig, UnlockV8 > Proxy;
        typedef typename XSignatureType::ReturnType ReturnType;
        static ReturnType CallNative( v8::Arguments const & argv )
        {
            return (ReturnType)Proxy::CallNative( Func, argv );
        }
        static v8::Handle<v8::Value> Call( v8::Arguments const & argv )
        {
            return Proxy::Call( Func, argv );
        }
        ASSERT_UNLOCK_SANITY_CHECK;
    };

    /** Method equivalent to FunctionToInCa. */
    template <typename T,
              typename Sig,
              typename XMethodSignature<T,Sig>::FunctionType Func,
              bool UnlockV8 = XSignatureIsUnlockable< XMethodSignature<T,Sig> >::Value
              >
    struct MethodToInCa : XMethodPtr<T,Sig, Func>, InCa
    {
        typedef XMethodPtr<T, Sig, Func> XSignatureType;
        enum { Arity = sl::Arity<XSignatureType>::Value };
        typedef XMethodForwarder< T, Arity, Sig, UnlockV8 > Proxy;
        typedef typename XSignatureType::ReturnType ReturnType;
        static ReturnType CallNative( T & self, v8::Arguments const & argv )
        {
            return Proxy::CallNative( self, Func, argv );
        }
        static ReturnType CallNative( v8::Arguments const & argv )
        {
            return Proxy::Call( Func, argv );
        }
        static v8::Handle<v8::Value> Call( v8::Arguments const & argv )
        {
            return Proxy::Call( Func, argv );
        }
        static v8::Handle<v8::Value> Call( T & self, v8::Arguments const & argv )
        {
            return Proxy::Call( self, Func, argv );
        }
        ASSERT_UNLOCK_SANITY_CHECK;
    };

    /** Method equivalent to FunctionToInCaVoid. */
    template <typename T,
              typename Sig,
              typename XMethodSignature<T,Sig>::FunctionType Func,
              bool UnlockV8 = XSignatureIsUnlockable< XMethodSignature<T,Sig> >::Value
              >
    struct MethodToInCaVoid : XMethodPtr<T,Sig,Func>, InCa
    {
        typedef XMethodPtr<T, Sig, Func> XSignatureType;
        enum { Arity = sl::Arity<XSignatureType>::Value };
        typedef XMethodForwarderVoid< T, Arity, Sig, UnlockV8 > Proxy;
        typedef typename XSignatureType::ReturnType ReturnType;
        static ReturnType CallNative( T & self, v8::Arguments const & argv )
        {
            return Proxy::CallNative( self, Func, argv );
        }
        static ReturnType CallNative( v8::Arguments const & argv )
        {
            return Proxy::Call( Func, argv );
        }
        static v8::Handle<v8::Value> Call( v8::Arguments const & argv )
        {
            return Proxy::Call( Func, argv );
        }
        static v8::Handle<v8::Value> Call( T & self, v8::Arguments const & argv )
        {
            return Proxy::Call( self, Func, argv );
        }
        ASSERT_UNLOCK_SANITY_CHECK;
    };

    /** Const method equivalent to MethodToInCa. */
    template <typename T,
              typename Sig,
              typename XConstMethodSignature<T,Sig>::FunctionType Func,
              bool UnlockV8 = XSignatureIsUnlockable< XConstMethodSignature<T,Sig> >::Value
              >
    struct ConstMethodToInCa : XConstMethodPtr<T,Sig, Func>, InCa
    {
        typedef XConstMethodPtr<T, Sig, Func> XSignatureType;
        typedef XConstMethodForwarder< T, sl::Arity<XSignatureType>::Value, Sig, UnlockV8 > Proxy;
        typedef typename XSignatureType::ReturnType ReturnType;
        static ReturnType CallNative( T const & self, v8::Arguments const & argv )
        {
            return Proxy::CallNative( self, Func, argv );
        }
        static ReturnType CallNative( v8::Arguments const & argv )
        {
            return Proxy::Call( Func, argv );
        }
        static v8::Handle<v8::Value> Call( v8::Arguments const & argv )
        {
            return Proxy::Call( Func, argv );
        }
        static v8::Handle<v8::Value> Call( T const & self, v8::Arguments const & argv )
        {
            return Proxy::Call( self, Func, argv );
        }
    };

    template <typename T,
              typename Sig,
              typename XConstMethodSignature<T,Sig>::FunctionType Func,
              bool UnlockV8 = XSignatureIsUnlockable< XConstMethodSignature<T,Sig> >::Value
              >
    struct ConstMethodToInCaVoid : XConstMethodPtr<T,Sig,Func>, InCa
    {
        typedef XConstMethodPtr<T, Sig, Func> XSignatureType;
        typedef XConstMethodForwarderVoid< T, sl::Arity<XSignatureType>::Value, Sig, UnlockV8 > Proxy;
        typedef typename XSignatureType::ReturnType ReturnType;
        static ReturnType CallNative( T const & self, v8::Arguments const & argv )
        {
            return Proxy::CallNative( self, Func, argv );
        }
        static ReturnType CallNative( v8::Arguments const & argv )
        {
            return Proxy::Call( Func, argv );
        }
        static v8::Handle<v8::Value> Call( v8::Arguments const & argv )
        {
            return Proxy::Call( Func, argv );
        }
        static v8::Handle<v8::Value> Call( T const & self, v8::Arguments const & argv )
        {
            return Proxy::Call( self, Func, argv );
        }
        ASSERT_UNLOCK_SANITY_CHECK;
    };

} // Detail namespace
#endif // DOXYGEN
#undef ASSERT_UNLOCK_SANITY_CHECK
#undef ASSERT_UNLOCKV8_IS_FALSE


/**
   A template for converting free (non-member) function pointers
   to v8::InvocationCallback.

   Sig must be a function XSignature. Func must be a pointer to a function
   with that XSignature.

   If UnlockV8 is true then v8::Unlocker will be used to unlock v8
   for the duration of the call to Func(). HOWEVER... see
   FunctionForwarder for the details/caveats regarding that
   parameter.

   Example:

    @code
    v8::InvocationCallback cb =
        FunctionToInCa< int (int), ::putchar>::Call;
    @endcode
*/
template <typename Sig,
          typename XFunctionSignature<Sig>::FunctionType Func,
          bool UnlockV8 = XSignatureIsUnlockable< XSignature<Sig> >::Value
          >
struct FunctionToInCa
    : XIfElse< XSameType<void ,typename XFunctionSignature<Sig>::ReturnType>::Value,
                 Detail::FunctionToInCaVoid< Sig, Func, UnlockV8>,
                 Detail::FunctionToInCa< Sig, Func, UnlockV8>
        >::Type
{};

/**
    A variant of FunctionToInCa with the property of NOT invoking the
    conversion of the function's return type from native to JS form. i.e. it
    does not cause NativeToJS< XSignature<Sig>::ReturnType > to be
    instantiated. This is useful when such a conversion is not legal because
    CastToJS() won't work on it or, more generally, when you want the JS
    interface to always get the undefined return value.

    Call() always returns v8::Undefined(). CallNative(), however,
    returns the real return type specified by Sig (which may be void).

    Example:

    @code
    v8::InvocationCallback cb =
       FunctionToInCaVoid< int (int), ::putchar>::Call;
    @endcode
*/
template <typename Sig,
          typename XFunctionSignature<Sig>::FunctionType Func,
          bool UnlockV8 = XSignatureIsUnlockable< XSignature<Sig> >::Value
          >
struct FunctionToInCaVoid : Detail::FunctionToInCaVoid< Sig, Func, UnlockV8>
{};


/**
   A template for converting non-const member function pointers to
   v8::InvocationCallback. If T is const qualified then this works
   as for ConstMethodToInCaVoid.

   To convert JS objects to native 'this' pointers this API uses
   CastFromJS<T>(arguments.This()), where arguments is the
   v8::Arguments instance passed to the generated InvocationCallback
   implementation. If that conversion fails then the generated
   functions will throw a JS-side exception when called.

   T must be some client-specified type which is presumably bound (or
   at least bindable) to JS-side Objects. Sig must be a member
   function XSignature for T. Func must be a pointer to a function with
   that XSignature.

   See FunctionForwarder for details about the UnlockV8 parameter.

    Example:

    @code
    v8::InvocationCallback cb =
        MethodToInCa<T, int (int), &T::myFunc>::Call;
    // For const methods:
    cb = MethodToInCa<const T, int (int), &T::myFunc>::Call;
    @endcode
*/
template <typename T, typename Sig,
          typename XMethodSignature<T,Sig>::FunctionType Func,
          bool UnlockV8 = XSignatureIsUnlockable< XMethodSignature<T,Sig> >::Value
          >
struct MethodToInCa
    : XIfElse< XSameType<void ,typename XMethodSignature<T,Sig>::ReturnType>::Value,
                 Detail::MethodToInCaVoid<T, Sig, Func, UnlockV8>,
                 Detail::MethodToInCa<T, Sig, Func, UnlockV8>
        >::Type
{
};

/**
    See FunctionToInCaVoid - this is identical exception that it
    works on member functions of T. If T is const qualified
    then this works as for ConstMethodToInCaVoid.

    Example:

    @code
    v8::InvocationCallback cb =
       MethodToInCaVoid< T, int (int), &T::myFunc>::Call;
    @endcode

*/
template <typename T, typename Sig,
          typename XMethodSignature<T,Sig>::FunctionType Func,
          bool UnlockV8 = XSignatureIsUnlockable< XMethodSignature<T,Sig> >::Value
          >
struct MethodToInCaVoid
    : Detail::MethodToInCaVoid<T, Sig, Func, UnlockV8>
{
};

#if !defined(DOXYGEN)
namespace Detail {
    template <typename RV, typename Ftor, typename Sig, bool UnlockV8>
    struct FunctorToInCaSelector : XConstMethodForwarder<Ftor, sl::Arity<XSignature<Sig> >::Value, Sig, UnlockV8>
    {
    };
    template <typename Ftor, typename Sig, bool UnlockV8>
    struct FunctorToInCaSelector<void,Ftor,Sig,UnlockV8> : XConstMethodForwarderVoid<Ftor, sl::Arity<XSignature<Sig> >::Value, Sig, UnlockV8>
    {};
}
#endif

/**
    This class converts a functor to an InvocationCallback. Ftor must
    be a functor type. Sig must be a XSignature unambiguously matching
    a Ftor::operator() implementation and Ftor::operator() must be
    const. UnlockV8 is as described for FunctionToInCa.
*/
template <typename Ftor, typename Sig,
        bool UnlockV8 = XSignatureIsUnlockable< XMethodSignature<Ftor,Sig> >::Value
        >
struct FunctorToInCa : InCa
{
    inline static v8::Handle<v8::Value> Call( v8::Arguments const & argv )
    {
        typedef Detail::FunctorToInCaSelector<
            typename XSignature<Sig>::ReturnType, Ftor, Sig, UnlockV8
        > Proxy;
        return Proxy::Call( Ftor(), &Ftor::operator(), argv );
    }
};

/**
    The functor equivalent of FunctionToInCaVoid. See FunctorToInCa for
    the requirements of the Ftor and Sig types.
*/
template <typename Ftor, typename Sig,
        bool UnlockV8 = XSignatureIsUnlockable< XMethodSignature<Ftor,Sig> >::Value
        >
struct FunctorToInCaVoid : InCa
{
    inline static v8::Handle<v8::Value> Call( v8::Arguments const & argv )
    {
        typedef Detail::FunctorToInCaSelector<
            void, Ftor, Sig, UnlockV8
        > Proxy;
        return Proxy::Call( Ftor(), &Ftor::operator(), argv );
    }
};

/**
   Functionally identical to MethodToInCa, but for const member functions.

   See FunctionForwarder for details about the UnlockV8 parameter.

   Note that the Sig XSignature must be suffixed with a const qualifier!

    Example:

    @code
    v8::InvocationCallback cb =
       ConstMethodToInCa< T, int (int), &T::myFunc>::Call;
    @endcode

*/
template <typename T, typename Sig, typename XConstMethodSignature<T,Sig>::FunctionType Func,
          bool UnlockV8 = XSignatureIsUnlockable< XConstMethodSignature<T,Sig> >::Value
          >
struct ConstMethodToInCa
#if 0 // not working due to an ambiguous Sig<T,...> vs Sig<const T,...>. Kinda weird, since MethodToInCa<const T,...> works.
    : MethodToInCa<const T,Sig,Func,UnlockV8> {};
#else
    : XIfElse< XSameType<void ,typename XConstMethodSignature<T, Sig>::ReturnType>::Value,
                 Detail::ConstMethodToInCaVoid<T, Sig, Func, UnlockV8>,
                 Detail::ConstMethodToInCa<T, Sig, Func, UnlockV8>
        >::Type
{};
#endif
/**
    See FunctionToInCaVoid - this is identical exception that it
    works on const member functions of T.

    Note that the Sig XSignature must be suffixed with a const qualifier!

    Example:

    @code
    v8::InvocationCallback cb =
       ConstMethodToInCaVoid< T, int (int), &T::myFunc>::Call;
    @endcode

*/
template <typename T, typename Sig, typename XConstMethodSignature<T,Sig>::FunctionType Func,
          bool UnlockV8 = XSignatureIsUnlockable< XConstMethodSignature<T,Sig> >::Value
          >
struct ConstMethodToInCaVoid : Detail::ConstMethodToInCaVoid<T, Sig, Func, UnlockV8>
{};

/**
   Identicial to FunctionForwarder, but works on non-const
   member methods of type T.

   Sig must be a function-signature-like argument. e.g. <double
   (int,double)>, and the members of this class expect T member
   functions matching that XSignature.
*/
template <typename T, typename Sig, bool UnlockV8 = XSignatureIsUnlockable< XMethodSignature<T,Sig> >::Value>
struct XMethodForwarder
{
private:
    typedef typename
    XIfElse< XSameType<void ,typename XMethodSignature<T,Sig>::ReturnType>::Value,
                 Detail::XMethodForwarderVoid< T, sl::Arity< XSignature<Sig> >::Value, Sig, UnlockV8 >,
                 Detail::XMethodForwarder< T, sl::Arity< XSignature<Sig> >::Value, Sig, UnlockV8 >
    >::Type
    Proxy;
public:
    typedef typename Proxy::SignatureType XSignatureType;
    typedef typename Proxy::FunctionType FunctionType;
    typedef typename Proxy::ReturnType ReturnType;
    /**
       Passes the given arguments to (self.*func)(), converting them
       to the appropriate types. If argv.Length() is less than
       sl::Arity<XSignatureType>::Value then a JS exception is
       thrown, with one exception: if the function has "-1 arity"
       (i.e. it is InvocationCallback-like) then argv is passed on
       to it regardless of the value of argv.Length().
    */
    inline static v8::Handle<v8::Value> Call( T & self, FunctionType func, v8::Arguments const & argv )
    {
        return Proxy::Call( self, func, argv );
    }

    /**
       Like the 3-arg overload, but tries to extract the (T*) object using
       CastFromJS<T>(argv.This()).
    */
    inline static v8::Handle<v8::Value> Call( FunctionType func, v8::Arguments const & argv )
    {
        return Proxy::Call( func, argv );
    }
};


/**
   Identical to XMethodForwarder, but works on const member methods.
*/
template <typename T, typename Sig,
        bool UnlockV8 = XSignatureIsUnlockable< XConstMethodSignature<T,Sig> >::Value
        >
struct XConstMethodForwarder
#if 1 //?msvc Seems to work.
: XMethodForwarder<T const, Sig, UnlockV8> {};
#else
{
private:
    typedef typename
    XIfElse< XSameType<void ,typename XConstMethodSignature<T,Sig>::ReturnType>::Value,
                 Detail::XConstMethodForwarderVoid< T, sl::Arity< XSignature<Sig> >::Value, Sig, UnlockV8 >,
                 Detail::XConstMethodForwarder< T, sl::Arity< XSignature<Sig> >::Value, Sig, UnlockV8 >
    >::Type
    Proxy;
public:
    typedef typename Proxy::SignatureType XSignatureType;
    typedef typename Proxy::FunctionType FunctionType;

    /**
       Passes the given arguments to (self.*func)(), converting them
       to the appropriate types. If argv.Length() is less than
       sl::Arity< XSignature<Sig> >::Value then a JS exception is thrown, with one
       exception: if the function has "-1 arity" (i.e. it is
       InvocationCallback-like) then argv is passed on to it
       regardless of the value of argv.Length().
    */
    inline static v8::Handle<v8::Value> Call( T const & self, FunctionType func, v8::Arguments const & argv )
    {
        return Proxy::Call( self, func, argv );
    }

    /**
       Like the 3-arg overload, but tries to extract the (T const *)
       object using CastFromJS<T>(argv.This()).
    */
    inline static v8::Handle<v8::Value> Call( FunctionType func, v8::Arguments const & argv )
    {
        return Proxy::Call( func, argv );
    }
};
#endif

/**
   Tries to forward the given arguments to the given native
   function. Will fail if argv.Lengt() is not at least
   sl::Arity<XSignature<Sig>>::Value, throwing a JS exception in that
   case _unless_ the function is InvocationCallback-like, in which
   case argv is passed directly to it regardless of the value of
   argv.Length().
*/
template <typename Sig>
inline typename XFunctionSignature<Sig>::ReturnType
forwardFunction( Sig func, v8::Arguments const & argv )
{
    typedef XFunctionSignature<Sig> MSIG;
    typedef typename MSIG::ReturnType RV;
    enum { Arity = sl::Arity< XSignature<Sig> >::Value };
    typedef typename
        XIfElse< XSameType<void ,RV>::Value,
        Detail::FunctionForwarderVoid< Arity, Sig >,
        Detail::FunctionForwarder< Arity, Sig >
        >::Type Proxy;
    return (RV)Proxy::CallNative( func, argv );
}

/**
   Works like forwardFunction(), but forwards to the
   given non-const member function and treats the given object
   as the 'this' pointer.
*/
template <typename T, typename Sig>
inline typename XMethodSignature<T,Sig>::ReturnType
forwardMethod( T & self,
               Sig func,
               /* if i do: typename XMethodSignature<T,Sig>::FunctionType
                  then this template is never selected. */
               v8::Arguments const & argv )
{
    typedef XMethodSignature<T,Sig> MSIG;
    typedef typename MSIG::ReturnType RV;
    enum { Arity = sl::Arity< MSIG >::Value };
    typedef typename
        XIfElse< XSameType<void ,RV>::Value,
                 Detail::XMethodForwarderVoid< T, Arity, Sig >,
                 Detail::XMethodForwarder< T, Arity, Sig >
    >::Type Proxy;
    return (RV)Proxy::CallNative( self, func, argv );
}

/**
   Like the 3-arg forwardMethod() overload, but
   extracts the native T 'this' object from argv.This().

   Note that this function requires that the caller specify
   the T template parameter - it cannot deduce it.
*/
template <typename T, typename Sig>
inline typename XMethodSignature<T,Sig>::ReturnType
forwardMethod(Sig func, v8::Arguments const & argv )
{
    typedef XMethodSignature<T,Sig> MSIG;
    typedef typename MSIG::ReturnType RV;
    enum { Arity = sl::Arity< MSIG >::Value };
    typedef typename
        XIfElse< XSameType<void ,RV>::Value,
                 Detail::XMethodForwarderVoid< T, Arity, Sig >,
                 Detail::XMethodForwarder< T, Arity, Sig >
    >::Type Proxy;
    return (RV)Proxy::CallNative( func, argv );
}


/**
   Works like forwardMethod(), but forwards to the given const member
   function and treats the given object as the 'this' pointer.

*/
template <typename T, typename Sig>
inline typename XConstMethodSignature<T,Sig>::ReturnType
forwardConstMethod( T const & self,
                    //typename XConstMethodSignature<T,Sig>::FunctionType func,
                    Sig func,
                    v8::Arguments const & argv )
{
    typedef XConstMethodSignature<T,Sig> MSIG;
    typedef typename MSIG::ReturnType RV;
    enum { Arity = sl::Arity< MSIG >::Value };
    typedef typename
        XIfElse< XSameType<void ,RV>::Value,
                 Detail::XConstMethodForwarderVoid< T, Arity, Sig >,
                 Detail::XConstMethodForwarder< T, Arity, Sig >
    >::Type Proxy;
    return (RV)Proxy::CallNative( self, func, argv );
}

/**
   Like the 3-arg forwardConstMethod() overload, but
   extracts the native T 'this' object from argv.This().

   Note that this function requires that the caller specify
   the T template parameter - it cannot deduce it.
*/
template <typename T, typename Sig>
inline typename XConstMethodSignature<T,Sig>::ReturnType
forwardConstMethod(Sig func, v8::Arguments const & argv )
{
    typedef XConstMethodSignature<T,Sig> MSIG;
    typedef typename MSIG::ReturnType RV;
    enum { Arity = sl::Arity< MSIG >::Value };
    typedef typename
        XIfElse< XSameType<void ,RV>::Value,
                 Detail::XConstMethodForwarderVoid< T, Arity, Sig >,
                 Detail::XConstMethodForwarder< T, Arity, Sig >
    >::Type Proxy;
    return (RV)Proxy::CallNative( func, argv );
}

/**
   A structified/functorified form of v8::InvocationCallback.  It is
   sometimes convenient to be able to use a typedef to create an alias
   for a given InvocationCallback. Since we cannot typedef function
   templates this way, this class can fill that gap.
*/
template <v8::InvocationCallback ICB>
struct InCaToInCa : FunctionToInCa< v8::Handle<v8::Value> (v8::Arguments const &), ICB>
{
};


#if 0 // YAGNI
/**
   "Converts" an InCaToInCa<ICB> instance to JS by treating it as an
   InvocationCallback function. This is primarily useful in
   conjunction with ClassCreator::Set() to add function bindings.
*/
template <v8::InvocationCallback ICB>
struct NativeToJS< InCaToInCa<ICB> >
{
    /**
       Returns a JS handle to InCaToInCa<ICB>::Call().
    */
    v8::Handle<v8::Value> operator()( InCaToInCa<ICB> const & )
    {
        return v8::FunctionTemplate::New(InCaToInCa<ICB>::Call)->GetFunction();
    }
};
#endif

#if !defined(DOXYGEN)
namespace Detail {
    /** Internal code duplication reducer. */
    template <int ExpectingArity>
    v8::Handle<v8::Value> TossArgCountError( v8::Arguments const & args )
    {
        return Toss((StringBuffer()
                   <<"Incorrect argument count ("<<args.Length()
                   <<") for function - expecting "
                   <<ExpectingArity<<" arguments.").toError());
    }
}
#endif

/**
   A utility template to assist in the creation of InvocationCallbacks
   overloadable based on the number of arguments passed to them at runtime.

   See Call() for more details.

   InCaT and Fallback must be InCa classes.

   Using this class almost always requires more code than doing the
   equivalent with ArityDispatchList, the exception being when we have only
   one or two overloads.

   Note that the Fallback parameter has a default value, and that that
   default value will cause a JS-side exception to be triggered if Call() is
   called without exactly Arity arguments. Binding a single function
   "overload" this way is a simple way to ensure that the function can only
   be called with the specified number of arguments.
*/
template < int Arity,
           typename InCaT,
           typename Fallback = InCaToInCa< Detail::TossArgCountError<Arity> >
>
struct ArityDispatch : InCa
{
    /**
       When called, if (Artity==-1) or if (Arity==args.Length()) then
       InCaT::Call(args) is returned, else Fallback::Call(args) is returned.

       Implements the InvocationCallback interface.
    */
    inline static v8::Handle<v8::Value> Call( v8::Arguments const & args )
    {
        return ( (-1==Arity) || (Arity == args.Length()) )
            ? InCaT::Call(args)
            : Fallback::Call(args);
    }
};


/**
   InvocationCallback wrapper which calls another InvocationCallback
   and translates any native ExceptionT exceptions thrown by that
   function into JS exceptions.

   ExceptionT must be an exception type which is thrown by const
   reference (e.g. STL-style) as opposed to by pointer (MFC-style).

   SigGetMsg must be a function-signature-style argument describing
   a method within ExceptionT which can be used to fetch the message
   reported by the exception. It must meet these requirements:

   a) Be const
   b) Take no arguments
   c) Return a type convertible to JS via CastToJS()

   Getter must be a pointer to a function matching that XSignature.

   InCaT must be a InCa type. When Call() is called by v8, it will pass
   on the call to InCaT::Call() and catch exceptions as described below.

   Exceptions of type (ExceptionT const &) and (ExceptionT const *) which
   are thrown by ICB get translated into a JS exception with an error
   message fetched using ExceptionT::Getter().

   If PropagateOtherExceptions is true then exception of types other
   than ExceptionT are re-thrown (which can be fatal to v8, so be
   careful). If it is false then they are not propagated but the error
   message in the generated JS exception is unspecified (because we
   have no generic way to get such a message). If a client needs to
   catch multiple exception types, enable propagation and chain the
   callbacks together. In such a case, the outer-most (last) callback
   in the chain should not propagate unknown exceptions (to avoid
   killing v8).

   This type can be used to implement "chaining" of exception
   catching, such that we can use the InCaCatcher
   to catch various distinct exception types in the context
   of one v8::InvocationCallback call.

   Example:

   @code
   // Here we want to catch MyExceptionType, std::runtime_error, and
   // std::exception (the base class of std::runtime_error, by the
   // way) separately:

   // When catching within an exception hierarchy, start with the most
   // specific (most-derived) exceptions.

   // Client-specified exception type:
   typedef InCaCatcher<
      MyExceptionType,
      std::string (),
      &MyExceptionType::getMessage,
      InCaToInCa<MyCallbackWhichThrows>, // the "real" InvocationCallback
      true // make sure propagation is turned on so chaining can work!
      > Catch_MyEx;

  // std::runtime_error:
  typedef InCaCatcher_std< Catch_MyEx, std::runtime_error > Catch_RTE;

   // std::exception:
   typedef InCaCatcher_std< Catch_RTE > MyCatcher;

   // Now MyCatcher::Call is-a InvocationCallback which will handle
   // MyExceptionType, std::runtime_error, and std::exception via
   // different catch blocks. Note, however, that the visible
   // behaviour for runtime_error and std::exception (its base class)
   // will be identical here, though they actually have different
   // code.
   @endcode
*/
template < typename ExceptionT,
           typename SigGetMsg,
           typename XConstMethodSignature<ExceptionT,SigGetMsg>::FunctionType Getter,
           typename InCaT,
           bool PropagateOtherExceptions = false
    >
struct InCaCatcher : InCa
{
    /**
        Returns ICB(args), converting any exceptions of type (ExceptionT
        const &) or (ExceptionT const *) to JS exceptions. Other exception
        types are handled as described in the class-level documentation.

        See the class-level docs for full details.
    */
    static v8::Handle<v8::Value> Call( v8::Arguments const & args )
    {
        try
        {
            return InCaT::Call( args );
        }
        catch( ExceptionT const & e2 )
        {
            /* reminder to self:  we now have the unusual corner case that
            if Getter() returns a v8::Handle<v8::Value> it will be thrown
            as-is instead of wrapped in an Error object. See the Toss() docs
            for why that is so. We could use XSameType to alternately
            call TossAsError(), but i'm too tired and i honestly don't ever
            expect any exception type to return v8 handles.
            */
            return Toss((e2.*Getter)());
        }
        catch( ExceptionT const * e2 )
        {
            return Toss((e2->*Getter)());
        }
        catch(...)
        {
            if( PropagateOtherExceptions ) throw;
            else return Toss("Unknown native exception thrown!");
        }
    }
};

/**
   Convenience form of InCaCatcher which catches std::exception objects and
   uses their what() method to fetch the error message.

   The ConcreteException parameter may be std::exception (the default) or
   any publically derived subclass of std::exception. When using a
   non-default value, one can chain exception catchers to catch most-derived
   types first.

   PropagateOtherExceptions determines whether _other_ exception types are
   propagated or not. It defaults to false if ConcreteException is
   std::exception (exactly, not a subtype), else it defaults to true. The
   reasoning is: when chaining these handlers we need to catch the
   most-derived first. Those handlers need to propagate other exceptions so
   that we can catch the lesser-derived ones (or those from completely
   different hierarchies) in subsequent catchers. The final catcher "should"
   (arguably) swallow unknown exceptions, converting them to JS exceptions
   with an unspecified message string (propagating them is technically legal
   but will most likely crash v8).

   Here is an example of chaining:

   @code
    typedef InCaCatcher_std< InCaToInCa<MyThrowingCallback>, std::logic_error > LECatch;
    typedef InCaCatcher_std< LECatch, std::runtime_error > RECatch;
    typedef InCaCatcher_std< RECatch, std::bad_cast > BCCatch;
    typedef InCaCatcher_std< BCCatch > BaseCatch;
    v8::InvocationCallback cb = BaseCatch::Call;
   @endcode

   In the above example any exceptions would be processed in the order:
   logic_error, runtime_error, bad_cast, std::exception, anything else. Notice
   that we took advantage of the PropagateOtherExceptions default value for all
   cases to get the propagation behaviour we want.
*/
template <
        typename InCaT,
        typename ConcreteException = std::exception,
        bool PropagateOtherExceptions = !XSameType< std::exception, ConcreteException >::Value
>
struct InCaCatcher_std :
    InCaCatcher<ConcreteException,
                char const * (),
                &ConcreteException::what,
                InCaT,
                PropagateOtherExceptions
                >
{};

namespace Detail {
    /**
        An internal level of indirection for overloading-related
        dispatchers.
    */
    template <typename InCaT>
    struct OverloadCallHelper : InCa
    {
        inline static v8::Handle<v8::Value> Call( v8::Arguments const & argv )
        {
            return InCaT::Call(argv);
        }
    };
    //! End-of-list specialization.
    template <>
    struct OverloadCallHelper<XNilType> : InCa
    {
        inline static v8::Handle<v8::Value> Call( v8::Arguments const & argv )
        {
          return Toss( QString("End of typelist reached. Argument count=%1").arg(argv.Length()) );
        }
    };
}

/**
   A helper class which allows us to dispatch to multiple
   overloaded native functions from JS, depending on the argument
   count.

   FwdList must be-a TypeList (or interface-compatible type list)
   containing types which have the following function:

   static v8::Handle<v8::Value> Call( v8::Arguments const & argv );

   And a static const integer (or enum) value called Arity,
   which must specify the expected number of arguments, or be
   negative specify that the function accepts any number of
   arguments.

   In other words, all entries in FwdList must implement the
   interface used by most of the InCa-related API.

   Example:

   @code
   // Overload 3 variants of a member function:
   typedef CVV8_TYPELIST((
            MethodToInCa<BoundNative, void (), &BoundNative::overload0>,
            MethodToInCa<BoundNative, void (int), &BoundNative::overload1>,
            MethodToInCa<BoundNative, void (int,int), &BoundNative::overload2>
            // Note that "N-arity" callbacks MUST come last in the list
            // because they will always match any arity count and therefore
            // trump any overloads which follow them.
        )) OverloadList;
   typedef ArityDispatchList< OverloadList > MyOverloads;
   v8::InvocationCallback cb = MyOverloads::Call;
   @endcode

   Note that only one line of that code is evaluated at runtime - the rest
   is all done at compile-time.
*/
template <typename FwdList>
struct ArityDispatchList : InCa
{
    /**
       Tries to dispatch argv to one of the bound functions defined
       in FwdList, based on the number of arguments in argv and
       the Arity

       Implements the v8::InvocationCallback interface.
    */
    inline static v8::Handle<v8::Value> Call( v8::Arguments const & argv )
    {
        typedef typename FwdList::Head FWD;
        typedef typename FwdList::Tail Tail;
        enum { Arity = sl::Arity< FWD >::Value };
        return ( (-1 == Arity) || (Arity == argv.Length()) )
            ? Detail::OverloadCallHelper<FWD>::Call(argv)
            : ArityDispatchList<Tail>::Call(argv);
    }
};

/**
   End-of-list specialization.
*/
template <>
struct ArityDispatchList<XNilType> : Detail::OverloadCallHelper<XNilType>
{
};

#if !defined(DOXYGEN)
namespace Detail {
    //! Internal helper for ToInCa impl.
    template <typename T, typename Sig >
    struct ToInCaSigSelector : XMethodSignature<T,Sig>
    {
        template < typename XMethodSignature<T,Sig>::FunctionType Func, bool UnlockV8 >
        struct Base : cvv8::MethodToInCa<T, Sig, Func, UnlockV8 >
        {
        };
    };

    //! Internal helper for ToInCa impl.
    template <typename Sig>
    struct ToInCaSigSelector<void,Sig> : XFunctionSignature<Sig>
    {
        template < typename XFunctionSignature<Sig>::FunctionType Func, bool UnlockV8 >
        struct Base : cvv8::FunctionToInCa<Sig, Func, UnlockV8 >
        {
        };
    };

    //! Internal helper for ToInCaVoid impl.
    template <typename T, typename Sig >
    struct ToInCaSigSelectorVoid : XMethodSignature<T,Sig>
    {
        template < typename XMethodSignature<T,Sig>::FunctionType Func, bool UnlockV8 >
        struct Base : cvv8::MethodToInCaVoid<T, Sig, Func, UnlockV8 >
        {
        };
    };

    //! Internal helper for ToInCaVoid impl.
    template <typename Sig>
    struct ToInCaSigSelectorVoid<void,Sig> : XFunctionSignature<Sig>
    {
        template < typename XFunctionSignature<Sig>::FunctionType Func, bool UnlockV8 >
        struct Base : cvv8::FunctionToInCaVoid<Sig, Func, UnlockV8 >
        {
        };
    };

}
#endif // DOXYGEN

/**
    A wrapper for MethodToInCa, ConstMethodToInCa, and
    FunctionToInCa, which determines which one of those to use based
    on the type of T and its constness.

    For non-member functions, T must be void. For non-const member
    functions T must be non-cvp-qualified T. For const member functions
    T must be const-qualified.

    See FunctionForwarder for the meaning of the UnlockV8 parameter.

    Examples:

    @code
    typedef ToInCa<MyT, int (int), &MyT::nonConstFunc> NonConstMethod;
    typedef ToInCa<MyT const, void (int), &MyT::constFunc> ConstMethod;
    typedef ToInCa<void, int (char const *), ::puts, true> Func;

    v8::InvocationCallback cb;
    cb = NonConstMethod::Call;
    cb = ConstMethod::Call;
    cb = Func::Call;
    @endcode

    Note the extra 'const' qualification for const method. This is
    neccessary to be able to portably distinguish the constness
    (some compilers allow us to add the const as part of the
    function XSignature). Also note that the 'void' 1st parameter for
    non-member functions is a bit of a hack.
*/
template <typename T, typename Sig,
        typename Detail::ToInCaSigSelector<T,Sig>::FunctionType Func,
        bool UnlockV8 = XSignatureIsUnlockable< Detail::ToInCaSigSelector<T,Sig> >::Value
        >
struct ToInCa : Detail::ToInCaSigSelector<T,Sig>::template Base<Func,UnlockV8>
{
};

/**
    This works just like ToInCa but instead of behaving like
    FunctionToInCa or Const/MethoToInCa it behaves like
    FunctionToInCaVoid or Const/MethoToInCaVoid.
*/
template <typename T, typename Sig,
        typename Detail::ToInCaSigSelectorVoid<T,Sig>::FunctionType Func,
        bool UnlockV8 = XSignatureIsUnlockable< Detail::ToInCaSigSelector<T,Sig> >::Value
        >
struct ToInCaVoid : Detail::ToInCaSigSelectorVoid<T,Sig>::template Base<Func,UnlockV8>
{
};


/**
    A slightly simplified form of FunctionToInCa which is only
    useful for "InvocationCallback-like" functions and requires
    only two arguments:

    @code
    // int my_func( v8::Arguments const & );
    typedef InCaLikeFunction< int, my_func > F;
    @endcode
*/
template <typename RV, RV (*Func)(v8::Arguments const &)>
struct InCaLikeFunction : FunctionToInCa< RV (v8::Arguments const &), Func>
{
};

/**
    A slightly simplified form of MethodToInCa which is only
    useful for non-const "InvocationCallback-like" methods:

    @code
    // Method: int MyType::func( v8::Arguments const & )
    typedef InCaLikeMethod<MyType, int, &MyType::func > F;
    @endcode
*/
template <typename T, typename RV, RV (T::*Func)(v8::Arguments const &)>
struct InCaLikeMethod : MethodToInCa< T, RV (v8::Arguments const &), Func>
{};

/**
    A slightly simplified form of ConstMethodToInCa which is only
    useful for const "InvocationCallback-like" methods:

    @code
    // Method: int MyType::func( v8::Arguments const & ) const
    typedef InCaLikeConstMethod<MyType, int, &MyType::func > F;
    @endcode
*/
template <typename T, typename RV, RV (T::*Func)(v8::Arguments const &) const>
struct InCaLikeConstMethod : ConstMethodToInCa< T, RV (v8::Arguments const &), Func>
{};


#if 0
//! Don't use. Doesn't yet compile. Trying to consolidate Const/MethodXyz
template <typename ASig, typename XSignature<ASig>::FunctionType Func>
struct SigToInCa :
    XIfElse<
        sl::IsFunction< XSignature<ASig> >::Value,
        FunctionToInCa< ASig, Func >,
        typename XIfElse<
            sl::IsConstMethod< XSignature<ASig> >::Value,
            ConstMethodToInCa< typename tmp::PlainType<typename XSignature<ASig>::Context>::Type, ASig, Func >,
            MethodToInCa< typename XSignature<ASig>::Context, ASig, Func >
        >::Type
    >::Type
{};
#endif

/**
    This class acts as a proxy for another InCa-compatible class,
    running client-defined intialization code the _first_ time
    its callback is called from JS. This could be used to run
    library-dependent intialization routines such as lt_dlinit().

    InCaT must conform to the InCa interface (i.e., have a static Call()
    function which implements the v8::InvocationCallback interface).

    InitFunctor must be default-constructable and have an operator()
    (preferably const) taking no args and returning any type which can be
    ignored (i.e. not dynamically-allocated resources).
*/
template <typename InCaT, typename InitFunctor>
struct OneTimeInitInCa : InCa
{
    /**
        The first time this function is called it runs
        InitFunctor()() to execute any client-dependent setup. If
        that throws a native exception it is propagated back to the
        caller and initialization is considered NOT to have
        occurred, meaning the next call to this function will also
        run InitFunctor()().

        If initialization does not throw, InCaT::Call(argv) is
        called and its value is returned. Once initialization
        succeeds, it is _not_ triggered on subsequent calls to this
        function.

        Pedantic note: if this class is used in code which is linked
        in from multiple DLLs, the init routine might be called
        more than once, depending on the platform.
    */
    static v8::Handle<v8::Value> Call( v8::Arguments const & argv )
    {
        static bool bob = false;
        if( ! bob )
        {
            InitFunctor()();
            /* Reminder: if it throws we do not set bob=true.
               This is part of the interface, not an accident.
            */
            bob = true;
        }
        return InCaT::Call( argv );
    }
};


#if 0// i'm not yet decided on these bits...
struct NativeToJS_InCa_Base
{
    typedef v8::InvocationCallback ArgType;
    template <typename SigT>
    inline v8::Handle<v8::Value> operator()( SigT const & ) const
    {
        return v8::FunctionTemplate::New(SigT::Call)->GetFunction();
    }
};

template <typename Sig, typename XFunctionSignature<Sig>::FunctionType Func>
struct NativeToJS< FunctionToInCa<Sig, Func> > : NativeToJS_InCa_Base {};
template <typename Sig, typename XFunctionSignature<Sig>::FunctionType Func>
struct NativeToJS< FunctionToInCaVoid<Sig, Func> > : NativeToJS_InCa_Base {};

template <typename T, typename Sig, typename XMethodSignature<T,Sig>::FunctionType Func>
struct NativeToJS< MethodToInCa<T, Sig, Func> > : NativeToJS_InCa_Base {};
template <typename T, typename Sig, typename XMethodSignature<T,Sig>::FunctionType Func>
struct NativeToJS< MethodToInCaVoid<T, Sig, Func> > : NativeToJS_InCa_Base {};

template <typename T, typename Sig, typename XConstMethodSignature<T,Sig>::FunctionType Func>
struct NativeToJS< ConstMethodToInCa<T, Sig, Func> > : NativeToJS_InCa_Base {};
template <typename T, typename Sig, typename XConstMethodSignature<T,Sig>::FunctionType Func>
struct NativeToJS< ConstMethodToInCaVoid<T, Sig, Func> > : NativeToJS_InCa_Base {};

template <typename T, typename Sig, typename XMethodSignature<T,Sig>::FunctionType Func>
struct NativeToJS< ToInCa<T, Sig, Func> > : NativeToJS_InCa_Base {};
template <typename T, typename Sig, typename XMethodSignature<T,Sig>::FunctionType Func>
struct NativeToJS< ToInCaVoid<T, Sig, Func> > : NativeToJS_InCa_Base {};
#endif

#include "invocable_generated.hpp"
} // namespace

#undef HANDLE_PROPAGATE_EXCEPTION
#undef ENABLE_TOINCA

#endif /* CODE_GOOGLE_COM_V8_CONVERT_INVOCABLE_V8_HPP_INCLUDED */
