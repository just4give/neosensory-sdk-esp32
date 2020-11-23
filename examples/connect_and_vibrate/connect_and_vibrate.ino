#include <neosensory_esp32.h>


NeosensoryESP32 NeoESP32;

int motor = 0;
float intensity = 0;
float **rumble_frames;


void setup() {
  Serial.begin(115200);
  NeoESP32.setConnectedCallback(onConnected);
  NeoESP32.setReadNotifyCallback(onReadNotify);
  NeoESP32.setDisconnectedCallback(onDisconnected);
  NeoESP32.begin();
  set_rumble_frames();
  while (!NeoESP32.isConnected() || !NeoESP32.isAuthorized()) {}
  NeoESP32.deviceInfo();
  NeoESP32.deviceBattery();

}

void loop() {
  if (NeoESP32.isConnected() && NeoESP32.isAuthorized()) {
    NeoESP32.vibrateMotor(motor, intensity);
    intensity += 0.1;
    if (intensity > 1) {
      intensity = 0;
      motor++;
      if (motor >= NeoESP32.num_motors()) {
        motor = 0;
        rumble();
        rumble();
        rumble();
      }
    }
    delay(500);
  }
}

void set_rumble_frames() {
  rumble_frames = new float*[NeoESP32.max_frames_per_bt_package()];
  for (int i = 0; i < NeoESP32.max_frames_per_bt_package(); i++) {
    rumble_frames[i] = new float[NeoESP32.num_motors()];
    for (int j = 0; j < NeoESP32.num_motors(); j++) {
      rumble_frames[i][j] = (i % 2) == (j % 2);
    }
  }
}


void rumble() {
  NeoESP32.vibrateMotors(rumble_frames, NeoESP32.max_frames_per_bt_package());
  delay((NeoESP32.max_frames_per_bt_package()) * NeoESP32.firmware_frame_duration());
}



void onConnected(bool success) {
  if (!success) {
    Serial.println("Attempted connection but failed.");
    return;
  }
  Serial.println("Successfully connected to Neosensory Buzz...");
  NeoESP32.authorizeDeveloper();
  NeoESP32.acceptTermsAndConditions();
  NeoESP32.stopAlgorithm();


}

void onDisconnected() {
  Serial.println("onDisconnected callback");
}

void onReadNotify(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
  for (int i = 0; i < length; i++) {
    Serial.write(pData[i]);
  }
}