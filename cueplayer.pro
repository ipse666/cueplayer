TEMPLATE = app
QT += network
TARGET = 
INSTALL_ROOT=$$(DESTDIR)
prefix.path = $$PREFIX
isEmpty(PREFIX) {
		prefix.path = /usr/local
}

binaries.path = prefix.path/bin
binaries.files = $$TARGET

INSTALLS += binaries
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
    youtubedl.h
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
    youtubedl.cpp
RESOURCES += ressources.qrc
CONFIG += release \
    link_pkgconfig \
    warn_on
PKGCONFIG += gstreamer-0.10 \
    gstreamer-interfaces-0.10
OTHER_FILES += INSTALL \
    INSTALL \
	README	\
	CHANGELOG
