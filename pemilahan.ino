#include <Servo.h>

//Setting PIN
int IRSensorLogam = 15; // Sensor inframerah pemilah logam
int sensorlogam = 12; // PROXIMITY sensor

Servo servoLogam;

void setup() {
  pinMode(IRSensorLogam, INPUT); // IR Sensor pin INPUT
  Serial.begin(9600); // Init Serial at 115200 Baud Rate.
  servoLogam.attach(13);
  Serial.println("Serial Working"); // Test to check if serial is working or not

}

void loop() {
  int IMLogam = digitalRead(IRSensorLogam); // Set the GPIO as Input infrared IMLogam(Infra Merah Logam)
  if (IMLogam != 1) // Check if the pin high or not
  {
    delay(500);
    if (digitalRead(sensorlogam) == LOW ) {
      Serial.println("Terdeteksi Sampah Logam"); // print Motion Detected! on the serial monitor window
      for (int posDegrees = 90; posDegrees <= 180; posDegrees++) {
        servoLogam.write(posDegrees);
        Serial.println(posDegrees);
        delay(5);
      }

      for (int posDegrees = 180; posDegrees >= 90; posDegrees--) {
        servoLogam.write(posDegrees);
        Serial.println(posDegrees);
        delay(5);
      }
    }
    else if (digitalRead(sensorlogam) == HIGH ) {
      Serial.println("Terdeteksi Bukan Sampah Logam");
      for (int posDegrees = 90; posDegrees >= 0; posDegrees--) {
        servoLogam.write(posDegrees);
        Serial.println(posDegrees);
        delay(5);
      }
      for (int posDegrees = 0; posDegrees <= 90; posDegrees++) {
        servoLogam.write(posDegrees);
        Serial.println(posDegrees);
        delay(5);
      }
    }
  }
}
