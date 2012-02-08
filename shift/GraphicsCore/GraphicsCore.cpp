#include "GraphicsCore.h"
#include "styperegistry.h"

#include "GCImage.h"

#include "3D/GCGeometry.h"
#include "3D/GCScreenRenderTarget.h"
#include "3D/GCTextureRenderTarget.h"
#include "3D/GCTextureRenderTarget.h"

#include "3D/Renderable/GCCuboid.h"
#include "3D/Renderable/GCSphere.h"
#include "3D/Renderable/GCPlane.h"

#include "3D/GCCamera.h"
#include "3D/GCTransform.h"
#include "3D/GCRenderable.h"
#include "3D/GCScene.h"

#include "3D/GCViewport.h"

#include "3D/GCShader.h"
#include "3D/GCShaderDataBindings.h"
#include "3D/GCShadingGroup.h"

#include "3D/GCTexture.h"

#include "3D/Shaders/GCStandardSurface.h"

#include "3D/Manipulators/GCManipulator.h"
#include "3D/Manipulators/GCDistanceManipulator.h"
#include "3D/Manipulators/GCTranslateManipulator.h"

void initiateGraphicsCore()
  {
  STypeRegistry::addType(GCImage::staticTypeInformation());
  STypeRegistry::addType(GCQImage::staticTypeInformation());
  STypeRegistry::addType(TransformProperty::staticTypeInformation());
  STypeRegistry::addType(ComplexTransformProperty::staticTypeInformation());
  STypeRegistry::addType(GCRuntimeGeometry::staticTypeInformation());
  STypeRegistry::addType(GCRuntimeShader::staticTypeInformation());


  STypeRegistry::addType(GCGeometryAttribute::staticTypeInformation());
  STypeRegistry::addType(GCGeometry::staticTypeInformation());

  STypeRegistry::addType(GCTransform::staticTypeInformation());
  STypeRegistry::addType(GCGeometryTransform::staticTypeInformation());

  STypeRegistry::addType(GCRenderable::staticTypeInformation());

  STypeRegistry::addType(GCTexture::staticTypeInformation());


  STypeRegistry::addType(GCViewableTransform::staticTypeInformation());
  STypeRegistry::addType(GCCamera::staticTypeInformation());
  STypeRegistry::addType(GCPerspectiveCamera::staticTypeInformation());


  STypeRegistry::addType(GCRenderTarget::staticTypeInformation());
  STypeRegistry::addType(GCScreenRenderTarget::staticTypeInformation());
  STypeRegistry::addType(GCTextureRenderTarget::staticTypeInformation());
  STypeRegistry::addType(GCViewport::staticTypeInformation());

  STypeRegistry::addType(GCScene::staticTypeInformation());
  STypeRegistry::addType(GCManipulatableScene::staticTypeInformation());

  STypeRegistry::addType(GCShader::staticTypeInformation());

  STypeRegistry::addType(GCShaderComponent::staticTypeInformation());
  STypeRegistry::addType(GCFragmentShaderComponent::staticTypeInformation());
  STypeRegistry::addType(GCVertexShaderComponent::staticTypeInformation());
  STypeRegistry::addType(GCShadingGroup::staticTypeInformation());

  STypeRegistry::addType(GCCuboid::staticTypeInformation());
  STypeRegistry::addType(GCSphere::staticTypeInformation());
  STypeRegistry::addType(GCPlane::staticTypeInformation());

  STypeRegistry::addType(GCStandardSurface::staticTypeInformation());

  STypeRegistry::addType(GCVisualManipulator::staticTypeInformation());
  STypeRegistry::addType(GCVisualCompoundManipulator::staticTypeInformation());
  STypeRegistry::addType(GCVisualDragManipulator::staticTypeInformation());
  STypeRegistry::addType(GCLinearDragManipulator::staticTypeInformation());
  STypeRegistry::addType(GCDistanceManipulator::staticTypeInformation());
  STypeRegistry::addType(GCSingularTranslateManipulator::staticTypeInformation());
  STypeRegistry::addType(GCTranslateManipulator::staticTypeInformation());

  ColourProperty::staticTypeInformation()->addStaticInterface(new GCShaderDataBindings::Vector4);
  GCTexturePointer::staticTypeInformation()->addStaticInterface(new GCShaderDataBindings::TextureRef);
  }
