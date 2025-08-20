QT += quick quickcontrols2



SOURCES += \
        main.cpp \
        timebackend.cpp \
        weatherbackend.cpp

resources.files = main.qml 
resources.prefix = /$${TARGET}
RESOURCES += resources \
    flags.qrc \
    img.qrc


# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    timebackend.h \
    weatherbackend.h

TRANSLATIONS +=

DISTFILES +=



