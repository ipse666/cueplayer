TEMPLATE = app
QT += network
TARGET = cueplayer
isEmpty(PREFIX):PREFIX = /usr/local
target.path = $$PREFIX/bin
target.files = $$TARGET
locale.path = /usr/share/$${TARGET}/loc
locale.files = loc/*.qm
INSTALLS += target locale
DEPENDPATH += . \
    ui
HEADERS += cueplayer.h \
    cueparser.h \
    transcoder.h \
    apetoflac.h \
    videowindow.h \
    videoslider.h \
    callbacks.h \
    streamform.h \
    plparser.h \
    youtubedl.h \
    singleserver.h \
    singleclient.h \
    widgetmanager.h
FORMS += ui/cueplayer.ui \
    ui/transcoder.ui \
    ui/apetoflacdialog.ui \
    ui/videowindow.ui \
    ui/videoslider.ui \
    ui/streamform.ui
SOURCES += cueplayer.cpp \
    main.cpp \
    cueparser.cpp \
    transcoder.cpp \
    apetoflac.cpp \
    videowindow.cpp \
    videoslider.cpp \
    streamform.cpp \
    plparser.cpp \
    youtubedl.cpp \
    singleserver.cpp \
    singleclient.cpp \
    widgetmanager.cpp
RESOURCES += ressources.qrc
CONFIG += release \
    link_pkgconfig \
    warn_on
PKGCONFIG += gstreamer-0.10 \
    gstreamer-interfaces-0.10
OTHER_FILES += INSTALL \
    INSTALL \
    README \
	CHANGELOG
TRANSLATIONS = loc/$${TARGET}_pl_PL.ts
