#include <string.h>

int readSettings() {
    FILE* stream = fopen(CONFIG_FILE, "r");
    if (stream == NULL) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s()", F);
        perror("");
#endif
        return 0;
    }
    skipLine(stream);
    int n;
    n = fscanf(stream, "%d\n", &sock_port);
    if (n != 1) {
        fclose(stream);
#ifdef MODE_DEBUG
        fprintf(stderr, "%s(): bad row format\n", F);
#endif
        return 0;
    }
    fclose(stream);
#ifdef MODE_DEBUG
    printf("%s(): \n\tsock_port: %d\n", F, sock_port);
#endif
    return 1;
}

#define DEVICE_ROW_FORMAT "%d\t%d\t%d\t%d\n"
#define DEVICE_FIELD_COUNT 4

int initDevice(DeviceList *list) {
    FILE* stream = fopen(DEVICE_FILE, "r");
    if (stream == NULL) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s()", F);
        perror("");
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
            fprintf(stderr,"%s(): failed to allocate memory for device\n", F);
#endif
            fclose(stream);
            return 0;
        }
        skipLine(stream);
        int done = 1;
        FORLIST(i){
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
            fprintf(stderr,"%s(): failure while reading rows\n", F);
#endif
            return 0;
        }
    }
    fclose(stream);
    return 1;
}

int initDeviceLCorrection(DeviceList *list) {
    FORLIST(i){
        LIi.lcorrection.active = 0;
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
        Device * item = getDeviceById(device_id, list);
        if (item == NULL) {
            break;
        }
        item->lcorrection.active = 1;
        item->lcorrection.factor = factor;
        item->lcorrection.delta = delta;
#ifdef MODE_DEBUG
        printf("%s(): device_id = %d, factor = %f, delta = %f\n",F, device_id, factor, delta);
#endif

    }
    fclose(stream);
    return 1;
}
