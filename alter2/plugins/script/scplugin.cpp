#include "scplugin.h"
#include "splugin.h"
#include "scsurface.h"
#include "scembeddedtypes.h"
#include "QApplication"
#include "QScriptEngine"
#include "UIPlugin.h"
#include "acore.h"
#include "QScriptEngineDebugger"
#include "scshiftentity.h"
#include "QDir"
#include "aplugin.h"
#include "QDebug"
#include "QMainWindow"
#include "scio.h"
#include "sdatabasemodel.h"
#include "QGraphicsObject"
#include "qdeclarative.h"
#include "QMLExtensions/scpath.h"
#include "QMLExtensions/scellipse.h"
#include "QMLExtensions/scbrush.h"
#include "QMLExtensions/scnodedisplay.h"
#include "QMLExtensions/scnodeitem.h"
#include "QMLExtensions/scconnectoritem.h"
#include "QMLExtensions/scmousearea.h"
#include "QGLWidget"
#include "scdeclarativesurface.h"

ALTER_PLUGIN(ScPlugin);

ScPlugin::ScPlugin() : _engine(0), _debugger(0), _surface(0), _types(0), _io(0), _model(0)
  {
  setObjectName("script");
  }

ScPlugin::~ScPlugin()
  {
  delete _io;
  delete _engine;
  delete _types;
  delete _model;

  _model = 0;
  _engine = 0;
  _surface = 0;
  }

void ScPlugin::initDebugger()
  {
  if(_debugger)
    {
    return;
    }

  _debugger = new QScriptEngineDebugger(this);
  _debugger->setAutoShowStandardWindow(true);

  APlugin<UIPlugin> ui(this, "ui");
  if(ui.isValid())
    {
    connect(ui.plugin(), SIGNAL(aboutToClose()), _debugger, SLOT(deleteLater()));
    }
  }


void ScPlugin::pluginAdded(const QString &type)
  {
  if(type != "db")
    {
    QObject *plug = core()->plugin(type);
    xAssert(plug);
    registerScriptGlobal(type, plug);

    if(type == "ui")
      {
      _surface = new ScSurface(this);

      APlugin<UIPlugin> ui(this, "ui");
      xAssert(ui.isValid());
      ui->addSurface(_surface);

      includePath(":/Sc/StartupUI.js");
      }
    }
  }

void ScPlugin::pluginRemoved(const QString &type)
  {
  engine()->globalObject().setProperty(type, QScriptValue());

  if(type == "db")
    {
    delete _model;
    _model = 0;
    }
  else if(type == "ui")
    {
    delete _surface;
    _surface = 0;
    }
  }

QScriptValue printFn(QScriptContext *context, QScriptEngine *engine)
  {
  QString result;
  for (int i = 0; i < context->argumentCount(); ++i)
    {
    result.append(context->argument(i).toString());
    }

  qDebug() << result;

  return engine->undefinedValue();
  }

void ScPlugin::load()
  {
  XProfiler::setStringForContext(ScriptProfileScope, "Script");
  _engine = new QScriptEngine(this);

  _engine->globalObject().setProperty("print", _engine->newFunction(printFn));

  _io = new ScIO;
  registerScriptGlobal(_io);

  registerScriptGlobal(this);

  APlugin<SPlugin> db(this, "db");
  xAssert(db.isValid());

  _engine->globalObject().setProperty("dbTypes", _engine->newObject());
  _types = new ScEmbeddedTypes(_engine);

  connect(_types, SIGNAL(typeAdded(QString)), this, SIGNAL(typeAdded(QString)));
  connect(_types, SIGNAL(typeRemoved(QString)), this, SIGNAL(typeRemoved(QString)));

  registerScriptGlobal("db", ScEmbeddedTypes::packValue(&db->db()));

  QScriptValue dbObject = engine()->globalObject().property("db");
  foreach(const SPropertyInformation *t, STypeRegistry::types())
    {
    QScriptValue type = dbObject.property(t->typeName());
    if(type.isNull())
      {
      type = _engine->newObject();
      type.setProperty("typeName", t->typeName());
      }
    }

  qmlRegisterType<ScPath>("VexxQMLExtensions", 1, 0, "Path");
  qmlRegisterType<ScEllipse>("VexxQMLExtensions", 1, 0, "Ellipse");
  qmlRegisterType<ScGradient>("VexxQMLExtensions", 1, 0, "Gradient");
  qmlRegisterType<ScGradientStop>("VexxQMLExtensions", 1, 0, "GradientStop");
  qmlRegisterType<ScPen>("VexxQMLExtensions", 1, 0, "Pen");
  qmlRegisterType<ScNodeDisplay>("VexxQMLExtensions", 1, 0, "NodeDisplay");
  qmlRegisterType<ScPropertyDisplay>("VexxQMLExtensions", 1, 0, "PropertyDisplay");
  qmlRegisterType<ScNodeItem>("VexxQMLExtensions", 1, 0, "NodeItem");
  qmlRegisterType<ScPropertyItem>("VexxQMLExtensions", 1, 0, "PropertyItem");
  qmlRegisterType<ScConnectorItem>("VexxQMLExtensions", 1, 0, "ConnectorItem");
  qmlRegisterType<ScMouseAndScrollArea>("VexxQMLExtensions", 1, 0, "MouseAndScrollArea");
  qmlRegisterUncreatableType<ScMouseEvent>("VexxQMLExtensions", 1, 0, "MouseEventV2", "thing");

#ifdef X_DEBUG
  core()->addDirectory(core()->rootPath() + "../alter2/plugins/script/");
#endif

  xAssert(!_model);
  _model = new SDatabaseModel(&db->db(), &db->db(), SDatabaseModel::NoOptions);

  include("CoreStartup.js");
  }

void ScPlugin::enableDebugging(bool enable)
  {
  initDebugger();
  emit debuggingStateChanged(enable);
  if(enable)
    {
    _debugger->attachTo(_engine);
    }
  else
    {
    _debugger->detach();
    }
  }

void ScPlugin::showDebugger()
  {
  initDebugger();
  _debugger->standardWindow()->show();
  }

void ScPlugin::hideDebugger()
  {
  if(_debugger)
    {
    _debugger->standardWindow()->hide();
    }
  }

bool ScPlugin::loadPlugin(const QString &plugin)
  {
  ScProfileFunction
  xAssert(core());
  return core()->load(plugin);
  }

void ScPlugin::includePath(const QString &filename)
  {
  ScProfileFunction
  bool result = executeFile(filename);
  if(result)
    {
    qDebug() << " Include File" << filename << " ... Success";
    }
  else
    {
    qWarning() << " Include File" << filename << " ... Failure";
    }
  }

void ScPlugin::include(const QString &filename)
  {
  ScProfileFunction
  qDebug() << "Include File" << filename;
  QString file;
  foreach( const QString &dir, core()->directories() )
    {
    file = dir + QDir::separator() + filename;
    if(QFile::exists(file))
      {
      includePath(file);
      return;
      }
    }
  qWarning() << "Failed to include file " + filename + ", couldn't locate the file in any include directory.";
  }

void ScPlugin::includeFolder(const QString &folder)
  {
  ScProfileFunction
  qDebug() << "Include Folder" << folder;
  QDir dir(folder);
  if(dir.exists())
    {
    foreach(const QFileInfo &filename, dir.entryInfoList(QStringList() << "*.js"))
      {
      includePath(filename.filePath());
      }
    }
  else
    {
    foreach( const QString &dir, core()->directories() )
      {
      QDir relativeDir(dir + QDir::separator() + folder);
      if(relativeDir.exists())
        {
        foreach(const QFileInfo &filename, relativeDir.entryInfoList(QStringList() << "*.js"))
          {
          includePath(filename.filePath());
          }
        }
      }
    }

  qDebug() << "... Success";
  }

void ScPlugin::registerScriptGlobal(QObject *in)
  {
  QScriptValue objectValue = _engine->newQObject(in);
  _engine->globalObject().setProperty(in->objectName(), objectValue);
  }

void ScPlugin::registerScriptGlobal(const QString &name, QObject *in)
  {
  QScriptValue objectValue = _engine->newQObject(in);
  _engine->globalObject().setProperty(name, objectValue);
  }

void ScPlugin::registerScriptGlobal(const QString &name, const QScriptValue &val)
  {
  _engine->globalObject().setProperty(name, val);
  }

void ScPlugin::registerScriptGlobal(const QString &name, QScriptClass *in)
  {
  QScriptValue objectValue = _engine->newObject(in);
  _engine->globalObject().setProperty(name, objectValue);
  }

bool ScPlugin::executeFile(const QString &filename)
  {
  ScProfileFunction
  QFile file(filename);
  if(file.exists() && file.open( QIODevice::ReadOnly))
    {
    QString data(QString::fromUtf8(file.readAll()));
    return execute(data);
    }
  return false;
  }

bool ScPlugin::isDebuggingEnabled()
  {
  return _engine->agent() != 0;
  }

QScriptEngine *ScPlugin::engine()
  {
  return _engine;
  }

QScriptValue ScPlugin::call(QScriptValue fn, QScriptValue th, const QList<QScriptValue> &args)
  {
  QScriptEngine *e = fn.engine();
  QScriptValue ret = fn.call(th, args);

  if(e->hasUncaughtException())
    {
    qWarning() << "Error in script at line " << e->uncaughtExceptionLineNumber() << ": " << endl << ret.toString() << endl;
    return QScriptValue();
    }
  else if(!ret.isUndefined())
    {
    qDebug() << "Script Returned: " << ret.toString() << endl;
    }
  return QScriptValue();
  }

QWidget *ScPlugin::addQMLWindow(const QString &url, const QVariantMap &qmlData)
  {
  APlugin<UIPlugin> ui(this, "ui");
  if(!ui.isValid())
    {
    return 0;
    }

  ScDeclarativeWindow *s = new ScDeclarativeWindow(url, _model, qmlData);

  return s;
  }

QObject *ScPlugin::addQMLSurface(const QString &name, const QString &type, const QString &url, const QVariantMap &qmlData)
  {
  APlugin<UIPlugin> ui(this, "ui");
  if(!ui.isValid())
    {
    return 0;
    }

  UISurface::SurfaceType t = (type == "Properties") ? UISurface::PropertiesPage : UISurface::Dock;

  ScDeclarativeSurface *s = new ScDeclarativeSurface(name, url, t, _model, qmlData);
  ui->addSurface(s);

  return s;
  }

bool ScPlugin::execute(const QString &code)
  {
  ScProfileFunction
  QScriptValue ret = _engine->evaluate(code);

  if(_engine->hasUncaughtException())
    {
    qWarning() << "Error in script at line " << _engine->uncaughtExceptionLineNumber() << ": " << endl << ret.toString() << endl;
    return false;
    }
  else if(!ret.isUndefined())
    {
    qDebug() << "Script Returned: " << ret.toString() << endl;
    }
  return true;
  }
