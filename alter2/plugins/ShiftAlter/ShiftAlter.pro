#-------------------------------------------------
#
# Project created by QtCreator 2010-10-31T08:46:00
#
#-------------------------------------------------

TARGET = ShiftAlter
TEMPLATE = lib

include("../../../EksCore/GeneralOptions.pri")


SOURCES += splugin.cpp \
    sappdatabase.cpp \
    saparteditor.cpp \
    saparteditorinterface.cpp \
    sashaderparteditorinterface.cpp \
    saviewport.cpp \
    sadocument.cpp

HEADERS += splugin.h \
    saglobal.h \
    sappdatabase.h \
    saparteditor.h \
    saparteditorinterface.h \
    sashaderparteditorinterface.h \
    saviewport.h \
    sadocument.h

INCLUDEPATH += $$ROOT/EksCore $$ROOT/EksGui $$ROOT/Eks3D/include $$ROOT/Shift ../../ $$ROOT/Shift/GraphicsCore $$ROOT/Shift/MathsCore

QT += opengl
LIBS += -lshift -lEksCore -lEksGui -lEks3D -lalter -lShiftGraphicsCore -lShiftMathsCore

FORMS +=

RESOURCES += \
    Resources.qrc






















