#include <NewPing.h>
#include <Homie.h>

#define PING_DELAY_MS       1000
#define PING_AVERAGE_COUNT  5

const int relayPin =        D1;           // Relay shield
const int echoPin =         D8;
const int triggerPin =      D7;
const int max_distance_cm = 300;
const long interval_ms =    1000;

unsigned long timer;
bool active = false;

HomieNode garageNode("relay", "button");
NewPing sonar(triggerPin, echoPin, max_distance_cm);

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

long pingDistance() {
  // wait between pings - 29ms should be the shortest delay between pings
  delay(PING_DELAY_MS);
  
  // send ping, get ping time in microseconds (uS)
  unsigned int uS = sonar.ping();
  long cm = uS / US_ROUNDTRIP_CM;
  // if no distance is read, set at max distance
  if (cm == 0) cm = max_distance_cm;
  return cm;
} 


void loopHandler() {
  // measure the distance (in cm) - defaults to MAX_DISTANCE_CM if no reading
  int current = pingDistance();
  Homie.getLogger() << "Measured distance: " << current << endl;
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;

  pinMode(relayPin, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  timer = millis();

  Homie_setFirmware("garage-door-mqtt", "1.0.0"); // The underscore is not a typo! See Magic bytes
  
  garageNode.advertise("TOGGLE").settable(relayHandler);
  Homie.setLoopFunction(loopHandler);
  
  Homie.setup();
}

void loop() {
  Homie.loop();

  // deactive button after interval
  if (active && millis() - timer >= interval_ms) {
    Homie.getLogger() << "Deactivate relais" << endl;
    digitalWrite(relayPin, LOW);  // turn off relay with voltage LOW
    active = false;
  }
}
