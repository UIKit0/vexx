#ifndef GCSHADER_H
#define GCSHADER_H

#include "GCGlobal.h"
#include "sentity.h"
#include "sbaseproperties.h"
#include "sbasepointerproperties.h"
#include "XShader.h"
#include "GCBaseProperties.h"

class GCShaderBindableData : public SStaticInterfaceBase
  {
  S_STATIC_INTERFACE_TYPE(GCShaderBindableData, GCShaderBindableInterface)
public:
  GCShaderBindableData(bool deleteOnNoReferences) : SStaticInterfaceBase(deleteOnNoReferences) { }
  virtual void bindData(XShader *, const SProperty *) const = 0;
  };

class GRAPHICSCORE_EXPORT GCShaderComponent : public StringProperty
  {
  S_ENTITY(GCShaderComponent, StringProperty, 0)
public:
  GCShaderComponent();
  };

S_PROPERTY_INTERFACE(GCShaderComponent)

class GRAPHICSCORE_EXPORT GCFragmentShaderComponent : public GCShaderComponent
  {
  S_ENTITY(GCFragmentShaderComponent, GCShaderComponent, 0)
  };

S_PROPERTY_INTERFACE(GCFragmentShaderComponent)

class GRAPHICSCORE_EXPORT GCVertexShaderComponent : public GCShaderComponent
  {
  S_ENTITY(GCVertexShaderComponent, GCShaderComponent, 0)
  };

S_PROPERTY_INTERFACE(GCVertexShaderComponent)

S_TYPED_POINTER_TYPE(GCShaderComponentPointer, GCShaderComponent);

S_TYPED_POINTER_ARRAY_TYPE(GCShaderComponentPointerArray, GCShaderComponentPointer);

class GRAPHICSCORE_EXPORT GCShader : public SEntity
  {
  S_ENTITY(GCShader, SEntity, 0)
public:
  GCShader();

  void bind(XRenderer *r) const;

  GCRuntimeShader runtimeShader;
  GCShaderComponentPointerArray components;

  static void postChildSet(SPropertyContainer *, SProperty *);

private:
  bool _rebuildShader;
  bool _setVariables;
  static void computeShaderRuntime(const SPropertyInstanceInformation *info, GCShader *cont);
  };

S_PROPERTY_INTERFACE(GCShader)

S_TYPED_POINTER_TYPE(GCShaderPointer, GCShader);

#endif // GCSHADER_H
