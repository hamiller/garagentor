#include <NewPing.h>
#include <Homie.h>

#define PING_DELAY_MS                   1000
#define PING_AVERAGE_COUNT              5

const int relayPin =                    D1;           // Relay shield
const int cistern_echoPin =             D8;
const int cistern_triggerPin =          D7;
const int max_distance_cm =             220;
const int garageDoor_echoPin =          D6;
const int garageDoor_triggerPin =       D5;
const long relayInterval_ms =           700;
const long cisternInterval_ms =         60*1000;
const long doorCheckInterval_ms =       10*1000;

unsigned long relayTimer;
bool relayActive = false;
unsigned long cisternTimer;
unsigned long doorCheckTimer;

HomieNode garageNode("relay", "button");
HomieNode garageDoorNode("door", "sensor");
HomieNode cisternNode("cistern", "sensor");
NewPing cisternSonar(cistern_triggerPin, cistern_echoPin, max_distance_cm);
NewPing garageDoorSonar(garageDoor_triggerPin, garageDoor_echoPin, max_distance_cm);


bool relayHandler(const HomieRange& range, const String& value) {
    Homie.getLogger() << "Received garage door command " << value << endl;
    if (value == "ON") {
        Homie.getLogger() << "Activate relais" << endl;
        digitalWrite(relayPin, HIGH); // turn on relay with voltage HIGH
        relayTimer = millis();
        relayActive = true;
    }
    return true;
}

long pingDistance(NewPing sonar) {
  // wait between pings - 29ms should be the shortest delay between pings
  delay(PING_DELAY_MS);
  
  // send ping, get ping time in microseconds (uS)
  unsigned int uS = sonar.ping();
  long cm = uS / US_ROUNDTRIP_CM;
  // if no distance is read, set at max distance
  if (cm == 0) cm = max_distance_cm;
  return cm;
}

void measureCistern() {
  // measure the distance (in cm) - defaults to MAX_DISTANCE_CM if no reading
  int current = pingDistance(cisternSonar);
  Homie.getLogger() << "Measured distance in cistern: " << current << endl;
  cisternNode.setProperty("distance").send(String(current));
}

void checkGarageDoor() {
  // measure the distance (in cm) - defaults to MAX_DISTANCE_CM if no reading
  int current = pingDistance(garageDoorSonar);
  Homie.getLogger() << "Garage door distance: " << current << endl;
  if (current > 50) {
    Homie.getLogger() << "Garage door is open!" << endl;
    garageDoorNode.setProperty("open").send("true");
  }
  else {
    garageDoorNode.setProperty("open").send("false");
  }
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;

  pinMode(relayPin, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  long currentMs = millis();
  relayTimer = currentMs;
  cisternTimer = currentMs;
  doorCheckTimer = currentMs;

  Homie_setFirmware("garage-door-mqtt", "1.0.0"); // The underscore is not a typo! See Magic bytes
  
  garageNode.advertise("TOGGLE").settable(relayHandler);
  
  Homie.disableResetTrigger(); // so we may also use D1
  Homie.setup();
}



void loop() {
  Homie.loop();

  long currentMs = millis();
  
  // deactive relay after interval
  if (relayActive && currentMs - relayTimer >= relayInterval_ms) {
    Homie.getLogger() << "Deactivate relais" << endl;
    digitalWrite(relayPin, LOW);  // turn off relay with voltage LOW
    relayActive = false;
  }

  // measure cistern distance
  if (currentMs - cisternTimer >= cisternInterval_ms) {
    measureCistern();
    cisternTimer = millis();
  }

  // measure door distance
  if (currentMs - doorCheckTimer >= doorCheckInterval_ms) {
    checkGarageDoor();
    doorCheckTimer = millis();
  }
}
