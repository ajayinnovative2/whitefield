#define _WFOT_MAIN_C_

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <openthread/config.h>
#include <openthread-core-config.h>
#include <assert.h>

#include <openthread/cli.h>
#include <openthread/diag.h>
#include <openthread/openthread.h>
#include <openthread/platform/platform.h>
#include <openthread/platform/logging.h>

#include <openthread/platform/debug_uart.h>
#include <openthread/platform/uart.h>

#include <commline/commline.h>

#if OPENTHREAD_ENABLE_MULTIPLE_INSTANCES
#error "Does not support multiple instances"
#endif

void otTaskletsSignalPending(otInstance *aInstance)
{
    (void)aInstance;
}

void wfot_handle_ini(const char *ini)
{
    FILE *fp;
    char line[512], *ptr;

    if(!ini) return;
    fp = fopen(ini, "rt");
    if(!fp) {
        ERROR("Error opening INI file <%s>\n", ini);
        exit(1);
    }
    while((ptr = fgets(line, sizeof(line), fp))) {
        while(isspace(*ptr)) ptr++;
        if(!(*ptr) || *ptr=='#') continue;
        otPlatUartReceived((uint8_t*)ptr, strlen(ptr));
    }
    fclose(fp);
    INFO("Finished with openthread ini <%s>\r\n", ini);
}

void pty_send_cmd(char *buf, int buflen)
{
    INFO("Sending buf:[%.*s]\r\n", buflen, buf);
    otPlatUartReceived((uint8_t*)buf, buflen);
}

extern int start_pty_thread(int);
extern int NODE_ID;
int main(int argc, char *argv[])
{
    otInstance *sInstance;

    PlatformInit(argc, argv);

    sInstance = otInstanceInitSingle();
    assert(sInstance);

    otCliUartInit(sInstance);

#if OPENTHREAD_ENABLE_DIAG
    otDiagInit(sInstance);
#endif
    wfot_handle_ini(getenv("INI"));
    start_pty_thread(NODE_ID-1);

    while (1)
    {
        otTaskletsProcess(sInstance);
        PlatformProcessDrivers(sInstance);
    }
    return 0;
}

/*
 * Provide, if required an "otPlatLog()" function
 */
#if OPENTHREAD_CONFIG_LOG_OUTPUT == OPENTHREAD_CONFIG_LOG_OUTPUT_APP
void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char *aFormat, ...)
{
    OT_UNUSED_VARIABLE(aLogLevel);
    OT_UNUSED_VARIABLE(aLogRegion);
    OT_UNUSED_VARIABLE(aFormat);

    va_list ap;
    va_start(ap, aFormat);
    otCliPlatLogv(aLogLevel, aLogRegion, aFormat, ap);
    va_end(ap);
}
#endif
