#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'*','7','4','1'},
  {'0','8','5','2'},
  {'#','9','6','3'},
  {'D','C','B','A'}
};

// SAME pins as you used
byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {32, 33, 25, 26};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(115200);
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    Serial.print("Pressed: ");
    Serial.println(key);
  }
}