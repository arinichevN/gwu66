#include <string.h>

int readSettings() {
    FILE* stream = fopen(CONFIG_FILE, "r");
    if (stream == NULL) {
#ifdef MODE_DEBUG
        perror("readSettings()");
#endif
        return 0;
    }
skipLine(stream);
    int n;
    n = fscanf(stream, "%d\t%255s\n", &sock_port, pid_path);
    if (n != 2) {
        fclose(stream);
#ifdef MODE_DEBUG
        fputs("ERROR: readSettings: bad row format\n", stderr);
#endif
        return 0;
    }
    fclose(stream);
#ifdef MODE_DEBUG
    printf("readSettings: \n\tsock_port: %d, \n\tpid_path: %s\n", sock_port, pid_path);
#endif
    return 1;
}

#define DEVICE_ROW_FORMAT "%d\t%d\t%d\t%d\n"
#define DEVICE_FIELD_COUNT 4

int initDevice(DeviceList *list) {
    FILE* stream = fopen(DEVICE_FILE, "r");
    if (stream == NULL) {
#ifdef MODE_DEBUG
        fputs("ERROR: initDevice: fopen\n", stderr);
#endif
        return 0;
    }
    skipLine(stream);
    int rnum = 0;
    while (1) {
        int n = 0, x1, x2, x3, x4;
        n = fscanf(stream, DEVICE_ROW_FORMAT, &x1, &x2, &x3, &x4);
        if (n != DEVICE_FIELD_COUNT) {
            break;
        }
        rnum++;
    }
    rewind(stream);

    list->length = rnum;
    if (list->length > 0) {
        list->item = (Device *) malloc(list->length * sizeof *(list->item));
        if (list->item == NULL) {
            list->length = 0;
#ifdef MODE_DEBUG
            fputs("ERROR: initDevice: failed to allocate memory for pins\n", stderr);
#endif
            fclose(stream);
            return 0;
        }
        skipLine(stream);
        int done = 1;
        size_t i;
#ifdef MODE_DEBUG
        puts("initDevice: read:");
#endif
        FORL{
            int n;
            n = fscanf(stream, DEVICE_ROW_FORMAT,
            &LIi.id,
            &LIi.sclk,
            &LIi.miso,
            &LIi.cs
            );
            if (n != DEVICE_FIELD_COUNT) {
                done = 0;
            }
#ifdef MODE_DEBUG
            printf("\tid = %d, sclk = %d, cs = %d, miso = %d\n", LIi.id, LIi.sclk, LIi.cs, LIi.miso);
#endif
            LIi.value_state = 0;
        }
        if (!done) {
            FREE_LIST(list);
            fclose(stream);
#ifdef MODE_DEBUG
            fputs("ERROR: initDevice: failure while reading rows\n", stderr);
#endif
            return 0;
        }
    }
    fclose(stream);
    return 1;
}

int initDeviceLCorrection(DeviceList *list) {
    int i;
    FORL{
        LIi.lcorrection.active=0;
    }
    FILE* stream = fopen(LCORRECTION_FILE, "r");
    if (stream == NULL) {
#ifdef MODE_DEBUG
        fputs("ERROR: initDeviceLCorrection: fopen\n", stderr);
#endif
        return 0;
    }
    skipLine(stream);
    while (1) {
        int n, device_id;
        float factor, delta;
        n = fscanf(stream, "%d\t%f\t%f\n", &device_id, &factor, &delta);
        if (n != 3) {
            break;
        }
        Device * item=getDeviceById(device_id, list);
        if(item==NULL){
            break;
        }
        item->lcorrection.active=1;
        item->lcorrection.factor=factor;
        item->lcorrection.delta=delta;
#ifdef MODE_DEBUG
        printf("initDeviceLCorrection: device_id = %d, factor = %f, delta = %f\n", device_id, factor, delta);
#endif

    }
    fclose(stream);
    return 1;
}
