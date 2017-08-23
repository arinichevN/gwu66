
#ifndef GWU66_H
#define GWU66_H

#include "lib/app.h"

#include "lib/gpio.h"

#include "lib/max6675.h"

#include "lib/timef.h"

#include "lib/acp/main.h"
#include "lib/acp/app.h"
#include "lib/udp.h"


#define APP_NAME gwu66
#define APP_NAME_STR TOSTRING(APP_NAME)
#define ID_BROADCAST 0xff
//#define RETRY_NUM 5

#ifdef MODE_FULL
#define CONF_DIR "/etc/controller/" APP_NAME_STR "/"
#endif
#ifndef MODE_FULL
#define CONF_DIR "./"
#endif

#define DEVICE_FILE "" CONF_DIR "device.tsv"
#define CONFIG_FILE "" CONF_DIR "config.tsv"
#define LCORRECTION_FILE "" CONF_DIR "lcorrection.tsv"

#define FLOAT_NUM "%.2f"

enum {
    ON = 1,
    OFF,
    DO,
    INIT,
    WTIME
} StateAPP;

typedef struct {
    int active;
    float factor;
    float delta;
} LCORRECTION;

typedef struct {
    int id;
    int sclk;
    int cs;
    int miso;
    struct timespec tm;
    float value;
    int value_state; //0 if reading value from device failed
       LCORRECTION lcorrection;
} Device;

DEF_LIST(Device)


extern int readSettings();

extern int initDevice(DeviceList *dl);

extern int checkDevice(DeviceList *dl);

extern void serverRun(int *state, int init_state);

extern void initApp();

extern int initData();

extern void freeData();

extern void freeApp();

extern void exit_nicely();

extern void exit_nicely_e(char *s);

#endif 

