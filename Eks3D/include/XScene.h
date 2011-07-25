#ifndef XSCENE_H
#define XSCENE_H

#include "X3DGlobal.h"
#include "XObject"
#include "XDoodad.h"

class XAbstractRenderer;
class XCamera;

class EKS3D_EXPORT XScene : public XObject
    {
    X_OBJECT( XScene, XObject, 3 )

public:
    XROProperty( XAbstractRenderer *, renderer );
    XROProperty( XCamera *, camera );
    XROProperty( QSize, viewportSize );

public:
    XScene( XAbstractRenderer *, XCamera *cam=0 );
    ~XScene( );

    void addDoodad( XDoodad * );

    void setViewportSize( QSize );
    void setCamera( XCamera * );

    void renderScene( );

    X_SIGNAL( frameEvent, XFrameEvent * );
    X_SIGNAL( update );

private:
    XDoodad *_root;
    friend class XDoodad;
    };

#endif // XSCENE_H
