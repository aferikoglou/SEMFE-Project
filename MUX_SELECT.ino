const int X_SIZE = 2;
const int Y_SIZE = 2;

int XPOS, YPOS;

String input_string;
const int X3 = 7, X2 = 6, X1 = 5;        //X-axis - X-MUX - Select Row
const int Y3 = 13, Y2 = 12, Y1 = 11;        //Y-axis - Y-MUX - Select Column
const int OE_bar = 4;

void setup() {
  Serial.begin(9600);
  
  while (! Serial);

  //Define I/O pins
  pinMode(OE_bar, OUTPUT);
  //Selectors
  pinMode(X3, OUTPUT); 
  pinMode(X2, OUTPUT);
  pinMode(X1, OUTPUT);
  pinMode(Y3, OUTPUT);
  pinMode(Y2, OUTPUT);
  pinMode(Y1, OUTPUT);

  //Initialization
  digitalWrite(OE_bar, LOW);
  digitalWrite(X3, LOW);
  digitalWrite(X2, LOW);
  digitalWrite(X1, LOW);
  digitalWrite(Y3, LOW);
  digitalWrite(Y2, LOW);
  digitalWrite(Y1, LOW);
}

/*
  TEST MODES:
  i)   RF          : run full test
  ii)  R(5,8)     : runs test on sensor (5,8)
  iii) S           : stops
*/
void loop() {
  if (Serial.available()) {
    input_string = Serial.readString();

    if (input_string[0] == 'R' && input_string[1] == 'F') {
      for (int i = 0; i < X_SIZE; i++) {
        for (int j = 0; j < Y_SIZE; j++) {
          
          /* Select sensor */
          digitalWrite(X3, ((i & 4) == 4) ? HIGH : LOW );
          digitalWrite(X2, ((i & 2) == 2) ? HIGH : LOW );
          digitalWrite(X1, ((i & 1) == 1) ? HIGH : LOW );

          digitalWrite(Y3, ((j & 4) == 4) ? HIGH : LOW );
          digitalWrite(Y2, ((j & 2) == 2) ? HIGH : LOW );
          digitalWrite(Y1, ((j & 1) == 1) ? HIGH : LOW );

          delay(1000);
        }
      }
    }
    else if (input_string[0] == 'R') {
      XPOS = input_string[2] - '0';
      YPOS = input_string[4] - '0';

      /* Select sensor */
      digitalWrite(X3, ((XPOS & 4) == 4) ? HIGH : LOW );
      digitalWrite(X2, ((XPOS & 2) == 2) ? HIGH : LOW );
      digitalWrite(X1, ((XPOS & 1) == 1) ? HIGH : LOW );

      digitalWrite(Y3, ((YPOS & 4) == 4) ? HIGH : LOW );
      digitalWrite(Y2, ((YPOS & 2) == 2) ? HIGH : LOW );
      digitalWrite(Y1, ((YPOS & 1) == 1) ? HIGH : LOW );

      delay(1000);
    }
  }
}

