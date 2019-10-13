/***** CONSTANTS *****/
const int BAUD_RATE = 9600;

const int INTERRUPT_PIN = 3;

const int S0_PIN = 4;
const int S1_PIN = 5;
const int S2_PIN = 6;
const int OE_BAR_PIN = 22;

const int NUM_OF_SENSORS = 8;

/***** ANALOG CIRCUIT CONSTANTS *****/
const double CAPACITOR_VALUE = 400; //pF
const double GAIN_VALUE = 3.5222672065;
const double R_BIAS_VALUE = 98.7; //kΩ

/***** GLOBAL VARIABLES *****/
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
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), count_time_of_k_pulses, RISING);

  pinMode(OE_BAR_PIN, OUTPUT);
  pinMode(S0_PIN, OUTPUT);
  pinMode(S1_PIN, OUTPUT);
  pinMode(S2_PIN, OUTPUT);

  TIMER1_init();

  Serial.begin(BAUD_RATE);
  while (! Serial);
}

void loop() {
  if (Serial.available()) {
    /***** DEFINE MEASUREMENT MODE *****/
    String input = Serial.readString();
    if (input[0] == 'r' && interrupts_enabled == false) {
      if (input[1] == 'f') {
        full_mode = true;
        sensor_coord = 0;
      } else
        sensor_coord = input[2] - '0';

      select_sensor();

      /***** SET VARIABLES FOR DUMMY MEASUREMENT *****/
      number_of_samples = 10;

      pulse_counter = 0;
      measurement_counter = 0;

      TIMER1_clear();

      interrupts_enabled = true;
    }
  }

  if (measurement_finished) {
    measurement_counter++;

    ticks = (overflows * 65536) + TCNT1;
    period = (ticks * 0.0625) / (number_of_samples - 1);

    if (measurement_counter == 1) {
      /***** SET VARIABLES FOR MEASUREMENT *****/
      number_of_samples = 500;

      pulse_counter = 0;

      TIMER1_clear();

      interrupts_enabled = true;
    }
    else {
      TIMER1_print_results();

      if (full_mode) {
        sensor_coord++;

        select_sensor();

        /***** SET VARIABLES FOR DUMMY MEASUREMENT *****/
        number_of_samples = 10;

        pulse_counter = 0;
        measurement_counter = 0;

        TIMER1_clear();

        interrupts_enabled = true;

        if (sensor_coord == NUM_OF_SENSORS) {
          full_mode = false;
          interrupts_enabled = false;
        }
      }
    }
    measurement_finished = false;
  }
}

/***** INTERRUPT ROUTINES *****/

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

/***** FUNCTION FOR SENSOR SELECTION *****/

void select_sensor() {
  digitalWrite(OE_BAR_PIN, LOW);
  digitalWrite(S2_PIN, ((sensor_coord & 4) == 4) ? HIGH : LOW );
  digitalWrite(S1_PIN, ((sensor_coord & 2) == 2) ? HIGH : LOW );
  digitalWrite(S0_PIN, ((sensor_coord & 1) == 1) ? HIGH : LOW );
}

/***** TIMER1 FUNCTIONS *****/

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

  double resistanse = ((period * GAIN_VALUE * 1000) / (4 * CAPACITOR_VALUE)) - R_BIAS_VALUE; //kΩ
  Serial.print(resistanse);

  Serial.println(" kΩ");

  Serial.println("----------------------------------------");
}
