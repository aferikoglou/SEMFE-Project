/*CONSTANTS*/
const int interrupt_pin = 3;

const int S0_pin = 4;
const int S1_pin = 5;
const int S2_pin = 6;
const int OE_bar_pin = 22;

const int num_of_sensors = 8;

const int baud_rate = 9600;

/*GLOBAL VARIABLES*/
int number_of_samples = 10;

int pulse_counter = 0;
byte measurement_counter = 0;

unsigned long overflows = 0;
unsigned long ticks = 0;
double period = 0;

boolean interrupts_enabled = false;
boolean measurement_finished = false;

int sensor_coord;

boolean full_mode = false;

void setup() {
  attachInterrupt(digitalPinToInterrupt(interrupt_pin), count_time_of_k_pulses, RISING);

  pinMode(OE_bar_pin, OUTPUT);
  pinMode(S0_pin, OUTPUT);
  pinMode(S1_pin, OUTPUT);
  pinMode(S2_pin, OUTPUT);

  TIMER1_init();

  Serial.begin(baud_rate);
  while (! Serial);
}

void loop() {
  if (Serial.available()) {
    /*DEFINE MEASUREMENT MODE*/
    String input = Serial.readString();
    if (input[0] == 'r' && interrupts_enabled == false) {
      if (input[1] == 'f') {
        full_mode = true;
        sensor_coord = 0;
      } else
        sensor_coord = input[2] - '0';

      select_sensor();

      /*SET VARIABLES FOR DUMMY MEASUREMENT*/
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

    if (measurement_counter == 1) {
      /*SET VARIABLES FOR MEASUREMENT*/
      number_of_samples = 100;

      pulse_counter = 0;

      TIMER1_clear();

      interrupts_enabled = true;
    }
    else {
      TIMER1_print_results();

      if (full_mode) {
        sensor_coord++;

        select_sensor();

        /*SET VARIABLES FOR DUMMY MEASUREMENT*/
        number_of_samples = 10;

        pulse_counter = 0;
        measurement_counter = 0;

        TIMER1_clear();

        interrupts_enabled = true;

        if (sensor_coord == num_of_sensors) {
          full_mode = false;
          interrupts_enabled = false;
        }
      }
    }
    measurement_finished = false;
  }
}

/*INTERRUPT ROUTINES*/

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

/*FUNCTION FOR SENSOR SELECTION*/

void select_sensor() {
  digitalWrite(OE_bar_pin, LOW);
  digitalWrite(S2_pin, ((sensor_coord & 4) == 4) ? HIGH : LOW );
  digitalWrite(S1_pin, ((sensor_coord & 2) == 2) ? HIGH : LOW );
  digitalWrite(S0_pin, ((sensor_coord & 1) == 1) ? HIGH : LOW );
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
  Serial.print("SENSOR COORDINATE         = ");
  Serial.println(sensor_coord);

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

  Serial.print("MEASURED RESISTANCE       = ");

  double resistanse = ((period * 3.551020408 * 10) / 4) - 90.7;
  Serial.print(resistanse);

  Serial.println(" kΩ");

  Serial.println("----------------------------------------");
}
