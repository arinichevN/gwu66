

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

void getTemperature(Device *item) {
    int i;
    item->value_state = 0;
    for (i = 0; i < RETRY_NUM; i++) {
#ifndef PLATFORM_ANY
        printf("reading: id:%d sclk:%d cs:%d so:%d\n", item->id, item->sclk, item->cs, item->miso);
        if (max6675_read(&item->value, item->sclk, item->cs, item->miso)) {
            item->tm = getCurrentTime();
            item->value_state = 1;
            return;
        }
#endif
#ifdef PLATFORM_ANY
        item->value = 0.0f;
        item->tm = getCurrentTime();
        item->value_state = 1;
        return;
#endif
    }
}

int sendStrPack(char qnf, char *cmd) {
    extern Peer peer_client;
    return acp_sendStrPack(qnf, cmd, &peer_client);
}

int sendBufPack(char *buf, char qnf, char *cmd_str) {
    extern Peer peer_client;
    return acp_sendBufPack(buf, qnf, cmd_str, &peer_client);
}

void sendStr(const char *s, uint8_t *crc) {
    acp_sendStr(s, crc, &peer_client);
}

void sendFooter(int8_t crc) {
    acp_sendFooter(crc, &peer_client);
}

int catFTS(Device *item, char *buf, size_t buf_size) {
    char q[LINE_SIZE];
    snprintf(q, sizeof q, "%d" ACP_DELIMITER_COLUMN_STR FLOAT_NUM ACP_DELIMITER_COLUMN_STR "%ld" ACP_DELIMITER_COLUMN_STR "%ld" ACP_DELIMITER_COLUMN_STR "%d" ACP_DELIMITER_ROW_STR, item->id, item->value, item->tm.tv_sec, item->tm.tv_nsec, item->value_state);
    if (bufCat(buf, q, buf_size) == NULL) {
        sendStrPack(ACP_QUANTIFIER_BROADCAST, ACP_RESP_BUF_OVERFLOW);
        return 0;
    }
    return 1;
}