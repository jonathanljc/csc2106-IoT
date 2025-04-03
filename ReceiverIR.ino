#include <M5StickCPlus.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

// Define the IR receiver pin and the expected NEC code.
#define IR_RECV_PIN 26
#define IR_EXPECTED_CODE 0x20DF10EF

// Define the relay control pin and the delay for how long the relay stays on.
#define RELAY_PIN 25
#define RELAY_ON_MS 10000  // 10 seconds
bool fan_status=false;

IRrecv irrecv(IR_RECV_PIN);
decode_results results;

void setup() {
  // Initialize M5StickC Plus hardware
  M5.begin();
  Serial.begin(115200);
  delay(1000);               // Allow time for initialization
  
  // Configure relay pin as output, and start with the relay off.
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // Begin listening for IR signals
  irrecv.enableIRIn();
  Serial.println("Ready to receive IR signals...");
}

void loop() {
  // Update M5StickC Plus (handles button events, etc.)
  M5.update();
  
  // Check if we have decoded an IR signal
  if (irrecv.decode(&results)) {
    // Print the human-readable decoded result to the serial monitor
    Serial.println(resultToHumanReadableBasic(&results));
    
    // Check if the decoded value matches our expected NEC code
    if (results.value == IR_EXPECTED_CODE) {
      if(fan_status==false){
        Serial.println("Matching IR signal detected! Turning relay ON...");
        digitalWrite(RELAY_PIN, HIGH);
        fan_status = true;
      }else{
      Serial.println("Matching IR signal detected! Turning relay OFF...");
      digitalWrite(RELAY_PIN, LOW);  // Turn the relay on
        fan_status = false;
      }
      
      
    }
    
    // Prepare for the next IR signal
    irrecv.resume();
  }
}
