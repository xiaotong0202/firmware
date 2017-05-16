/**
 ******************************************************************************
  Copyright (c) 2013-2014 IntoRobot Team.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
  ******************************************************************************
*/

#include "intorobot_config.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "ajson.h"
#include "wiring.h"
#include "wiring_string.h"
#include "system_cloud_def.h"
#include "system_cloud.h"
#include "system_datapoint.h"
#include "system_lorawan.h"

/*debug switch*/
#define SYSTEM_DATAPOINT_DEBUG

#ifdef SYSTEM_DATAPOINT_DEBUG
#define SDATAPOINT_DEBUG(...)  do {DEBUG(__VA_ARGS__);}while(0)
#define SDATAPOINT_DEBUG_D(...)  do {DEBUG_D(__VA_ARGS__);}while(0)
static void debug_dump(const char* buf, int len)
{
    int i = 0;

    if(len > 0) {
        for(i = 0; i < len-1; i++)
        {
            DEBUG_D("%02x:", buf[i]);
        }
        DEBUG_D("%02x\r\n", buf[i]);
    }
}
#define SDATAPOINT_DEBUG_DUMP  debug_dump
#else
#define SDATAPOINT_DEBUG(...)
#define SDATAPOINT_DEBUG_D(...)
#define SDATAPOINT_DEBUG_DUMP
#endif

volatile datapoint_control_t g_datapoint_control = {true, DP_TRANSMIT_MODE_AUTOMATIC, DATAPOINT_TRANSMIT_AUTOMATIC_INTERVAL, 0};

property_conf_t *properties[PROPERTIES_MAX];
int properties_count = 0;

static int intorobotDiscoverProperty(const uint16_t dpID)
{
    for (int i = 0; i < properties_count; i++)
    {
        if (properties[i]->dpID == dpID) {
            return i;
        }
    }
    return -1;
}

static bool intorobotPropertyChanged(void)
{
    for (int i = 0; i < properties_count; i++)
    {
        if (properties[i]->change) {
            return true;
        }
    }
    return false;
}

static uint8_t intorobotGetPropertyCount(void)
{
    return properties_count;
}

static void intorobotPropertyChangeClear(void)
{
    for (int i = 0; i < properties_count; i++)
    {
        if (properties[i]->change) {
            properties[i]->change = false;
        }
    }
}

bool intorobotDatapointOpened(void)
{
    return g_datapoint_control.datapoint_function_open;
}

dp_transmit_mode_t intorobotGetDatapointTransmitMode(void)
{
    return g_datapoint_control.datapoint_transmit_mode;
}

void intorobotDatapointControl(bool open, dp_transmit_mode_t mode, uint32_t lapse)
{
    g_datapoint_control.datapoint_function_open = open;
    if(g_datapoint_control.datapoint_function_open) {
        g_datapoint_control.datapoint_transmit_mode = mode;
        if(DP_TRANSMIT_MODE_AUTOMATIC == g_datapoint_control.datapoint_transmit_mode) {
            if(lapse < DATAPOINT_TRANSMIT_AUTOMATIC_INTERVAL) {
                g_datapoint_control.datapoint_transmit_lapse = DATAPOINT_TRANSMIT_AUTOMATIC_INTERVAL;
            } else {
                g_datapoint_control.datapoint_transmit_lapse = lapse;
            }
        }
    }
}

void intorobotDefineDatapointBool(const uint16_t dpID, const dp_permission_t permission, const bool value, const dp_policy_t policy, const int lapse)
{
    int lapseTemp = lapse;

    if(!intorobotDatapointOpened()) {
        return;
    }

    if (-1 == intorobotDiscoverProperty(dpID)) {
        if(DP_POLICY_NONE == policy) {
            lapseTemp = 0;
        }
        // Create property structure
        property_conf_t *prop = new property_conf_t {dpID, DATA_TYPE_BOOL, permission, policy, (long)lapseTemp*1000, 0, false, RESULT_DATAPOINT_OLD};
        prop->value = String(value);
        properties[properties_count] = prop; // Save pointer to scructure
        properties_count++; // count the number of properties
    }
}

void intorobotDefineDatapointNumber(const uint16_t dpID, const dp_permission_t permission, const double minValue, const double maxValue, const int resolution, const double value, const dp_policy_t policy, const int lapse)
{
    int lapseTemp = lapse;

    if(!intorobotDatapointOpened()) {
        return;
    }

    if (-1 == intorobotDiscoverProperty(dpID)) {
        property_conf_t *prop;
        double defaultValue = value;
        if(DP_POLICY_NONE == policy) {
            lapseTemp = 0;
        }
        // Create property structure
        prop = new property_conf_t {dpID, DATA_TYPE_NUM, permission, policy, (long)lapseTemp*1000, 0, false, RESULT_DATAPOINT_OLD};
        prop->numberProperty.minValue = String(minValue, resolution).toDouble();
        prop->numberProperty.maxValue = String(maxValue, resolution).toDouble();
        if(resolution < 0) {
            prop->numberProperty.resolution = 0;
        } else if (resolution > 4) {
            prop->numberProperty.resolution = 4;
        } else {
            prop->numberProperty.resolution = resolution;
        }
        if(defaultValue < minValue) {
            defaultValue = minValue;
        } else if(defaultValue > maxValue) {
            defaultValue = maxValue;
        }
        prop->value = String(defaultValue, resolution);
        properties[properties_count] = prop; // Save pointer to scructure
        properties_count++; // count the number of properties
    }
}

void intorobotDefineDatapointEnum(const uint16_t dpID, const dp_permission_t permission, const int value, const dp_policy_t policy, const int lapse)
{
    int lapseTemp = lapse;

    if(!intorobotDatapointOpened()) {
        return;
    }

    if (-1 == intorobotDiscoverProperty(dpID)) {
        double defaultValue = value;
        if(DP_POLICY_NONE == policy) {
            lapseTemp = 0;
        }
        // Create property structure
        property_conf_t *prop = new property_conf_t {dpID, DATA_TYPE_ENUM, permission, policy, (long)lapseTemp*1000, 0, false, RESULT_DATAPOINT_OLD};
        if(defaultValue < 0) {
            defaultValue = 0;
        }
        prop->value = String(defaultValue);
        properties[properties_count] = prop; // Save pointer to scructure
        properties_count++; // count the number of properties
    }
}

void intorobotDefineDatapointString(const uint16_t dpID, const dp_permission_t permission, const char *value, const dp_policy_t policy, const int lapse)
{
    int lapseTemp = lapse;

    if(!intorobotDatapointOpened()) {
        return;
    }

    if (-1 == intorobotDiscoverProperty(dpID)) {
        if(DP_POLICY_NONE == policy) {
            lapseTemp = 0;
        }
        // Create property structure
        property_conf_t *prop = new property_conf_t {dpID, DATA_TYPE_STRING, permission, policy, (long)lapseTemp*1000, 0, false, RESULT_DATAPOINT_OLD};
        prop->value = value;
        properties[properties_count] = prop; // Save pointer to scructure
        properties_count++; // count the number of properties
    }
}

void intorobotDefineDatapointBinary(const uint16_t dpID, const dp_permission_t permission, const uint8_t *value, const uint16_t len, const dp_policy_t policy, const int lapse)
{
    int lapseTemp = lapse;

    if(!intorobotDatapointOpened()) {
        return;
    }

    if (-1 == intorobotDiscoverProperty(dpID)) {
        if(DP_POLICY_NONE == policy) {
            lapseTemp = 0;
        }

        // Create property structure
        property_conf_t *prop = new property_conf_t {dpID, DATA_TYPE_BINARY, permission, policy, (long)lapseTemp*1000, 0, false, RESULT_DATAPOINT_OLD};
        prop->valueBinary.value = malloc(len);
        if(NULL != prop->valueBinary.value) {
            memcpy(prop->valueBinary.value, value, len);
            prop->valueBinary.len = len;
        }
        properties[properties_count] = prop; // Save pointer to scructure
        properties_count++; // count the number of properties
    }
}

read_datapoint_result_t intorobotReadDatapointBool(const uint16_t dpID, bool &value)
{
    if(!intorobotDatapointOpened()) {
        return RESULT_DATAPOINT_NONE;
    }

    int index = intorobotDiscoverProperty(dpID);
    if (index == -1) {
        return RESULT_DATAPOINT_NONE;
    }

    value = (bool)(properties[index]->value.toInt());
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;
    return readResult;
}

read_datapoint_result_t intorobotReadDatapointInt(const uint16_t dpID, int &value)
{
    if(!intorobotDatapointOpened()) {
        return RESULT_DATAPOINT_NONE;
    }

    int index = intorobotDiscoverProperty(dpID);
    if (index == -1) {
        return RESULT_DATAPOINT_NONE;
    }

    value = (int)(properties[index]->value.toInt());
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;
    return readResult;
}

read_datapoint_result_t intorobotReadDatapointInt32(const uint16_t dpID, int32_t &value)
{
    if(!intorobotDatapointOpened()) {
        return RESULT_DATAPOINT_NONE;
    }

    int index = intorobotDiscoverProperty(dpID);
    if (index == -1) {
        return RESULT_DATAPOINT_NONE;
    }

    value = (int32_t)(properties[index]->value.toInt());
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;
    return readResult;
}

read_datapoint_result_t intorobotReadDatapointUint32(const uint16_t dpID, uint32_t &value)
{
    if(!intorobotDatapointOpened()) {
        return RESULT_DATAPOINT_NONE;
    }

    int index = intorobotDiscoverProperty(dpID);
    if (index == -1) {
        return RESULT_DATAPOINT_NONE;
    }

    value = (uint32_t)(properties[index]->value.toInt());
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;
    return readResult;
}

read_datapoint_result_t intorobotReadDatapointFloat(const uint16_t dpID, float &value)
{
    if(!intorobotDatapointOpened()) {
        return RESULT_DATAPOINT_NONE;
    }

    int index = intorobotDiscoverProperty(dpID);
    if (index == -1) {
        return RESULT_DATAPOINT_NONE;
    }

    value = properties[index]->value.toDouble();
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;
    return readResult;
}

read_datapoint_result_t intorobotReadDatapointDouble(const uint16_t dpID, double &value)
{
    if(!intorobotDatapointOpened()) {
        return RESULT_DATAPOINT_NONE;
    }

    int index = intorobotDiscoverProperty(dpID);
    if (index == -1) {
        return RESULT_DATAPOINT_NONE;
    }

    value = properties[index]->value.toDouble();
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;
    return readResult;
}

read_datapoint_result_t intorobotReadDatapointString(const uint16_t dpID, String &value)
{
    if(!intorobotDatapointOpened()) {
        return RESULT_DATAPOINT_NONE;
    }

    int index = intorobotDiscoverProperty(dpID);
    if (index == -1) {
        return RESULT_DATAPOINT_NONE;
    }

    value = properties[index]->value;
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;
    return readResult;
}

read_datapoint_result_t intorobotReadDatapointStringChar(const uint16_t dpID, char *value)
{
    if(!intorobotDatapointOpened()) {
        return RESULT_DATAPOINT_NONE;
    }

    int index = intorobotDiscoverProperty(dpID);
    if (index == -1) {
        return RESULT_DATAPOINT_NONE;
    }

    value = (char *)(properties[index]->value.c_str());
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;
    return readResult;
}

read_datapoint_result_t intorobotReadDatapointBinary(const uint16_t dpID, uint8_t *value, uint16_t &len)
{
    if(!intorobotDatapointOpened()) {
        return RESULT_DATAPOINT_NONE;
    }

    int index = intorobotDiscoverProperty(dpID);
    if (index == -1) {
        return RESULT_DATAPOINT_NONE;
    }

    value = properties[index]->valueBinary.value;
    len = properties[index]->valueBinary.len;
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;
    return readResult;
}

// type   0: 平台控制写数据   1：用户写数据
void intorobotWriteDatapoint(const uint16_t dpID, const char* value, const uint8_t type )
{
    if(!intorobotDatapointOpened()) {
        return RESULT_DATAPOINT_NONE;
    }

    int i = intorobotDiscoverProperty(dpID);

    if (i == -1) {
        // not found, nothing to do
        return;
    }

    String valueTemp;
    if(DATA_TYPE_NUM == properties[i]->dataType) {
        //根据分辨率  截取小数点位数。
        if(String(value).toDouble() < properties[i]->numberProperty.minValue) {
            valueTemp = String(properties[i]->numberProperty.minValue, properties[i]->numberProperty.resolution);
        } else if(String(value).toDouble() > properties[i]->numberProperty.maxValue) {
            valueTemp = String(properties[i]->numberProperty.maxValue, properties[i]->numberProperty.resolution);
        } else {
            valueTemp = String(String(value).toDouble(), properties[i]->numberProperty.resolution);
        }
    } else {
        valueTemp = value;
    }

    if(!properties[i]->value.equals(valueTemp)) {  //数据不相等
        properties[i]->change = true;
        if(type) { //用户操作
            properties[i]->readFlag = RESULT_DATAPOINT_OLD;
        } else {
            properties[i]->readFlag = RESULT_DATAPOINT_NEW;
        }
        properties[i]->value = valueTemp;
    } else {
        properties[i]->change = false;
        if(type) { //用户操作
            properties[i]->readFlag = RESULT_DATAPOINT_OLD;
        } else {
            properties[i]->readFlag = RESULT_DATAPOINT_NEW;
        }
    }
}

void intorobotWriteDatapointBinary(const uint16_t dpID, const uint8_t* value, const uint16_t len, const uint8_t type )
{
    if(!intorobotDatapointOpened()) {
        return RESULT_DATAPOINT_NONE;
    }

    int i = intorobotDiscoverProperty(dpID);

    if (i == -1) {
        // not found, nothing to do
        return;
    }

    if(DATA_TYPE_BINARY == properties[i]->dataType) {
        if(memcmp(properties[i]->valueBinary.value, value, len)) {
            if(type) { //用户操作
                properties[i]->change = true;
                properties[i]->readFlag = RESULT_DATAPOINT_OLD;
            } else {
                properties[i]->change = true;
                properties[i]->readFlag = RESULT_DATAPOINT_NEW;
            }

            if(NULL != properties[i]->valueBinary.value) {
                free(properties[i]->valueBinary.value);
            }
            properties[i]->valueBinary.value = malloc(len);
            if(NULL != properties[i]->valueBinary.value) {
                memcpy(properties[i]->valueBinary.value, value, len);
                properties[i]->valueBinary.len = len;
            }
        } else {
            if(type) { //用户操作
                properties[i]->change = false;
                properties[i]->readFlag = RESULT_DATAPOINT_OLD;
            } else {
                properties[i]->change = true;
                properties[i]->readFlag = RESULT_DATAPOINT_NEW;
            }
        }
    }
}

void intorobotParseReceiveDatapoints(uint8_t *payload, uint16_t len)
{
    //dpid(1-2 bytes)+data type(1 byte)+data len(1-2 bytes)+data(n bytes)
    //大端表示，如果最高位是1，则表示两个字节，否则是一个字节
    int32_t index = 0;
    uint16_t dpID = 0;
    uint8_t dataType;
    uint16_t dataLength=0;
    int i = 0;

    if(!intorobotDatapointOpened()) {
        return;
    }

    SDATAPOINT_DEBUG("OK! Rev datapoint data <%d>: ", len);
    SDATAPOINT_DEBUG_DUMP(payload, len);
    if(payload[index++] != BINARY_DATA_FORMAT) {
        SDATAPOINT_DEBUG("Error! Not Binary data format");
        return;
    }

    while(index < len) {

        if(payload[index] & 0x80)  {      //数据点有2个字节
            dpID = ((payload[index] & 0x7F) << 8) | payload[index+1]; //去掉最高位
            index += 2;
        } else {
            dpID = payload[index];
            index += 1;
        }

        i = intorobotDiscoverProperty(dpID);
        if((i == -1) || (DP_PERMISSION_UP_ONLY == properties[i]->permission)) {
            return;
        }

        dataType = payload[index++];
        switch(dataType) {
            case DATA_TYPE_BOOL:
                {
                    index++;
                    bool valueBool = payload[index++];
                    if(DATA_TYPE_BOOL == properties[i]->dataType) {
                        intorobotWriteDatapoint(dpID, String(valueBool).c_str(), 0);
                    }
                    break;
                }

            case DATA_TYPE_NUM:
                {
                    uint32_t valueUint32 = 0;
                    dataLength = payload[index++];
                    if(dataLength == 1) {
                        valueUint32 = payload[index];
                        index += 1;
                    } else if(dataLength == 2) {
                        valueUint32 = (payload[index] << 8) | payload[index+1];
                        index += 2;
                    } else if(dataLength == 4) {
                        valueUint32 = (payload[index] << 24) | (payload[index+1] << 16) | (payload[index+2] << 8) | payload[index+3];
                        index += 4;
                    }
                    if(DATA_TYPE_NUM == properties[i]->dataType) {
                        double valueDouble = (valueUint32 / pow(10, properties[i]->numberProperty.resolution)) + properties[i]->numberProperty.minValue;
                        intorobotWriteDatapoint(dpID, String(valueDouble).c_str(), 0);
                    }
                    break;
                }

            case DATA_TYPE_ENUM:
                {
                    index++;
                    uint8_t valueUint8 = payload[index++];
                    if(DATA_TYPE_ENUM == properties[i]->dataType) {
                        intorobotWriteDatapoint(dpID, String(valueUint8).c_str(), 0);
                    }
                }
                break;

            case DATA_TYPE_STRING:
                {
                    if(payload[index] & 0x80)  {      //长度有2个字节
                        dataLength = ((payload[index] & 0x7F) << 8) | payload[index+1]; //去掉最高位
                        index += 2;
                    } else {
                        dataLength = payload[index];
                        index += 1;
                    }
                    char *str = (char *)malloc(dataLength+1);
                    if(NULL != str) {
                        memset(str, 0, dataLength+1);
                        memcpy(str, &payload[index], dataLength);
                        if(DATA_TYPE_STRING == properties[i]->dataType) {
                            intorobotWriteDatapoint(dpID, str, 0);
                        }
                        free(str);
                    }
                    index += dataLength;
                }
                break;

            case DATA_TYPE_BINARY:
                {
                    if(payload[index] & 0x80)  {      //长度有2个字节
                        dataLength = ((payload[index] & 0x7f) << 8) | payload[index+1]; //去掉最高位
                        index += 2;
                    } else {
                        dataLength = payload[index];
                        index+=1;
                    }
                    if(DATA_TYPE_BINARY == properties[i]->dataType) {
                        intorobotWriteDatapointBinary(dpID, &payload[index], dataLength,0);
                    }
                    index += dataLength;
                }
                break;

            default:
                break;
        }
    }
    system_notify_event(event_cloud_data, ep_cloud_data_datapoint);
}

static uint16_t intorobotFormSingleDatapoint(int property_index, uint8_t* buffer, uint16_t len)
{
    int32_t index = 0;

    if(properties[property_index]->dpID < 0x80) {
        buffer[index++] = properties[property_index]->dpID & 0xFF;
    } else {
        buffer[index++] = (properties[property_index]->dpID >> 8) | 0x80;
        buffer[index++] = properties[property_index]->dpID & 0xFF;
    }
    switch(properties[property_index]->dataType)
    {
        case DATA_TYPE_BOOL:       //bool型
            {
                buffer[index++] = DATA_TYPE_BOOL;  //类型
                buffer[index++] = 0x01;  //长度
                buffer[index++] = (bool)(properties[property_index]->value.toInt());
                break;
            }
        case DATA_TYPE_NUM:        //数值型 int型
            {
                buffer[index++] = DATA_TYPE_NUM;
                uint32_t value = (properties[property_index]->value.toDouble() - properties[property_index]->numberProperty.minValue) \
                                 * pow(10, properties[property_index]->numberProperty.resolution);
                if(value & 0xFFFF0000) {
                    buffer[index++] = 0x04;
                    buffer[index++] = (value >> 24) & 0xFF;
                    buffer[index++] = (value >> 16) & 0xFF;
                    buffer[index++] = (value >> 8) & 0xFF;
                    buffer[index++] = value & 0xFF;
                } else if(value & 0xFFFFFF00) {
                    buffer[index++] = 0x02;
                    buffer[index++] = (value >> 8) & 0xFF;
                    buffer[index++] = value & 0xFF;
                } else {
                    buffer[index++] = 0x01;
                    buffer[index++] = value & 0xFF;
                }
            }
            break;
        case DATA_TYPE_ENUM:       //枚举型
            {
                buffer[index++] = DATA_TYPE_ENUM;
                buffer[index++] = 0x01;
                buffer[index++] = (uint8_t)properties[property_index]->value.toInt() & 0xFF;
                break;
            }
        case DATA_TYPE_STRING:     //字符串型
            {
                uint16_t strlen = properties[property_index]->value.length();

                buffer[index++] = DATA_TYPE_STRING;
                if(strlen < 0x80) {
                    buffer[index++] = strlen & 0xFF;
                } else {
                    buffer[index++] = (strlen >> 8) | 0x80;
                    buffer[index++] = strlen & 0xFF;
                }
                memcpy(&buffer[index], properties[property_index]->value.c_str(), strlen);
                index+=strlen;
                break;
            }
        case DATA_TYPE_BINARY:     //二进制型
            {
                uint16_t len = properties[property_index]->valueBinary.len;

                buffer[index++] = DATA_TYPE_BINARY;
                if(len < 0x80) {
                    buffer[index++] = len & 0xFF;
                } else {
                    buffer[index++] = (len >> 8) | 0x80;
                    buffer[index++] = len & 0xFF;
                }
                memcpy(&buffer[index], properties[property_index]->valueBinary.value, len);
                index+=len;
                break;
            }
        default:
            break;
    }
    return index;
}

// type   0: 组织改变的数据点   1：组织全部的数据点
static uint16_t intorobotFormAllDatapoint(char* buffer, uint16_t len, uint8_t type)
{
    int32_t index = 0;

    for (int i = 0; i < properties_count; i++)
    {
        //只允许下发  不上传
        if (DP_PERMISSION_DOWN_ONLY == properties[i]->permission) {
            continue;
        }

        //系统默认dpID  不上传
        if (properties[i]->dpID > 0xFF00) {
            continue;
        }

        if( type || ((!type) && properties[i]->change) )  {
            index += intorobotFormSingleDatapoint(i, buffer+index, len);
        }
    }
    return index;
}

//datepoint process
void intorobotSendSingleDatapoint(const uint16_t dpID, const char* value)
{
    if(!intorobotDatapointOpened()) {
        return RESULT_DATAPOINT_NONE;
    }

    int i = intorobotDiscoverProperty(dpID);

    if (i == -1) {
        // not found, nothing to do
        return;
    }

    intorobotWriteDatapoint(dpID, value, 1);

    if(DP_TRANSMIT_MODE_AUTOMATIC == intorobotGetDatapointTransmitMode()) {
        return;
    }

    //只允许下发
    if ( DP_PERMISSION_DOWN_ONLY == properties[i]->permission ) {
        SDATAPOINT_DEBUG("only permit cloud -> terminal %d", properties[i]->dpID);
        return;
    }

    //数值未发生变化
    if ( !(properties[i]->change) && (DP_POLICY_ON_CHANGE == properties[i]->policy) ) {
        SDATAPOINT_DEBUG("No Changes for %d:%d", properties[i]->dpID, value);
        return;
    }

    //发送时间间隔到
    system_tick_t current_millis = millis();
    system_tick_t elapsed_millis = current_millis - properties[i]->runtime;
    if (elapsed_millis < 0) {
        elapsed_millis =  0xFFFFFFFF - properties[i]->runtime + current_millis;
    }

    if (elapsed_millis >= properties[i]->lapse) {
        uint8_t buffer[512];
        uint16_t index = 0;

        buffer[index++] = BINARY_DATA_FORMAT;
        index += intorobotFormSingleDatapoint(i, buffer+index, sizeof(buffer)-1);
        SDATAPOINT_DEBUG_D("manual send data:");
        SDATAPOINT_DEBUG_DUMP(buffer, index);
#ifndef configNO_CLOUD
        intorobot_publish(TOPIC_VERSION_V2, INTOROBOT_MQTT_RX_TOPIC, buffer, index, 0, false);
#endif
#ifndef configNO_LORAWAN
        intorobot_lorawan_send_data(buffer, index);
#endif
        properties[i]->runtime = current_millis;
    }
}

//datepoint process
void intorobotSendSingleDatapointBinary(const uint16_t dpID, const uint8_t *value, const uint16_t len)
{
    if(!intorobotDatapointOpened()) {
        return RESULT_DATAPOINT_NONE;
    }

    int i = intorobotDiscoverProperty(dpID);

    if (i == -1) {
        // not found, nothing to do
        return;
    }

    //只允许下发
    if ( DP_PERMISSION_DOWN_ONLY == properties[i]->permission ) {
        SDATAPOINT_DEBUG("only permit cloud -> terminal %d", properties[i]->dpID);
        return;
    }

    //数值未发生变化
    if ( !(properties[i]->change) && (DP_POLICY_ON_CHANGE == properties[i]->policy) ) {
        SDATAPOINT_DEBUG("No Changes for %d:%d", properties[i]->dpID, value);
        return;
    }

    intorobotWriteDatapointBinary(dpID, value, len, 1);

    if(DP_TRANSMIT_MODE_AUTOMATIC == intorobotGetDatapointTransmitMode()) {
        return;
    }

    //发送时间间隔到
    system_tick_t current_millis = millis();
    system_tick_t elapsed_millis = current_millis - properties[i]->runtime;
    if (elapsed_millis < 0) {
        elapsed_millis =  0xFFFFFFFF - properties[i]->runtime + current_millis;
    }

    if (elapsed_millis >= properties[i]->lapse) {
        uint8_t buffer[512];
        uint16_t index = 0;

        buffer[index++] = BINARY_DATA_FORMAT;
        index += intorobotFormSingleDatapoint(i, buffer+index, sizeof(buffer)-1);
        SDATAPOINT_DEBUG_D("manual send binary data:");
        SDATAPOINT_DEBUG_DUMP(buffer, index);
#ifndef configNO_CLOUD
        intorobot_publish(TOPIC_VERSION_V2, INTOROBOT_MQTT_RX_TOPIC, buffer, index, 0, false);
#endif
#ifndef configNO_LORAWAN
        intorobot_lorawan_send_data(buffer, index);
#endif
        properties[i]->runtime = current_millis;
    }
}

void intorobotSendAllDatapointManual(void)
{
    uint8_t buffer[512];
    uint16_t index = 0;

    if(!intorobotDatapointOpened() || (0 == intorobotGetPropertyCount())) {
        return;
    }

    if(DP_TRANSMIT_MODE_AUTOMATIC == intorobotGetDatapointTransmitMode()) {
        return;
    }

    buffer[index++] = BINARY_DATA_FORMAT;
    index += intorobotFormAllDatapoint(buffer+index, sizeof(buffer)-1, 1);
    SDATAPOINT_DEBUG_D("manual send all data:");
    SDATAPOINT_DEBUG_DUMP(buffer, index);
#ifndef configNO_CLOUD
    intorobot_publish(TOPIC_VERSION_V2, INTOROBOT_MQTT_RX_TOPIC, buffer, index, 0, false);
#endif
#ifndef configNO_LORAWAN
    intorobot_lorawan_send_data(buffer, index);
#endif
}

void intorobotSendDatapointAutomatic(void)
{
    uint8_t buffer[512];
    uint16_t index = 0;
    bool sendFlag = false;

    if(!intorobotDatapointOpened() || (0 == intorobotGetPropertyCount())) {
        return;
    }

    if(DP_TRANSMIT_MODE_MANUAL == intorobotGetDatapointTransmitMode()) {
        return;
    }

    //当数值发生变化
    if(intorobotPropertyChanged()) {
        buffer[index++] = BINARY_DATA_FORMAT;
        index += intorobotFormAllDatapoint(buffer+index, sizeof(buffer)-1, 0);
        sendFlag = true;
    } else {
        //发送时间间隔到
        system_tick_t current_millis = millis();
        system_tick_t elapsed_millis = current_millis - g_datapoint_control.runtime;
        if (elapsed_millis < 0) {
            elapsed_millis =  0xFFFFFFFF - g_datapoint_control.runtime + current_millis;
        }

        //发送时间时间到
        if ( elapsed_millis >= DATAPOINT_TRANSMIT_AUTOMATIC_INTERVAL*1000 ) {
            buffer[index++] = BINARY_DATA_FORMAT;
            index += intorobotFormAllDatapoint(buffer+index, sizeof(buffer)-1, 1);
            sendFlag = true;
        }
    }

    if(sendFlag) {
        SDATAPOINT_DEBUG_D("automatic send all data:");
        SDATAPOINT_DEBUG_DUMP(buffer, index);
#ifndef configNO_CLOUD
        intorobot_publish(TOPIC_VERSION_V2, INTOROBOT_MQTT_RX_TOPIC, buffer, index, 0, false);
#endif
#ifndef configNO_LORAWAN
        intorobot_lorawan_send_data(buffer, index);
#endif
        g_datapoint_control.runtime = millis();
        intorobotPropertyChangeClear();
    }
}
