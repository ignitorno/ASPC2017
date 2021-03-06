
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "AzureIoTHub.h"
#include "sdk/schemaserializer.h"
#include "dht22.h"
#include <stdlib.h>

#include <stdio.h>
#include <stdint.h>
#include <pgmspace.h>
#include <Arduino.h>

// Find under Microsoft Azure IoT Suite -> DEVICES -> <your device> -> Device Details and Authentication Keys
static const char* deviceId = "<DeviceID>";
static const char* deviceKey = "<DevieKey>";
static const char* hubName = "<HubName>";
static const char* hubSuffix = "<HubSuffix>";

//Pins
static int redLedPin = 12;
static int redLedState = LOW;
static int greenLedPin = 13;
static int greenLedState = LOW;

// Define the Model
BEGIN_NAMESPACE(Contoso);

DECLARE_STRUCT(SystemProperties,
    ascii_char_ptr, DeviceID,
    _Bool, Enabled
);

DECLARE_STRUCT(DeviceProperties,
ascii_char_ptr, DeviceID,
_Bool, HubEnabledState
);

DECLARE_MODEL(Thermostat,

    /* Event data (temperature, external temperature and humidity) */
    WITH_DATA(int, Temperature),
    WITH_DATA(int, ExternalTemperature),
    WITH_DATA(int, Humidity),
    WITH_DATA(ascii_char_ptr, DeviceId),

    /* Device Info - This is command metadata + some extra fields */
    WITH_DATA(ascii_char_ptr, ObjectType),
    WITH_DATA(_Bool, IsSimulatedDevice),
    WITH_DATA(ascii_char_ptr, Version),
    WITH_DATA(DeviceProperties, DeviceProperties),
    WITH_DATA(ascii_char_ptr_no_quotes, Commands),

    /* Commands implemented by the device */
    WITH_ACTION(SetTemperature, int, temperature),
    WITH_ACTION(SetHumidity, int, humidity),
    WITH_ACTION(TurnFanOn),
    WITH_ACTION(TurnFanOff)
);

END_NAMESPACE(Contoso);

DEFINE_ENUM_STRINGS(IOTHUB_CLIENT_CONFIRMATION_RESULT, IOTHUB_CLIENT_CONFIRMATION_RESULT_VALUES)

EXECUTE_COMMAND_RESULT SetTemperature(Thermostat* thermostat, int temperature)
{
    LogInfo("Received temperature %d\r\n", temperature);
    thermostat->Temperature = temperature;
    return EXECUTE_COMMAND_SUCCESS;
}

EXECUTE_COMMAND_RESULT SetHumidity(Thermostat* thermostat, int humidity)
{
    LogInfo("Received humidity %d\r\n", humidity);
    thermostat->Humidity = humidity;
    return EXECUTE_COMMAND_SUCCESS;
}
EXECUTE_COMMAND_RESULT TurnFanOff(Thermostat* device)
{
    (void)device;

    LogInfo("Toggling red LED.\r\n");
    if (redLedState != LOW)
    {
        redLedState = LOW;
    }
    else
    {
        redLedState = HIGH;
    }
    digitalWrite(redLedPin, redLedState);

    return EXECUTE_COMMAND_SUCCESS;
}

EXECUTE_COMMAND_RESULT TurnFanOn(Thermostat* device)
{
    (void)device;

    LogInfo("Toggling green LED.\r\n");
    if (greenLedState != LOW)
    {
        greenLedState = LOW;
    }
    else
    {
        greenLedState = HIGH;
    }
    digitalWrite(greenLedPin, greenLedState);

    return EXECUTE_COMMAND_SUCCESS;
}
void sendCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    int messageTrackingId = (intptr_t)userContextCallback;

    LogInfo("Message Id: %d Received.\r\n", messageTrackingId);

    LogInfo("Result Call Back Called! Result is: %s \r\n", ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
}

static void sendMessage(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, const unsigned char* buffer, size_t size)
{
    static unsigned int messageTrackingId;
    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray(buffer, size);
    if (messageHandle == NULL)
    {
        LogInfo("unable to create a new IoTHubMessage\r\n");
    }
    else
    {
        if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, sendCallback, (void*)(uintptr_t)messageTrackingId) != IOTHUB_CLIENT_OK)
        {
            LogInfo("failed to hand over the message to IoTHubClient");
        }
        else
        {
            LogInfo("IoTHubClient accepted the message for delivery\r\n");
        }
        IoTHubMessage_Destroy(messageHandle);
    }
    free((void*)buffer);
    messageTrackingId++;
}

/*this function "links" IoTHub to the serialization library*/
static IOTHUBMESSAGE_DISPOSITION_RESULT IoTHubMessage(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
    IOTHUBMESSAGE_DISPOSITION_RESULT result;
    const unsigned char* buffer;
    size_t size;
    if (IoTHubMessage_GetByteArray(message, &buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        LogInfo("unable to IoTHubMessage_GetByteArray\r\n");
        result = EXECUTE_COMMAND_ERROR;
    }
    else
    {
        /*buffer is not zero terminated*/
        char* temp = malloc(size + 1);
        if (temp == NULL)
        {
            LogInfo("failed to malloc\r\n");
            result = EXECUTE_COMMAND_ERROR;
        }
        else
        {
           

            memcpy(temp, buffer, size);
            temp[size] = '\0';
            EXECUTE_COMMAND_RESULT executeCommandResult = EXECUTE_COMMAND(userContextCallback, temp);
            result =
                (executeCommandResult == EXECUTE_COMMAND_ERROR) ? IOTHUBMESSAGE_ABANDONED :
                (executeCommandResult == EXECUTE_COMMAND_SUCCESS) ? IOTHUBMESSAGE_ACCEPTED :
                IOTHUBMESSAGE_REJECTED;
            free(temp);
        }
    }
    return result;
}

void remote_monitoring_run(void)
{
        initDht();

         digitalWrite(redLedPin, redLedState);
         pinMode(redLedPin, OUTPUT);
    
         digitalWrite(greenLedPin, greenLedState);
         pinMode(greenLedPin, OUTPUT);

        srand((unsigned int)time(NULL));
        if (serializer_init(NULL) != SERIALIZER_OK)
        {
            LogInfo("Failed on serializer_init\r\n");
        }
        else
        {
            IOTHUB_CLIENT_CONFIG config;
            IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

            config.deviceId = deviceId;
            config.deviceKey = deviceKey;
            config.iotHubName = hubName;
            config.iotHubSuffix = hubSuffix;
            config.protocol = HTTP_Protocol;

            /*
             * The new version of AzureIoTHub library introduces this new deviceSasToken parameter. Once we are 
             *    not using it on this sample, we must initialize with NULL, otherwise IoTHubClient_LL_Create 
             *    will fail.
             * As a temporary solution, we will test the definition of AzureIoTHubVersion, which is only defined 
             *    in the new AzureIoTHub library version. Once we totally deprecate the last version, we can take 
             *    the �#ifdef� out.
             */
#ifdef AzureIoTHubVersion
			config.deviceSasToken = NULL;
            config.protocolGatewayHostName = NULL;
#endif

            iotHubClientHandle = IoTHubClient_LL_Create(&config);
            if (iotHubClientHandle == NULL)
            {
                LogInfo("Failed on IoTHubClient_CreateFromConnectionString\r\n");
            }
            else
            {
#ifdef MBED_BUILD_TIMESTAMP
                // For mbed add the certificate information
                if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
                {
                    LogInfo("failure to set option \"TrustedCerts\"\r\n");
                }
#endif // MBED_BUILD_TIMESTAMP

                Thermostat* thermostat = CREATE_MODEL_INSTANCE(Contoso, Thermostat);
                if (thermostat == NULL)
                {
                    LogInfo("Failed on CREATE_MODEL_INSTANCE\r\n");
                }
                else
                {
                    STRING_HANDLE commandsMetadata;

                    if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, IoTHubMessage, thermostat) != IOTHUB_CLIENT_OK)
                    {
                        LogInfo("unable to IoTHubClient_SetMessageCallback\r\n");
                    }
                    else
                    {

                        /* send the device info upon startup so that the cloud app knows
                        what commands are available and the fact that the device is up */
                        thermostat->ObjectType = "DeviceInfo";
                        thermostat->IsSimulatedDevice = false;
                        thermostat->Version = "1.0";
                        thermostat->DeviceProperties.HubEnabledState = true;
                        thermostat->DeviceProperties.DeviceID = (char*)deviceId;

                        commandsMetadata = STRING_new();
                        if (commandsMetadata == NULL)
                        {
                            LogInfo("Failed on creating string for commands metadata\r\n");
                        }
                        else
                        {
                            /* Serialize the commands metadata as a JSON string before sending */
                            if (SchemaSerializer_SerializeCommandMetadata(GET_MODEL_HANDLE(Contoso, Thermostat), commandsMetadata) != SCHEMA_SERIALIZER_OK)
                            {
                                LogInfo("Failed serializing commands metadata\r\n");
                            }
                            else
                            {
                                unsigned char* buffer;
                                size_t bufferSize;
                                thermostat->Commands = (char*)STRING_c_str(commandsMetadata);

                                /* Here is the actual send of the Device Info */
                                if (SERIALIZE(&buffer, &bufferSize, thermostat->ObjectType, thermostat->Version, thermostat->IsSimulatedDevice, thermostat->DeviceProperties, thermostat->Commands) != IOT_AGENT_OK)
                                {
                                    LogInfo("Failed serializing\r\n");
                                }
                                else
                                {
                                    sendMessage(iotHubClientHandle, buffer, bufferSize);
                                }

                            }

                            STRING_delete(commandsMetadata);
                        }

                        thermostat->DeviceId = (char*)deviceId;
                        int sendCycle = 10;
                        int currentCycle = 0;
                        while (1)
                        {
                            if(currentCycle >= sendCycle) {
                                float Temp;
                                float Humi;
                                getNextSample(&Temp, &Humi);
                                //thermostat->Temperature = 50 + (rand() % 10 + 2);
                                thermostat->Temperature = (Temp>600)?600:(int)round(Temp);
                                thermostat->ExternalTemperature = 55 + (rand() % 5 + 2);
                                //thermostat->Humidity = 50 + (rand() % 8 + 2);
                                thermostat->Humidity = (Humi>100)?100:(int)round(Humi);
                                currentCycle = 0;
                                unsigned char*buffer;
                                size_t bufferSize;

                                LogInfo("Sending sensor value Temperature = %d, Humidity = %d\r\n", thermostat->Temperature, thermostat->Humidity);

                                if (SERIALIZE(&buffer, &bufferSize, thermostat->DeviceId, thermostat->Temperature, thermostat->Humidity, thermostat->ExternalTemperature) != IOT_AGENT_OK)
                                {
                                    LogInfo("Failed sending sensor value\r\n");
                                }
                                else
                                {
                                    sendMessage(iotHubClientHandle, buffer, bufferSize);
                                }                               
                            }

                            IoTHubClient_LL_DoWork(iotHubClientHandle);
                            ThreadAPI_Sleep(1000);
                            currentCycle++;
                        }
                    }

                    DESTROY_MODEL_INSTANCE(thermostat);
                }
                IoTHubClient_LL_Destroy(iotHubClientHandle);
            }
            serializer_deinit();

    }
}
