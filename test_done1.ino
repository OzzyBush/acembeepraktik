#include <U8g2lib.h> // Include the U8g2 library for the OLED display

// Define the pairs of pins
const int pairs[5][2] = {
  {18, 26},   // L5 & L10
  {19, 14},   // L4 & L9
  {27, 13},    // L1 & L6
  {5, 12},   // L3 & L8
  {25, 4}     // L2 & L7
};

// Define custom names for each pair
const char* pairNames[5] = {
  "L1", 
  "L2", 
  "L3",  
  "N",  
  "Gnd"  
};

// Define the red and green LED pins
const int red_led = 10;
const int green_led = 9;

// Create U8g2 object for the OLED display (I2C interface)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Function to test connectivity between a pair of pins
bool testConnection(int inputPin) {
  bool isConnected = digitalRead(inputPin);  // Check if the input pin is LOW (connected)
  
  return isConnected;
}

void setup() {

   for (int i = 0; i < 5; i++) {
    int outputPin = pairs[i][0];
    int inputPin = pairs[i][1];

    pinMode(outputPin, OUTPUT);         // Set the output pin as OUTPUT
    pinMode(inputPin, INPUT_PULLDOWN);

    digitalWrite(outputPin, LOW);
  }

  // Initialize the red and green LEDs as outputs
  pinMode(red_led, OUTPUT);
  pinMode(green_led, OUTPUT);

  // Initialize serial communication for debugging
  Serial.begin(9600);

  // Initialize the OLED display
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr); // Set a font
  u8g2.drawStr(0, 10, "Starting Test...");
  u8g2.sendBuffer();
  delay(1000); // Show the starting message for 1 second
}

void loop() {
  // Test each pair one at a time
  for (int i = 0; i < 5; i++) {
    int outputPin = pairs[i][0];
    int inputPin = pairs[i][1];
    digitalWrite(outputPin, HIGH);

    // Display the pair name on the OLED
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB12_tr); // Use a larger font for the pair name
    u8g2.drawStr(0, 20, pairNames[i]);  // Display the pair name
    u8g2.sendBuffer();

    // Test the connection
    if (testConnection(inputPin)) {
      Serial.print(pairNames[i]);  // Print the custom name
      Serial.println(" is connected.");
      digitalWrite(green_led, HIGH);  // Indicate success with green LED
      digitalWrite(red_led, LOW);

      // Display "Connected" on the OLED
      u8g2.setFont(u8g2_font_ncenB08_tr); // Use a smaller font for the status
      u8g2.drawStr(0, 40, "Connected");
      u8g2.sendBuffer();
    } else {
      Serial.print(pairNames[i]);  // Print the custom name
      Serial.println(" is NOT connected.");
      digitalWrite(red_led, HIGH);  // Indicate failure with red LED
      digitalWrite(green_led, LOW);

      // Display "Not Connected" on the OLED
      u8g2.setFont(u8g2_font_ncenB08_tr); // Use a smaller font for the status
      u8g2.drawStr(0, 40, "Not Connected");
      u8g2.sendBuffer();
      digitalWrite(outputPin, LOW);
    }

    delay(2000);  // Wait for 2 seconds before testing the next pair
  }
}
