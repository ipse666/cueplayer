TEMPLATE = app
TARGET = 
DEPENDPATH += . \
    ui
HEADERS += cueplayer.h \
    cueparser.h \
    transcoder.h \
    apetoflac.h
FORMS += ui/cueplayer.ui \
    ui/transcoder.ui \
    ui/apetoflacdialog.ui
SOURCES += cueplayer.cpp \
    main.cpp \
    cueparser.cpp \
    transcoder.cpp \
    apetoflac.cpp
RESOURCES += ressources.qrc
CONFIG += release \
    link_pkgconfig
PKGCONFIG += gstreamer-0.10
OTHER_FILES += INSTALL \
    INSTALL \
    README
