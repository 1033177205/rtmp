TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    flvanalyze.cpp \
    mediabase.cpp \
    pushwork.cpp \
    rtmpbase.cpp \
    rtmppusher.cpp \
    test_rtmp_pull.cpp \
    test_rtmp_push.cpp \
    librtmp/amf.c \
    librtmp/hashswf.c \
    librtmp/log.c \
    librtmp/parseurl.c \
    librtmp/rtmp.c \
    main_paly.cpp \
    pullwork.cpp \
    rtmpplayer.cpp

win32 {
INCLUDEPATH += $$PWD/ffmpeg-4.2.1-win32-dev/include
LIBS += -lws2_32 \
        -lwinmm
}

HEADERS += \
    flvanalyze.h \
    mediabase.h \
    pushwork.h \
    rtmpbase.h \
    rtmppusher.h \
    librtmp/amf.h \
    librtmp/bytes.h \
    librtmp/dh.h \
    librtmp/dhgroups.h \
    librtmp/handshake.h \
    librtmp/http.h \
    librtmp/log.h \
    librtmp/rtmp.h \
    librtmp/rtmp_sys.h \
    pullwork.h \
    rtmpplayer.h
