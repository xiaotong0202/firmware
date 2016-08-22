/*
 ESP8266WiFiGeneric.cpp - WiFi library for esp8266

 Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 This file is part of the esp8266 core for Arduino environment.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 Reworked on 28 Dec 2015 by Markus Sattler

 */

#include "esp8266_wifi_generic.h"

extern "C" {
#include "c_types.h"
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "smartconfig.h"
#include "lwip/err.h"
#include "lwip/dns.h"
}

extern "C" void esp_schedule();
extern "C" void esp_yield();

/**
 * set new mode
 * @param m WiFiMode_t
 */
bool esp8266_setMode(WiFiMode_t m) {
    if(wifi_get_opmode() == (uint8) m) {
        return true;
    }

    bool ret = false;

    ETS_UART_INTR_DISABLE();
    ret = wifi_set_opmode(m);
    ETS_UART_INTR_ENABLE();

    return ret;
}

/**
 * get WiFi mode
 * @return WiFiMode
 */
WiFiMode_t esp8266_getMode() {
    return (WiFiMode_t) wifi_get_opmode();
}

/**
 * control STA mode
 * @param enable bool
 * @return ok
 */
bool esp8266_enableSTA(bool enable) {

    WiFiMode_t currentMode = esp8266_getMode();
    bool isEnabled = ((currentMode & WIFI_STA) != 0);

    if(isEnabled != enable) {
        if(enable) {
            return esp8266_setMode((WiFiMode_t)(currentMode | WIFI_STA));
        } else {
            return esp8266_setMode((WiFiMode_t)(currentMode & (~WIFI_STA)));
        }
    } else {
        return true;
    }
}

/**
 * control AP mode
 * @param enable bool
 * @return ok
 */
bool esp8266_enableAP(bool enable){

    WiFiMode_t currentMode = esp8266_getMode();
    bool isEnabled = ((currentMode & WIFI_AP) != 0);

    if(isEnabled != enable) {
        if(enable) {
            return esp8266_setMode((WiFiMode_t)(currentMode | WIFI_AP));
        } else {
            return esp8266_setMode((WiFiMode_t)(currentMode & (~WIFI_AP)));
        }
    } else {
        return true;
    }
}

/**
 * Get the station interface MAC address.
 * @param mac   pointer to uint8_t array with length WL_MAC_ADDR_LENGTH
 * @return      pointer to uint8_t *
 */
uint8_t* esp8266_getMacAddress(uint8_t* mac) {
    wifi_get_macaddr(STATION_IF, mac);
    return mac;
}

bool esp8266_setDHCP(char enable)
{
    if(true == enable)
        wifi_station_dhcpc_start();
    else
        wifi_station_dhcpc_stop();
    return true;
}

/**
 * Setting the ESP8266 station to connect to the AP (which is recorded)
 * automatically or not when powered on. Enable auto-connect by default.
 * @param autoConnect bool
 * @return if saved
 */
bool esp8266_setAutoConnect(bool autoConnect) {
    bool ret;
    ETS_UART_INTR_DISABLE();
    ret = wifi_station_set_auto_connect(autoConnect);
    ETS_UART_INTR_ENABLE();
    return ret;
}

/**
 * Return the current network RSSI.
 * @return  RSSI value
 */
int32_t esp8266_getRSSI(void) {
    return wifi_station_get_rssi();
}


bool _smartConfigStarted = false;
bool _smartConfigDone = false;

/**
 * _smartConfigCallback
 * @param st
 * @param result
 */
void smartConfigCallback(uint32_t st, void* result) {
    sc_status status = (sc_status) st;
    if(status == SC_STATUS_LINK) {
        station_config* sta_conf = reinterpret_cast<station_config*>(result);

        wifi_station_set_config(sta_conf);
        wifi_station_disconnect();
        wifi_station_connect();

        _smartConfigDone = true;
    } else if(status == SC_STATUS_LINK_OVER) {
        esp8266_stopSmartConfig();
    }
}

/**
 * Start SmartConfig
 */
bool esp8266_beginSmartConfig() {
    if(_smartConfigStarted) {
        return false;
    }

    if(!esp8266_enableSTA(true)) {
        // enable STA failed
        return false;
    }

    if(smartconfig_start(reinterpret_cast<sc_callback_t>(&smartConfigCallback), 1)) {
        _smartConfigStarted = true;
        _smartConfigDone = false;
        return true;
    }
    return false;
}


/**
 *  Stop SmartConfig
 */
bool esp8266_stopSmartConfig() {
    if(!_smartConfigStarted) {
        return true;
    }

    if(smartconfig_stop()) {
        _smartConfigStarted = false;
        return true;
    }
    return false;
}

/**
 * Query SmartConfig status, to decide when stop config
 * @return smartConfig Done
 */
bool esp8266_smartConfigDone() {
    if(!_smartConfigStarted) {
        return false;
    }

    return _smartConfigDone;
}

void wifi_dns_found_callback(const char *name, ip_addr_t *ipaddr, void *callback_arg) {
    if(ipaddr) {
        (*reinterpret_cast<uint32_t*>(callback_arg)) = ipaddr->addr;
    }
    esp_schedule(); // resume the hostByName function
}

int esp8266_gethostbyname(const char* hostname, uint16_t hostnameLen, uint32_t *ip_addr)
{
    ip_addr_t addr;

    err_t err = dns_gethostbyname(hostname, &addr, &wifi_dns_found_callback, ip_addr);
    if(err == ERR_OK) {
        *ip_addr = addr.addr;
    } else if(err == ERR_INPROGRESS) {
        esp_yield();
        // will return here when dns_found_callback fires
        if(*ip_addr != 0) {
            err = ERR_OK;
        }
    }

    return (err == ERR_OK) ? 1 : 0;
}

int esp8266_connect()
{
    ETS_UART_INTR_DISABLE();
    wifi_station_connect();
    ETS_UART_INTR_ENABLE();
    return 0;
}

int esp8266_disconnect()
{
    ETS_UART_INTR_DISABLE();
    wifi_station_disconnect();
    ETS_UART_INTR_ENABLE();
    return 0;
}

wl_status_t esp8266_status() {
    station_status_t status = wifi_station_get_connect_status();

    switch(status) {
        case STATION_GOT_IP:
            return WL_CONNECTED;
        case STATION_NO_AP_FOUND:
            return WL_NO_SSID_AVAIL;
        case STATION_CONNECT_FAIL:
        case STATION_WRONG_PASSWORD:
            return WL_CONNECT_FAILED;
        case STATION_IDLE:
            return WL_IDLE_STATUS;
        default:
            return WL_DISCONNECTED;
    }
}
