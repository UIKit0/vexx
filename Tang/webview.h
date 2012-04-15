#ifndef WEBVIEW_H
#define WEBVIEW_H
#if 0

#include "UISurface.h"
#include "sentityweakpointer.h"

class Object;
class QWebView;

class WebView : public QObject, public UISurface
  {
  Q_OBJECT

XProperties:
  XROProperty(Object *, currentObject);

public:
  WebView(SPropertyArray *objectParent);

public slots:
  void loadObject(const QString& id);
  void load(const QString& url);

  void updateArea();

signals:
  void objectChanged(Object *object);

private slots:
  void loadFinished(bool ok);

private:
  QWebView *_webView;
  SPropertyArray *_objectParent;
  };
#endif
#endif // WEBVIEW_H
