#include "Arduino.h"
#include "neosensory_esp32.h"
#include <BLEDevice.h>
#include <BuzzBase64.h>

/*
	neosensory_esp32.cpp - Library for connecting to 
	Neosensory hardware via ESP32 boards.
	Created by Mithun Das, November 23, 2020.
*/
void BuzzAdvertisedDeviceCallbacks::onResult(BLEAdvertisedDevice advertisedDevice)
{

    Serial.println(advertisedDevice.toString().c_str());
    if (advertisedDevice.haveName())
    {

        String name = String(advertisedDevice.getName().c_str());

        if (name.indexOf("Buzz") != -1)
        {
            Serial.println("Found Neosensory Buzz Wristband");
            

            pServerAddress = new BLEAddress(advertisedDevice.getAddress());
            buzzFound_ = true;
            advertisedDevice.getScan()->stop();
        }

    } // Found our server
}

void BuzzBLEClientCallbacks::onConnect(BLEClient *pClient)
{
    Serial.println("#### onConnect");
}

void BuzzBLEClientCallbacks::onDisconnect(BLEClient *pClient)
{
    NeosensoryESP32::NeoESP32->externalDisconnectedCallback();
    Serial.println("#### onDisconnect");
    pClient->disconnect();
    delete (pServerAddress);
    NeosensoryESP32::NeoESP32->startScan();

    
}

NeosensoryESP32::NeosensoryESP32(char device_id[], uint8_t num_motors,
                                 uint8_t initial_min_vibration, uint8_t initial_max_vibration)
{
    NeoESP32 = this;
    //setDeviceId(device_id);
    num_motors_ = num_motors;
    max_vibration = initial_max_vibration;
    min_vibration = initial_min_vibration;

    // TODO: get this from firmware rather than hardcoding
    firmware_frame_duration_ = 16;
    uint8_t mtu = 247;
    max_frames_per_bt_package_ = (uint8_t)((mtu - 17) / (num_motors_ * (4 / 3.0f)));

    previous_motor_array_ = (uint8_t *)malloc(sizeof(uint8_t) * num_motors_);
    memset(previous_motor_array_, 0, sizeof(uint8_t) * num_motors_);
    is_authorized_ = false;
    buzzFound_ = false;
    is_connected_ = false;
}

void NeosensoryESP32::begin(void)
{

    BLEDevice::init("");

    pBLEScan = BLEDevice::getScan(); //create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new BuzzAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
    pBLEScan->setInterval(1000);
    pBLEScan->setWindow(999);

    startScan();
}

bool NeosensoryESP32::startScan(void)
{
    Serial.println("Start scanning...");
    is_authorized_ = false;
    buzzFound_ = false;
    is_connected_ = false;

    pBLEScan->stop();
    pBLEScan->start(SCAN_TIME, false);
    Serial.println("scanning started...");
    pBLEScan->clearResults();
    while (!buzzFound_)
    {
    }

    
    is_connected_ = connectToBuzz(pServerAddress);

    

    if (externalConnectedCallback)
    {
        externalConnectedCallback(is_connected_);
    }
}
bool NeosensoryESP32::connectToBuzz(BLEAddress *pAddress)
{
    Serial.printf("Connecting to Buzz...%s\n", pAddress->toString().c_str());
    if (pClient == NULL)
    {
        Serial.println("Creating BLE Client");
        pClient = BLEDevice::createClient();
        pClient->setClientCallbacks(new BuzzBLEClientCallbacks());
    }

    pClient->connect(*pAddress, BLE_ADDR_TYPE_RANDOM);

    if (!pClient->isConnected())
    {
        Serial.println("Failed to connect to BLE Server ");
        return false;
    }

    Serial.println(" - Connected to Buzz ");

    // Obtain a reference to the Nordic UART service on the remote BLE server.
    BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService != nullptr)
    {
        Serial.printf("Remote BLE service reference established with UUID=%s\n", pRemoteService->getUUID().toString().c_str());
    }
    else
    {
        Serial.print("Failed to find Nordic UART service UUID: ");
        Serial.println(serviceUUID.toString().c_str());
        return false;
    }

    // Obtain a reference to the TX characteristic of the Nordic UART service on the remote BLE server.
    pTXCharacteristic = pRemoteService->getCharacteristic(charUUID_TX);
    if (pTXCharacteristic == nullptr)
    {
        Serial.print("Failed to find TX characteristic UUID: ");
        Serial.println(charUUID_TX.toString().c_str());
        return false;
    }
    Serial.printf("Remote BLE TX characteristic reference established with UUID=%s\n", pTXCharacteristic->getUUID().toString().c_str());
    pTXCharacteristic->registerForNotify(readNotifyCallbackWrapper);

    // Obtain a reference to the RX characteristic of the Nordic UART service on the remote BLE server.
    pRXCharacteristic = pRemoteService->getCharacteristic(charUUID_RX);
    if (pRXCharacteristic == nullptr)
    {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(charUUID_RX.toString().c_str());
        return false;
    }
    Serial.printf("Remote BLE RX characteristic reference established with UUID=%s\n", pRXCharacteristic->getUUID().toString().c_str());

    return true;
    //sendCommand("auth as developer\n");
    //sendCommand("accept\n");
}

void NeosensoryESP32::setConnectedCallback(
    ConnectedCallback connectedCallback)
{
    externalConnectedCallback = connectedCallback;
}

void NeosensoryESP32::setDisconnectedCallback(
    DisconnectedCallback disconnectedCallback)
{
    externalDisconnectedCallback = disconnectedCallback;
}

void NeosensoryESP32::setReadNotifyCallback(
    ReadNotifyCallback readNotifyCallback)
{
    externalReadNotifyCallback = readNotifyCallback;
}

void NeosensoryESP32::readNotifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{

    // for (int i = 0; i < length; i++)
    // {
    //     Serial.print((char)pData[i]); // Print character to uart
    //     //Serial.print(pData[i]);           // print raw data to uart
    //     Serial.print(" ");
    // }
    // Serial.println();
    parseCliData(pData, length);

    if (externalReadNotifyCallback)
    {
        externalReadNotifyCallback(pBLERemoteCharacteristic, pData, length, isNotify);
    }
}

void NeosensoryESP32::handleCliJson(String jsonMessage)
{
    //  Serial.println(jsonMessage);
    //  Serial.println("$$$");

    if (jsonMessage.indexOf('Developer API access granted!') != -1)
    {
        is_authorized_ = true;
    }

  
}

void NeosensoryESP32::parseCliData(uint8_t *data, uint16_t len)
{
    for (int i = 0; i < len; i++)
    {
        if (data[i] == '{')
        {
            jsonStarted_ = true;
            jsonMessage_ = "";
        }
        if (jsonStarted_)
        {
            jsonMessage_ += (char)data[i];
        }
        if (data[i] == '}')
        {
            jsonStarted_ = false;
            handleCliJson(jsonMessage_);
        }
    }
}

/* Hardware */

uint8_t NeosensoryESP32::num_motors(void)
{
    return num_motors_;
}

uint8_t NeosensoryESP32::firmware_frame_duration(void)
{
    return firmware_frame_duration_;
}

uint8_t NeosensoryESP32::max_frames_per_bt_package(void)
{
    return max_frames_per_bt_package_;
}

/* Motor Control */

/** @brief Translates a linear intensity value into a 
 *	linearly perceived motor intensity value
 */
uint8_t linearIntensityToMotorSpace(
    float linear_intensity, uint8_t min_intensity, uint8_t max_intensity)
{
    if (linear_intensity <= 0)
    {
        return 0;
    }
    if (linear_intensity >= 1)
    {
        return max_intensity;
    }
    return uint8_t((exp(linear_intensity) - 1) /
                       (exp(1) - 1) * (max_intensity - min_intensity) +
                   min_intensity);
}

/** @brief Translates an array of intensities from linear space to motor space
 *	@param[in] lin_array Array of intensities from (0, 1)
 *	@param[out] motor_space_array Array of motor intensities 
 *	corresponding to the linear intensities
 *	@param[in] array_size Number of values in the arrays
 *	@note Translates an array of intensities between (0, 1) to 
 *	(THRESHOLD_INTENSITY, MAX_INTENSITY) on an exponential curve,
 *	so that each linear step in the lin_array feels like a linear
 *	change on the skin. This is due to the Weber Curve, which 
 *	shows that larger increases in intensity are needed for larger
 *	intensities than for lesser intensities, if the same 
 *	perceptual change is to be felt.
 */
void NeosensoryESP32::getMotorIntensitiesFromLinArray(
    float lin_array[], uint8_t motor_space_array[], size_t array_size)
{
    for (int i = 0; i < array_size; i++)
    {
        float input = lin_array[i];
        motor_space_array[i] = linearIntensityToMotorSpace(
            input, min_vibration, max_vibration);
    }
}

/** @brief Checks if two arrays are equal
 *	@param[in] arr1 First array
 *	@param[in] arr2 Second array
 *	@param[in] arr_len Length of both arrays
 *	@return True if arrays have equal values at all indices, else False
 */
bool compareArrays(uint8_t arr1[], uint8_t arr2[], size_t arr_len)
{
    for (int i = 0; i < arr_len; ++i)
    {
        if (arr1[i] != arr2[i])
        {
            return false;
        }
    }
    return true;
}

/** @brief Encode an array of motor intensity byte values 
 *	into a Base64 encoded string
 *	@param[in] motor_intensities The array of motor intensities to encode
 *	@param[in] arr_len Length of the array
 *	@pararm[out] encoded_motor_intensities Char array to fill 
 *	with encoded string
 *	@return Array of characters which is the Base64 encoded string
 */
void encodeMotorIntensities(
    uint8_t *motor_intensities, size_t arr_len, char encoded_motor_intensities[])
{
    size_t input_arr_size = sizeof(uint8_t) * arr_len;
    base64_encode(
        encoded_motor_intensities, (char *)motor_intensities, input_arr_size);
}

/** @brief Converts motor intensities to base64 encoded array and sends appropriate command
 *	@param[in] motor_intensities The motor intensities to send. If multiple frames, this
 *	is a flattened array. 
 *	@param[in] num_frames The number of frames in motor_intensities. Cannot be more 
 *	than max_frames_per_bt_package_.
 */
void NeosensoryESP32::sendMotorCommand(uint8_t motor_intensities[], size_t num_frames)
{
    num_frames = std::min(max_frames_per_bt_package_, static_cast<uint8_t>(num_frames));
    char encoded_motor_intensities[base64_enc_len(sizeof(uint8_t) * num_motors_ * num_frames)];
    encodeMotorIntensities(
        motor_intensities, num_motors_ * num_frames, encoded_motor_intensities);
    sendCommand("motors vibrate ");
    sendCommand(encoded_motor_intensities);
    sendCommand("\n");
}

void NeosensoryESP32::vibrateMotors(float intensities[])
{
    uint8_t motor_intensities[num_motors_];
    getMotorIntensitiesFromLinArray(intensities, motor_intensities, num_motors_);

    if (compareArrays(motor_intensities, previous_motor_array_, num_motors_))
    {
        return;
    }
    memcpy(previous_motor_array_, motor_intensities, sizeof(uint8_t) * num_motors_);

    sendMotorCommand(motor_intensities);
}

void NeosensoryESP32::vibrateMotors(float *intensities[], int num_frames)
{
    num_frames = std::min(max_frames_per_bt_package_, static_cast<uint8_t>(num_frames));
    float flat_intensities[num_motors_ * num_frames];
    for (int i = 0; i < num_frames; ++i)
    {
        for (int j = 0; j < num_motors_; ++j)
        {
            flat_intensities[i * num_motors_ + j] = intensities[i][j];
        }
    }
    uint8_t motor_intensities[num_motors_ * num_frames];
    getMotorIntensitiesFromLinArray(flat_intensities, motor_intensities, num_motors_ * num_frames);

    sendMotorCommand(motor_intensities, num_frames);
}

void NeosensoryESP32::turnOffAllMotors(void)
{
    float motor_intensities[num_motors_];
    memset(motor_intensities, 0, sizeof(float) * num_motors_);
    vibrateMotors(motor_intensities);
}

void NeosensoryESP32::vibrateMotor(uint8_t motor, float intensity)
{
    float motor_intensities[num_motors_];
    memset(motor_intensities, 0, sizeof(float) * num_motors_);
    motor_intensities[motor] = intensity;
    vibrateMotors(motor_intensities);
}

/* CLI */

void NeosensoryESP32::sendCommand(char cmd[])
{

    pRXCharacteristic->writeValue(cmd, strlen(cmd));
}

bool NeosensoryESP32::isConnected(void)
{
    return is_connected_;
}

bool NeosensoryESP32::isAuthorized(void)
{
    return is_authorized_;
}

void NeosensoryESP32::authorizeDeveloper(void)
{
    sendCommand("auth as developer\n");
}

void NeosensoryESP32::acceptTermsAndConditions(void)
{
    sendCommand("accept\n");
}

void NeosensoryESP32::stopAlgorithm(void)
{
    audioStop();
    motorsStart();
}

void NeosensoryESP32::deviceInfo(void)
{
    sendCommand("device info\n");
}

void NeosensoryESP32::motorsStart(void)
{
    sendCommand("motors start\n");
}

void NeosensoryESP32::motorsStop(void)
{
    sendCommand("motors stop\n");
}

void NeosensoryESP32::motorsClearQueue(void)
{
    sendCommand("motors clear_queue\n");
}

void NeosensoryESP32::deviceBattery(void)
{
    sendCommand("device battery_soc\n");
}

void NeosensoryESP32::audioStart(void)
{
    sendCommand("audio start\n");
}

void NeosensoryESP32::audioStop(void)
{
    sendCommand("audio stop\n");
}

NeosensoryESP32 *NeosensoryESP32::NeoESP32 = 0;

void readNotifyCallbackWrapper(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
    NeosensoryESP32::NeoESP32->readNotifyCallback(pBLERemoteCharacteristic, pData, length, isNotify);
}

