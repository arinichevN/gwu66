
#include "main.h"

FUN_LIST_GET_BY_ID(Device)

int deviceIdExists(int id) {
    int i, found;
    found = 0;
    for (i = 0; i < device_list.length; i++) {
        if (id == device_list.item[i].id) {
            found = 1;
            break;
        }
    }
    if (!found) {
        return 0;
    }

    return 1;
}

void lcorrect(Device *item) {
    if (item->lcorrection.active) {
        item->value = item->value * item->lcorrection.factor + item->lcorrection.delta;
    }
}

void getTemperature(Device *item) {
    item->value_state = 0;
    #ifdef CPU_ANY
    item->value = 0.0f;
    item->tm = getCurrentTime();
    item->value_state = 1;
    lcorrect(item);
    return;
#endif

    printf("reading: id:%d sclk:%d cs:%d so:%d\n", item->id, item->sclk, item->cs, item->miso);
    if (max6675_read(&item->value, item->sclk, item->cs, item->miso)) {
        item->tm = getCurrentTime();
        item->value_state = 1;
        lcorrect(item);
        return;
    }

}

int catFTS(Device *item, ACPResponse *response) {
    return acp_responseFTSCat(item->id, item->value, item->tm, item->value_state, response);
}