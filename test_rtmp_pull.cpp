#include <librtmp/log.h>
#include <stdio.h>
#include "librtmp/rtmp_sys.h"

#define RD_SUCCESS          0
#define RD_FAILED           1
#define RD_INCOMPLETE		2

#define DEF_TIMEOUT	30	/* seconds */
#define DEF_BUFTIME	(10 * 60 * 60 * 1000)	/* 10 hours default */
#define DEF_SKIPFRM	0

// starts sockets
static int InitSockets()
{
#ifdef WIN32
  WORD version;
  WSADATA wsaData;

  version = MAKEWORD(1, 1);
  return (WSAStartup(version, &wsaData) == 0);
#else
  return TRUE;
#endif
}

static inline void CleanupSockets()
{
#ifdef WIN32
  WSACleanup();
#endif
}


int test_rtmp_pull()
{
    int nStatus = RD_SUCCESS;

    uint32_t dSeek = 0;		// seek position in resume mode, 0 otherwise
    FILE *file = 0;
    int bResume = FALSE;		// true in resume mode
    uint32_t bufferTime = DEF_BUFTIME;

    AVal hostname = { 0, 0 };
    AVal playpath = { 0, 0 };
    AVal subscribepath = { 0, 0 };
    int port = -1;
    int protocol = RTMP_PROTOCOL_UNDEFINED;
    int bLiveStream = FALSE;	// is it a live stream? then we can't seek/resume

    long int timeout = DEF_TIMEOUT;	// timeout connection after 120 seconds
    RTMP rtmp = { 0 };
    uint32_t dStartOffset = 0;	// seek position in non-live mode
    uint32_t dStopOffset = 0;

    AVal swfUrl = { 0, 0 };
    AVal tcUrl = { 0, 0 };
    AVal pageUrl = { 0, 0 };
    AVal app = { 0, 0 };
    AVal auth = { 0, 0 };
    AVal swfHash = { 0, 0 };
    uint32_t swfSize = 0;
    AVal flashVer = { 0, 0 };
    AVal sockshost = { 0, 0 };

    RTMP_LogPrintf("RTMPDump test rtmp pull\n");
    RTMP_LogPrintf("2020-05-04\n");

    char *url = "rtmp://192.168.1.177:1935/live/livestream";
    char *flvFile = "test.flv";

    if (!InitSockets())
    {
      RTMP_Log(RTMP_LOGERROR, "Couldn't load sockets support on your platform, exiting!");
      return -RD_FAILED;
    }

    RTMP_Init(&rtmp);

    {
        AVal parsedHost, parsedApp, parsedPlaypath;
        unsigned int parsedPort = 0;
        int parsedProtocol = RTMP_PROTOCOL_UNDEFINED;

        if (!RTMP_ParseURL(url, &parsedProtocol, &parsedHost, &parsedPort, &parsedPlaypath, &parsedApp))
          //(const char *url, int *protocol, AVal *host, unsigned int *port, AVal *playpath, AVal *app)
        {
            RTMP_Log(RTMP_LOGWARNING, "Couldn't parse the specified url (%s)!", url);
        }
        else
        {
            if (!hostname.av_len)
              hostname = parsedHost;
            if (port == -1)
              port = parsedPort;
            if (playpath.av_len == 0 && parsedPlaypath.av_len)
            {
               playpath = parsedPlaypath;
            }
            if (protocol == RTMP_PROTOCOL_UNDEFINED)
              protocol = parsedProtocol;
            if (app.av_len == 0 && parsedApp.av_len)
            {
              app = parsedApp;
            }
            RTMP_LogPrintf("info (%s) %s %d %s %s\n", url, hostname.av_val, port, playpath.av_val, app.av_val);
        }
    }

    {
        if (!hostname.av_len)
        {
            RTMP_Log(RTMP_LOGERROR,
            "You must specify a hostname (--host) or url (-r \"rtmp://host[:port]/playpath\") containing a hostname");
            return RD_FAILED;
        }
        if (playpath.av_len == 0)
        {
            RTMP_Log(RTMP_LOGERROR,
            "You must specify a playpath (--playpath) or url (-r \"rtmp://host[:port]/playpath\") containing a playpath");
            return RD_FAILED;
        }

        if (protocol == RTMP_PROTOCOL_UNDEFINED)
        {
            RTMP_Log(RTMP_LOGWARNING,
            "You haven't specified a protocol (--protocol) or rtmp url (-r), using default protocol RTMP");
            protocol = RTMP_PROTOCOL_RTMP;
        }
        if (port == -1)
        {
            RTMP_Log(RTMP_LOGWARNING,
            "You haven't specified a port (--port) or rtmp url (-r), using default port 1935");
            port = 0;
        }
        if (port == 0)
        {
            if (protocol & RTMP_FEATURE_SSL)
                port = 443;
            else if (protocol & RTMP_FEATURE_HTTP)
                port = 80;
            else
                port = 1935;
        }
    }

    if (tcUrl.av_len == 0)
    {
        printf("tcUrl.av_len \n");
      char str[512] = { 0 };

      tcUrl.av_len = snprintf(str, 511, "%s://%.*s:%d/%.*s",
           RTMPProtocolStringsLower[protocol], hostname.av_len,
           hostname.av_val, port, app.av_len, app.av_val);
      tcUrl.av_val = (char *) malloc(tcUrl.av_len + 1);
      strcpy(tcUrl.av_val, str);
    }

    // User defined seek offset
    if (dStartOffset > 0)
    {
        // Live stream
        if (bLiveStream)
        {
            RTMP_Log(RTMP_LOGWARNING, "Can't seek in a live stream, ignoring --start option");
            dStartOffset = 0;
        }
    }

    RTMP_SetupStream(&rtmp, protocol, &hostname, port, &sockshost, &playpath,
               &tcUrl, &swfUrl, &pageUrl, &app, &auth, &swfHash, swfSize,
               &flashVer, &subscribepath, dSeek, dStopOffset, bLiveStream, timeout);

    // RTMP_SetupStream(RTMP *r,int protocol,AVal *host,unsigned int port, AVal *sockshost,AVal *playpath,AVal *tcUrl,AVal *swfUrl, AVal *pageUrl, AVal *app,
    //         AVal *auth, AVal *swfSHA256Hash, uint32_t swfSize,AVal *flashVer,AVal *subscribepath,int dStart, int dStop, int bLiveStream, long int timeout)

    file = fopen(flvFile, "w+b");
    if (file == 0)
    {
        RTMP_LogPrintf("Failed to open file! %s\n", flvFile);
        return RD_FAILED;
    }


    int bufferSize = 64 * 1024;
    char *buffer = (char *) malloc(bufferSize);
    int nRead = 0;
    off_t size = 0;

    RTMP_Log(RTMP_LOGDEBUG, "Setting buffer time to: %dms", bufferTime);
    RTMP_SetBufferMS(&rtmp, bufferTime);

    RTMP_LogPrintf("Connecting ...\n");

    if (!RTMP_Connect(&rtmp, NULL))
    {
        nStatus = RD_FAILED;
        printf("RTMP_Connect\n");
        return RD_FAILED;
    }

    RTMP_Log(RTMP_LOGINFO, "Connected...");

    // User defined seek offset
    if (dStartOffset > 0)
    {
        // Don't need the start offset if resuming an existing file
        if (bResume)
        {
            RTMP_Log(RTMP_LOGWARNING, "Can't seek a resumed stream, ignoring --start option");
            dStartOffset = 0;
        }
        else
        {
            dSeek = dStartOffset;
        }
    }

    // Calculate the length of the stream to still play
    if (dStopOffset > 0)
    {
        // Quit if start seek is past required stop offset
        if (dStopOffset <= dSeek)
        {
            RTMP_LogPrintf("Already Completed\n");
            nStatus = RD_SUCCESS;
            return RD_FAILED;
        }
    }

    if (!RTMP_ConnectStream(&rtmp, dSeek))
    {
        nStatus = RD_FAILED;
        printf("RTMP_ConnectStream\n");
        return RD_FAILED;
    }

    do
    {
        nRead = RTMP_Read(&rtmp, buffer, bufferSize);
        RTMP_LogPrintf("nRead: %d\n", nRead);
        if (nRead > 0)
        {
            if (fwrite(buffer, sizeof(unsigned char), nRead, file) != (size_t) nRead)
            {
                RTMP_Log(RTMP_LOGERROR, "%s: Failed writing, exiting!", __FUNCTION__);
                free(buffer);
                return RD_FAILED;
            }
            size += nRead;
        }
    }
    while ( nRead > -1 && RTMP_IsConnected(&rtmp));

    free(buffer);

    RTMP_Close(&rtmp);

    if (file != 0)
        fclose(file);

    CleanupSockets();

    return 0;
}
