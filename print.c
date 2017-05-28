
void printData(DeviceList *list) {
    int i = 0;
    char q[LINE_SIZE];
    uint8_t crc = 0;
    snprintf(q, sizeof q, "CONFIG_FILE: %s\n", CONFIG_FILE);
    sendStr(q, &crc);
    snprintf(q, sizeof q, "DEVICE_FILE: %s\n", DEVICE_FILE);
    sendStr(q, &crc);
    snprintf(q, sizeof q, "port: %d\n", sock_port);
    sendStr(q, &crc);
    snprintf(q, sizeof q, "pid_path: %s\n", pid_path);
    sendStr(q, &crc);
    snprintf(q, sizeof q, "sock_buf_size: %d\n", sock_buf_size);
    sendStr(q, &crc);
    snprintf(q, sizeof q, "app_state: %s\n", getAppState(app_state));
    sendStr(q, &crc);
    snprintf(q, sizeof q, "PID: %d\n", proc_id);
    sendStr(q, &crc);
    sendStr("+-----------------------------------------------------------------------+\n", &crc);
    sendStr("|                                device                                 |\n", &crc);
    sendStr("+-----------+-----------+-----------+-----------+-----------+-----------+\n", &crc);
    sendStr("|     id    |   sclk    |     cs    |    miso   |   value   |value_state|\n", &crc);
    sendStr("+-----------+-----------+-----------+-----------+-----------+-----------+\n", &crc);
    FORL{
        snprintf(q, sizeof q, "|%11d|%11d|%11d|%11d|%11f|%11d|\n",
        LIi.id,
        LIi.sclk,
        LIi.cs,
        LIi.miso,
        LIi.value,
        LIi.value_state
        );
        sendStr(q, &crc);
    }
    sendStr("+-----------+-----------+-----------+-----------+-----------+-----------+\n", &crc);
    sendFooter(crc);
}

void printHelp() {
    char q[LINE_SIZE];
    uint8_t crc = 0;
    sendStr("COMMAND LIST\n", &crc);
    snprintf(q, sizeof q, "%c\tput process into active mode; process will read configuration\n", ACP_CMD_APP_START);
    sendStr(q, &crc);
    snprintf(q, sizeof q, "%c\tput process into standby mode; all running programs will be stopped\n", ACP_CMD_APP_STOP);
    sendStr(q, &crc);
    snprintf(q, sizeof q, "%c\tfirst stop and then start process\n", ACP_CMD_APP_RESET);
    sendStr(q, &crc);
    snprintf(q, sizeof q, "%c\tterminate process\n", ACP_CMD_APP_EXIT);
    sendStr(q, &crc);
    snprintf(q, sizeof q, "%c\tget state of process; response: B - process is in active mode, I - process is in standby mode\n", ACP_CMD_APP_PING);
    sendStr(q, &crc);
    snprintf(q, sizeof q, "%c\tget some variable's values; response will be packed into multiple packets\n", ACP_CMD_APP_PRINT);
    sendStr(q, &crc);
    snprintf(q, sizeof q, "%c\tget this help; response will be packed into multiple packets\n", ACP_CMD_APP_HELP);
    sendStr(q, &crc);

    snprintf(q, sizeof q, "%c\tget temperature in format: sensorId_temperature_timeSec_timeNsec_valid; program id expected if '.' quantifier is used\n", ACP_CMD_GET_FTS);
    sendStr(q, &crc);
    sendFooter(crc);
}
