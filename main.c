/*
 * UPD gateway for MAX6675
 */

#include "main.h"

char pid_path[LINE_SIZE];
int app_state = APP_INIT;
int pid_file = -1;
int proc_id = -1;
int sock_port = -1;
int sock_fd = -1; //socket file descriptor
size_t sock_buf_size = 0;
Peer peer_client = {.fd = &sock_fd, .addr_size = sizeof peer_client.addr};

DeviceList device_list = {NULL, 0};

I1List i1l = {NULL, 0};

struct timespec interval = {.tv_sec = 0, .tv_nsec = 200000000};
Ton_ts tmr = {.ready = 0};

#include "util.c"
#include "print.c"
#include "init_f.c"


#ifdef PLATFORM_ANY
#pragma message("MESSAGE: building for all platforms")
#else
#pragma message("MESSAGE: building for certain platform")
#endif

int checkDevice(DeviceList *dl) {
    size_t i, j;
    //valid pin address
#ifndef PLATFORM_ANY
    for (i = 0; i < dl->length; i++) {
        if (!checkPin(dl->item[i].sclk)) {
            fprintf(stderr, "checkDevice: check device table: bad sclk=%d where id=%d\n", dl->item[i].sclk, dl->item[i].id);
            return 0;
        }
        if (!checkPin(dl->item[i].cs)) {
            fprintf(stderr, "checkDevice: check device table: bad cs=%d where id=%d\n", dl->item[i].cs, dl->item[i].id);
            return 0;
        }
        if (!checkPin(dl->item[i].miso)) {
            fprintf(stderr, "checkDevice: check device table: bad miso=%d where id=%d\n", dl->item[i].miso, dl->item[i].id);
            return 0;
        }
    }
#endif
    //same sclk
    /*
        int pin, f;
        f = 0;
        for (i = 0; i < dl->length; i++) {
            if (!f) {
                pin = dl->item[i].sclk;
                f = 1;
            }
            if (pin != dl->item[i].sclk) {
                fprintf(stderr, "checkDevice: the same sclk pin expected where id=%d\n", dl->item[i].id);
                return 0;
            }
        }
        //same miso
        f = 0;
        for (i = 0; i < dl->length; i++) {
            if (!f) {
                pin = dl->item[i].miso;
                f = 1;
            }
            if (pin != dl->item[i].miso) {
                fprintf(stderr, "checkDevice: the same cs pin expected where id=%d\n", dl->item[i].id);
                return 0;
            }
        }
     */
    //unique cs
    /*
        for (i = 0; i < dl->length; i++) {
            for (j = i + 1; j < dl->length; j++) {
                if (dl->item[i].cs == dl->item[j].cs) {
                    fprintf(stderr, "checkDevice: check device table: cs should be unique, repetition found where id=%d\n", dl->item[i].id);
                    return 0;
                }
            }
        }
     */
    //unique id
    for (i = 0; i < dl->length; i++) {
        for (j = i + 1; j < dl->length; j++) {
            if (dl->item[i].id == dl->item[j].id) {
                fprintf(stderr, "checkDevice: check device table: ids should be unique, repetition found where id=%d\n", dl->item[i].id);
                return 0;
            }
        }
    }
    return 1;
}

void serverRun(int *state, int init_state) {
    char buf_in[sock_buf_size];
    char buf_out[sock_buf_size];
    size_t i;
    memset(buf_in, 0, sizeof buf_in);
    acp_initBuf(buf_out, sizeof buf_out);
    if (recvfrom(sock_fd, (void *) buf_in, sizeof buf_in, 0, (struct sockaddr*) (&(peer_client.addr)), &(peer_client.addr_size)) < 0) {
#ifdef MODE_DEBUG
        perror("serverRun: recvfrom() error");
#endif
        return;
    }
#ifdef MODE_DEBUG
    acp_dumpBuf(buf_in, sizeof buf_in);
#endif    
    if (!acp_crc_check(buf_in, sizeof buf_in)) {
#ifdef MODE_DEBUG
        fputs("WARNING: serverRun: crc check failed\n", stderr);
#endif
        return;
    }
    switch (buf_in[1]) {
        case ACP_CMD_APP_START:
            if (!init_state) {
                *state = APP_INIT_DATA;
            }
            return;
        case ACP_CMD_APP_STOP:
            if (init_state) {
                *state = APP_STOP;
            }
            return;
        case ACP_CMD_APP_RESET:
            *state = APP_RESET;
            return;
        case ACP_CMD_APP_EXIT:
            *state = APP_EXIT;
            return;
        case ACP_CMD_APP_PING:
            if (init_state) {
                sendStrPack(ACP_QUANTIFIER_BROADCAST, ACP_RESP_APP_BUSY);
            } else {
                sendStrPack(ACP_QUANTIFIER_BROADCAST, ACP_RESP_APP_IDLE);
            }
            return;
        case ACP_CMD_APP_PRINT:
            printData(&device_list);
            return;
        case ACP_CMD_APP_HELP:
            printHelp();
            return;
        default:
            if (!init_state) {
                return;
            }
            break;
    }
    switch (buf_in[0]) {
        case ACP_QUANTIFIER_BROADCAST:
        case ACP_QUANTIFIER_SPECIFIC:
            break;
        default:
            return;
    }
    switch (buf_in[1]) {
        case ACP_CMD_GET_FTS:
            switch (buf_in[0]) {
                case ACP_QUANTIFIER_BROADCAST:
                    break;
                case ACP_QUANTIFIER_SPECIFIC:
                    acp_parsePackI1(buf_in, &i1l, sock_buf_size); //id
                    if (i1l.length <= 0) {
                        return;
                    }
                    break;
            }
            break;
        default:
            return;
    }
    switch (buf_in[1]) {
        case ACP_CMD_GET_FTS:
            switch (buf_in[0]) {
                case ACP_QUANTIFIER_BROADCAST:
                    for (i = 0; i < device_list.length; i++) {
                        getTemperature(&device_list.item[i]);
                        if (!catFTS(&device_list.item[i], buf_out, sock_buf_size)) {
                            return;
                        }
                    }
                    break;
                case ACP_QUANTIFIER_SPECIFIC:
                    for (i = 0; i < i1l.length; i++) {
                        Device *device = getDeviceById(i1l.item[i], &device_list);
                        if (device != NULL) {
                            getTemperature(device);
                            if (!catFTS(device, buf_out, sock_buf_size)) {
                                return;
                            }
                        }
                    }
                    break;
            }
            break;
    }
    switch (buf_in[1]) {
        case ACP_CMD_GET_FTS:
            if (!sendBufPack(buf_out, ACP_QUANTIFIER_SPECIFIC, ACP_RESP_REQUEST_SUCCEEDED)) {
                sendStrPack(ACP_QUANTIFIER_BROADCAST, ACP_RESP_BUF_OVERFLOW);
                return;
            }
            return;
    }
}

void initApp() {
#ifdef MODE_DEBUG
    printf("initApp: \n\tCONFIG_FILE: %s, \n\tDEVICE_FILE: %s\n", CONFIG_FILE, DEVICE_FILE);
#endif
    if (!readSettings()) {
        exit_nicely_e("initApp: failed to read settings\n");
    }
    peer_client.sock_buf_size = sock_buf_size;
    if (!initPid(&pid_file, &proc_id, pid_path)) {
        exit_nicely_e("initApp: failed to initialize pid\n");
    }
    if (!initServer(&sock_fd, sock_port)) {
        exit_nicely_e("initApp: failed to initialize socket server\n");
    }
#ifndef PLATFORM_ANY
    if (!gpioSetup()) {
        exit_nicely_e("initApp: failed to initialize GPIO\n");
    }
#endif
}

int initData() {
    if (!initDevice(&device_list)) {
        FREE_LIST(&device_list);
        return 0;
    }
    if (!checkDevice(&device_list)) {
        FREE_LIST(&device_list);
        return 0;
    }
    i1l.item = (int *) malloc(sock_buf_size * sizeof *(i1l.item));
    if (i1l.item == NULL) {
        FREE_LIST(&device_list);
        return 0;
    }
#ifndef PLATFORM_ANY
    size_t i;
    for (i = 0; i < device_list.length; i++) {
        max6675_init(device_list.item[i].sclk, device_list.item[i].cs, device_list.item[i].miso);
    }
#endif
    tmr.ready = 0;
    return 1;
}

void freeData() {
    FREE_LIST(&i1l);
    FREE_LIST(&device_list);
}

void freeApp() {
    freeData();
#ifndef PLATFORM_ANY
    gpioFree();
#endif
    freeSocketFd(&sock_fd);
    freePid(&pid_file, &proc_id, pid_path);
}

void exit_nicely() {
    freeApp();
#ifdef MODE_DEBUG
    puts("\nBye...");
#endif
    exit(EXIT_SUCCESS);
}

void exit_nicely_e(char *s) {
    fprintf(stderr, "%s", s);
    freeApp();
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    if (geteuid() != 0) {
#ifdef MODE_DEBUG
        fprintf(stderr, "%s: root user expected\n", APP_NAME_STR);
#endif
        return (EXIT_FAILURE);
    }
#ifndef MODE_DEBUG
    daemon(0, 0);
#endif
    conSig(&exit_nicely);
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
        perror("main: memory locking failed");
    }
    //#ifndef MODE_DEBUG
    setPriorityMax(SCHED_FIFO);
    //#endif
    int data_initialized = 0;
    while (1) {
        switch (app_state) {
            case APP_INIT:
#ifdef MODE_DEBUG
                puts("MAIN: init");
#endif
                initApp();
                app_state = APP_INIT_DATA;
                break;
            case APP_INIT_DATA:
#ifdef MODE_DEBUG
                puts("MAIN: init data");
#endif
                data_initialized = initData();
                app_state = APP_RUN;
                break;
            case APP_RUN:
#ifdef MODE_DEBUG
                puts("MAIN: run");
#endif
                serverRun(&app_state, data_initialized);
                break;
            case APP_STOP:
#ifdef MODE_DEBUG
                puts("MAIN: stop");
#endif
                freeData();
                data_initialized = 0;
                app_state = APP_RUN;
                break;
            case APP_RESET:
#ifdef MODE_DEBUG
                puts("MAIN: reset");
#endif
                freeApp();
                data_initialized = 0;
                app_state = APP_INIT;
                break;
            case APP_EXIT:
#ifdef MODE_DEBUG
                puts("MAIN: exit");
#endif
                exit_nicely();
                break;
            default:
                exit_nicely_e("main: unknown application state");
                break;
        }
    }
    freeApp();
    return (EXIT_SUCCESS);
}

