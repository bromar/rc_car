#define echoPin 10 // Echo Pin
#define trigPin 11 // Trigger Pin
#define LEDPin 13 // Onboard LED

int maximumRange = 255; // Maximum range needed
int minimumRange = 0; // Minimum range needed
long duration; 
unsigned char distance; // Duration used to calculate distance

void setup() {
  Serial.begin (9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LEDPin, OUTPUT); // Use LED indicator (if required)
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
}

void loop() {
  /* The following trigPin/echoPin cycle is used to determine the
  distance of the nearest object by bouncing soundwaves off of it. */ 
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2); 

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); 
 
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
 
  //Calculate the distance (in cm) based on the speed of sound.
  distance = duration/58.2;
 
  if (distance >= maximumRange || distance <= minimumRange)
  {
      /* Send a negative number to computer and Turn LED ON 
      to indicate "out of range" */
      Serial.println("-1");
      digitalWrite(LEDPin, HIGH); 
  }
  else 
  {
      /* Send the distance to the computer using Serial protocol, and
      turn LED OFF to indicate successful reading. */
      Serial.println(distance);
      digitalWrite(LEDPin, LOW); 
  }
  
  //Send distance value to AVR
  digitalWrite(2, bitRead(distance, 0));
  digitalWrite(3, bitRead(distance, 1));
  digitalWrite(4, bitRead(distance, 2));
  digitalWrite(5, bitRead(distance, 3));
  digitalWrite(6, bitRead(distance, 4));
  digitalWrite(7, bitRead(distance, 5));
  digitalWrite(8, bitRead(distance, 6));
  digitalWrite(9, bitRead(distance, 7));
 
  //Delay 50ms before next reading.
  delay(50);
}
