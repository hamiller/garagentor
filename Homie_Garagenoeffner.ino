#include <Homie.h>

const long interval = 1000;        // 500ms 
const int relayPin = D1;           // Relay shield
unsigned long timer;
bool active = false;

HomieNode garageNode("relay", "button");


bool relayHandler(const HomieRange& range, const String& value) {
    Homie.getLogger() << "Received garage door command " << value << endl;
    if (value == "ON") {
        Homie.getLogger() << "Activate relais" << endl;
        digitalWrite(relayPin, HIGH); // turn on relay with voltage HIGH
        timer = millis();
        active = true;
    }
    return true;
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;

  pinMode(relayPin, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  timer = millis();

  Homie_setFirmware("garage-door-mqtt", "1.0.0"); // The underscore is not a typo! See Magic bytes
  garageNode.advertise("TOGGLE").settable(relayHandler);
  Homie.setup();
}

void loop() {
  Homie.loop();

  // deactive button after intervall
  if (active && millis() - timer >= interval) {
    Homie.getLogger() << "Deactivate relais" << endl;
    digitalWrite(relayPin, LOW);  // turn off relay with voltage LOW
    active = false;
  }
}
