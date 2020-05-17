TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES +=  librtmp/amf.c \
    librtmp/hashswf.c \
    librtmp/log.c \
    librtmp/parseurl.c \
    librtmp/rtmp.c \
    test_rtmp_pull.cpp \
    test_rtmp_push.cpp \
    rtmpbase.cpp \
    main_publish.cpp \
    pushwork.cpp \
    mediabase.cpp \
    rtmppusher.cpp \
    flvanalyze.cpp

win32 {
INCLUDEPATH += $$PWD/ffmpeg-4.2.1-win32-dev/include
LIBS += -lws2_32 \
        -lwinmm
}


HEADERS += \
    librtmp/amf.h \
    librtmp/bytes.h \
    librtmp/dh.h \
    librtmp/dhgroups.h \
    librtmp/handshake.h \
    librtmp/http.h \
    librtmp/log.h \
    librtmp/rtmp.h \
    librtmp/rtmp_sys.h \
    rtmpbase.h \
    pushwork.h \
    mediabase.h \
    rtmppusher.h \
    flvanalyze.h
