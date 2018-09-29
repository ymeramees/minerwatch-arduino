char receivedChar;
char lastChar;
bool newData = false;
int heartBeatTime = 300;  //Timeout before restarting main miner after last heartbeat signal, in seconds
int lastHeartBeat = 0;  //Time from last heartbeat signal, in seconds
int restartDelay = 500; //Time how long reset button will be pressed, in milliseconds

#define juhtKaevur 0

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(juhtKaevur, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  // initialize timer1 
  //noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 15625;            // compare match register 16MHz/256/1Hz
  TCCR1B |= (1 << WGM12);   // CTC mode
  //TCCR1B |= (1 << CS12);    // 256 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  // set prescaler to 1024 and start the timer
  //TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  //interrupts();             // enable all interrupts

  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(9600);
  Serial.println("MinerWatch ver. 0.1");
}

void loop() {
  // put your main code here, to run repeatedly:
  readSerial();
  if(receivedChar == 'e') {
    Serial.println("Sain, elus");
    lastHeartBeat = 0;  //Reset timer
    lastChar = 0;
    receivedChar = 0;
    newData = false;
  }else if(lastChar == 'r') {
    restart(receivedChar - 48);
    lastChar = 0;
    receivedChar = 0;
    newData = false;
  }

  if((TIFR1 & (1 << OCF1A))) {
    lastHeartBeat++;
    if(lastHeartBeat >= heartBeatTime) {  //No signal from main miner, do restart
      restart(juhtKaevur);
      lastHeartBeat = 0;
    }
    if(digitalRead(LED_BUILTIN)) {
      digitalWrite(LED_BUILTIN, LOW);
    }else {
      digitalWrite(LED_BUILTIN, HIGH);
    }
    TIFR1 |= (1 << OCF1A); // reset the overflow flag
  }
}

void readSerial() {
  if (Serial.available() > 0) {
    if(newData) {
      lastChar = receivedChar;
    }
    if(lastChar == 'a'){  //New timeout before restarting main miner
      heartBeatTime = Serial.parseInt();
      lastChar = 0;
      receivedChar = 0;
      newData = false;
    }if(lastChar == 'd'){ //New time for pressing reset button, in milliseconds
      restartDelay = Serial.parseInt();
      lastChar = 0;
      receivedChar = 0;
      newData = false;
    }else{
      receivedChar = Serial.read();
      newData = true;
    }
 }
}

void restart(int no) {
  digitalWrite(no, HIGH); //Reset button down
  delay(restartDelay);
  digitalWrite(no, LOW);  //Release reset button
  
  Serial.print(no);
  Serial.println(", restart");
}

