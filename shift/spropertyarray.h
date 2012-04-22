#ifndef SPROPERTYARRAY_H
#define SPROPERTYARRAY_H

#include "spropertycontainer.h"
#include "styperegistry.h"

class SHIFT_EXPORT SPropertyArray : public SPropertyContainer
  {
  S_PROPERTY_CONTAINER(SPropertyArray, SPropertyContainer, 0);

public:
  SPropertyArray();
  virtual ~SPropertyArray();

  template <typename T> T *add()
    {
    return addProperty(T::staticTypeInformation(), X_SIZE_SENTINEL)->castTo<T>();
    }

  SProperty *add(const SPropertyInformation *info, const QString &name, SPropertyInstanceInformationInitialiser *init)
    {
    return addProperty(info, X_SIZE_SENTINEL, name, init);
    }

  SProperty *add(const SPropertyInformation *info, const QString &name=QString())
    {
    return addProperty(info, X_SIZE_SENTINEL, name, 0);
    }

  SProperty *operator[](xsize i) { return at(i); }
  using SPropertyContainer::at;
  using SPropertyContainer::clear;

  void remove(SProperty *);
  };

template <typename T> class STypedPropertyArray : public SPropertyContainer
  {
  //S_PROPERTY_CONTAINER(STypedPropertyArray, SPropertyContainer, 0);

public:
  T *add()
    {
    return addProperty(T::staticTypeInformation())->castTo<T>();
    }

  void remove(SProperty *prop)
    {
    removeProperty(prop);
    }

  void resize(xsize s)
    {
    xsize sz = size();
    xsize dif = s - sz;

    if(dif > 0)
      {
      for(xsize x=0; x<s; ++x)
        {
        addProperty(false, internalCreateHelper(T::Type, handler()));
        }
      }
    else if(dif < 0)
      {
      for(xsize x=s; x<sz; ++x)
        {
        // could be optimised lots...
        removeProperty(at(x));
        }
      }
    }

  T *operator[](xsize i) { return at(i); }
  T *at(xsize i)
    {
    SProperty *c = firstChild();
    while(c && i)
      {
      --i;
      c = c->nextSibling();
      }

    return c;
    }

  using SPropertyContainer::clear;
  };

//S_IMPLEMENT_TEMPLATED_PROPERTY(template <typename T>, STypedPropertyArray<T>)

/*template <typename T> SPropertyInformation *STypedPropertyArray<T>::createTypeInformation()
  {
  return SPropertyInformation::create<STypedPropertyArray<T> >("STypedPropertyArray<T>");
  }*/

S_PROPERTY_INTERFACE(SPropertyArray)

//S_PROPERTY_INTERFACE_TYPED(STypedPropertyArray)

#endif // SPROPERTYARRAY_H
