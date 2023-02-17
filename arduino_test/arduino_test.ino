void setup() {
  delay(500);
  Serial.begin(115200);
}

void loop() {
  Serial.println(analogRead(A3));
  delay(250);
}
