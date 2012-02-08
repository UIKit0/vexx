#include "sashaderparteditorinterface.h"
#include "3D/GCShader.h"
#include "3D/GCCamera.h"
#include "3D/GCViewport.h"
#include "3D/GCScene.h"
#include "3D/Shaders/GCStandardSurface.h"
#include "3D/GCShadingGroup.h"
#include "3D/GCScreenRenderTarget.h"
#include "3D/Renderable/GCCuboid.h"
#include "3D/Renderable/GCSphere.h"
#include "3D/Renderable/GCPlane.h"
#include "saviewport.h"
#include "QVBoxLayout"
#include "QTextEdit"
#include "QLabel"
#include "QFile"
#include "UI/spropertydefaultui.h"
#include "UI/sentityui.h"

class ShaderPreviewViewport : public SViewport
  {
  XProperty(QTextEdit *, log, setLog);
  XROProperty(const GCShader *, shader);

public:
  ShaderPreviewViewport(const SEntity *s, SPropertyArray *holder) : SViewport(holder->add<GCViewport>()), _log(0),
      _shader(s->uncheckedCastTo<GCShader>())
    {
    const GCShader *sha = shader();
    xAssert(sha);

    GCViewport *vp = viewport();

    GCPerspectiveCamera* cam = vp->addChild<GCPerspectiveCamera>("Camera");
    vp->x.connect(&cam->viewportX);
    vp->y.connect(&cam->viewportY);
    vp->width.connect(&cam->viewportWidth);
    vp->height.connect(&cam->viewportHeight);

    cam->setPosition(XVector3D(0.0f, 0.0f, 10.0f));
    cam->setFocalPoint(XVector3D(0.0f, 0.0f, 0.0f));

    GCManipulatableScene* msc = vp->addChild<GCManipulatableScene>("Scene");
    cam->projection.connect(&msc->cameraProjection);
    cam->viewTransform.connect(&msc->cameraTransform);
    cam->connect(&msc->activeCamera);
    setController(msc);

    GCShadingGroup *group = vp->addChild<GCShadingGroup>("Groups");
    msc->shadingGroups.addPointer(group);

    group->shader.setPointed(sha);

    GCGeometryTransform *transform = vp->addChild<GCGeometryTransform>("Transform");
    group->geometry.addPointer(transform);


    GCPlane *shape = vp->addChild<GCPlane>("Sphere");
    transform->geometry.setPointed(&shape->geometry);

    vp->source.setPointed(msc);
    }

  virtual void paintGL()
    {
    SViewport::paintGL();

    if(log())
      {
      const GCShader *s = shader()->uncheckedCastTo<GCShader>();
      QStringList newLog = s->runtimeShader().log();
      if(newLog.size())
        {
        QString oldText = log()->toPlainText();

        QString newText;
        foreach(const QString &t, newLog)
          {
          newText.append(t);
          newText.append("\n");
          }

        if(oldText != newText)
          {
          log()->setPlainText(newText);
          }
        }
      else
        {
        log()->setPlainText("");
        }
      }
    }
  };

xsize SShaderPartEditorInterface::numberOfTypeParameters(SEntity *prop) const
  {
  return SDefaultPartEditorInterface::numberOfTypeParameters(prop) - 1;
  }

void SShaderPartEditorInterface::addProperty(SEntity *e) const
  {
  class Initialiser : public SPropertyInstanceInformationInitialiser
    {
  public:
    void initialise(SPropertyInstanceInformation *i)
      {
      i->setAffects(shader->runtimeShader.instanceInformation());
      }

    GCShader *shader;
    };

  Initialiser i;
  i.shader = e->uncheckedCastTo<GCShader>();

  ColourProperty *newProp = e->addProperty<ColourProperty>("", &i);
  }

QStringList SShaderPartEditorInterface::possiblePropertyTypes() const
  {
  QStringList l;
  foreach(const SPropertyInformation *i, STypeRegistry::types())
    {
    if(i->interfaceFactory(GCShaderBindableData::InterfaceTypeId))
      {
      l << i->typeName();
      }
    }

  qSort(l);
  return l;
  }

xsize SShaderPartEditorInterface::numberOfTypeSubParameters(SEntity *, void *) const
  {
  return NumberOfSubTypeParameters;
  }

void SShaderPartEditorInterface::typeSubParameter(SPartEditorInterfaceFeedbacker *f, SEntity *e, void *prop, xsize i, QString &name, QWidget *&widget) const
  {
  if(i == SubValue)
    {
    name = "Value";

    SEntityUI ui;

    widget = ui.createControlWidget((SProperty*)prop);

    return;
    }

  SDefaultPartEditorInterface::typeSubParameter(f, e, prop, i, name, widget);
  }

QWidget *SShaderPartEditorInterface::buildCustomEditor(SEntity *e) const
  {
  GCShader *shader = e->uncheckedCastTo<GCShader>();
  xAssert(shader);

  QWidget *main(new QWidget);
  QVBoxLayout *layout(new QVBoxLayout(main));
  layout->setContentsMargins(0, 0, 0, 0);

  layout->addWidget(new QLabel("<b>Vertex</b>"));

  GCShaderComponent *vert = e->children.firstChild<GCVertexShaderComponent>();
  if(!vert)
    {
    vert = e->addChild<GCVertexShaderComponent>("Vert");
      {
        QFile shaderResource(":/GLResources/shaders/standardSurface.vert");
        if(shaderResource.open(QIODevice::ReadOnly))
        {
          vert->assign(shaderResource.readAll());
        }
      }
    shader->components.addPointer(vert);
    }



  QWidget *vertex(new SPropertyDefaultUI::LongString(vert, false, 0));
  layout->addWidget(vertex);

  layout->addWidget(new QLabel("<b>Fragment</b>"));

  GCShaderComponent *frag = e->children.firstChild<GCFragmentShaderComponent>();
  if(!frag)
    {
    frag = e->addChild<GCFragmentShaderComponent>("Frag");
      {
        QFile shaderResource(":/GLResources/shaders/standardSurface.frag");
        if(shaderResource.open(QIODevice::ReadOnly))
        {
          frag->assign(shaderResource.readAll());
        }
      }
    shader->components.addPointer(frag);
    }


  QWidget *fragment(new SPropertyDefaultUI::LongString(frag, false, 0));
  layout->addWidget(fragment);

  return main;
  }

QWidget *SShaderPartEditorInterface::buildCustomPreview(const SEntity *e, SPropertyArray *p) const
  {
  QWidget *base = new QWidget;
  QVBoxLayout *layout(new QVBoxLayout(base));
  layout->setContentsMargins(0, 0, 0, 0);

  ShaderPreviewViewport *w(new ShaderPreviewViewport(e, p));
  layout->addWidget(w);

  QTextEdit *text(new QTextEdit);
  layout->addWidget(text);

  w->setLog(text);

  return base;
  }
