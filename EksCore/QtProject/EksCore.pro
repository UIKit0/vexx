# -------------------------------------------------
# Project created by QtCreator 2010-01-16T14:33:07
# -------------------------------------------------
TARGET = EksCore
TEMPLATE = lib
DEFINES += EKSCORE_LIBRARY
SOURCES += ../src/XObject.cpp \
    ../src/XMessageRouter.cpp \
    ../src/XMessageHandler.cpp \
    ../src/XVariant.cpp \
    ../src/XColour.cpp \
    ../src/XVectorXD.cpp \
    ../src/XFileSequence.cpp \
    ../src/XMatrixXxX.cpp \
    ../src/XChildEvent.cpp \
    ../src/XBaseObject.cpp \
    ../src/XSize.cpp
HEADERS += ../XObject \
    ../XGlobal \
    ../XVector \
    ../XVariant \
    ../XString \
    ../XSet \
    ../XMap \
    ../XList \
    ../XHash \
    ../XMessageRouter \
    ../XMessageHandler \
    ../XProperty \
    ../XVector2D \
    ../XVector3D \
    ../XVector4D \
    ../XColour \
    ../XQuaternion \
    ../XDebug \
    ../XMatrix4x4 \
    ../XMatrix3x3 \
    ../XGenericMatrix \
    ../XFileSequence \
    ../XChildEvent \
    ../XSignal \
    ../XBaseObject \
    ../XSize
INCLUDEPATH += ../
DESTDIR = ../../bin
