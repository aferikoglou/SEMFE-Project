const int interrupt_pin = 3;
const int baud_rate = 9600;

int number_of_samples = 10;

int pulse_counter = 0;
byte measurement_counter = 0;

unsigned long overflows = 0;
unsigned long ticks = 0;
double period = 0;

boolean interrupts_enabled = false;
boolean measurement_finished = false;

void setup() {
  attachInterrupt(digitalPinToInterrupt(interrupt_pin), count_time_of_k_pulses, RISING);

  TIMER1_init();

  Serial.begin(baud_rate);
  while (! Serial);
}

void loop() {
  if (Serial.available()) {
    char input = Serial.read();

    if (input == 'r' && interrupts_enabled == false) {
      number_of_samples = 10;
      
      pulse_counter = 0;
      measurement_counter = 0;
      
      TIMER1_clear();
      
      interrupts_enabled = true;
    }
    else if (input == 's')
      TIMER1_stop();
  }

  if (measurement_finished) {
    measurement_counter++;

    ticks = (overflows * 65536) + TCNT1;
    period = (ticks * 0.0625) / (number_of_samples - 1);

    if (measurement_counter == 1){
      
      //MODIFY NUMBER OF SAMPLES FOR NEXT MEASUREMENT
      if (period > 10000)
        number_of_samples = 20;
      else
        number_of_samples = 100;
      
      pulse_counter = 0;

      TIMER1_clear();

      interrupts_enabled = true;
    } 
    else 
      TIMER1_print_results();

    measurement_finished = false;
  }
}

void count_time_of_k_pulses() {
  if (interrupts_enabled) {
    pulse_counter++;

    if (pulse_counter == 1)
      TIMER1_start();

    if (pulse_counter == number_of_samples) {
      TIMER1_stop();

      measurement_finished = true;
      interrupts_enabled = false;
    }
  }
}

ISR (TIMER1_OVF_vect) {
  overflows++;
}

/*TIMER1 FUNCTIONS*/

void TIMER1_init() {
  TCCR1A  = 0;
  TCCR1B  = 0;
  TIMSK1  = 0;
  TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);
}

void TIMER1_start() {
  TIMSK1 |= (1 << TOIE1);
}

void TIMER1_stop() {
  TIMSK1 = 0;
}

void TIMER1_clear() {
  TCNT1     = 0;
  overflows = 0;
}

void TIMER1_print_results() {
  Serial.println("----------------------------------------");
  Serial.print("NUMBER OF PULSES MEASURED = ");
  Serial.println(pulse_counter);
  
  Serial.print("TIMER1 VALUE              = ");
  Serial.println(TCNT1);

  Serial.print("TIMER1 OVERFLOWS          = ");
  Serial.println(overflows);

  Serial.print("TICKS                     = ");
  Serial.println(ticks);

  Serial.print("MEASURED PERIOD           = ");
  Serial.print(period);
  Serial.println(" US");
  Serial.println("----------------------------------------");
}
