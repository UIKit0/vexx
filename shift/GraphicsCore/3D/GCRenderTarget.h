#ifndef GCRENDERTARGET_H
#define GCRENDERTARGET_H

#include "GCGlobal.h"
#include "sentity.h"
#include "GCRenderable.h"

class GRAPHICSCORE_EXPORT GCRenderTarget : public SEntity
  {
  S_ENTITY(GCRenderTarget, SEntity, 0)

public:
  GCRenderTarget();

  GCRenderablePointer source;

  UnsignedIntProperty width;
  UnsignedIntProperty height;

  FloatProperty aspectRatio;

  void render(XRenderer *);
  };

S_PROPERTY_INTERFACE(GCRenderTarget)

#endif // GCRENDERTARGET_H
