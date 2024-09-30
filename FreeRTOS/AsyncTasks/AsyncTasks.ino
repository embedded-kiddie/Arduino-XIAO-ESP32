/* Basic Multi Threading Arduino Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
// Please read file README.md in the folder containing this example.
#include <Arduino.h>

#define RANDOMIZE false

#if RANDOMIZE
#define PROCESS(x) delay(random(x))
#else
#define PROCESS(x) delay(x)
#endif

#define PERIOD_SENDER   1000
#define PERIOD_RECEIVER 2000

#define CORE_SENDER   1
#define CORE_RECEIVER 0
#define NUM_BANKS     3

// Shared resources that should be exclusively controlled
typedef struct {
  uint8_t   bank;
  bool      flag[NUM_BANKS];
  uint32_t  time[NUM_BANKS];
} MultiBuffer_t;
MultiBuffer_t buffer;

TaskHandle_t taskHandle[2];
SemaphoreHandle_t semHandle;

// Define a task function
void OneCharSender(void *pvParameters);
void OneCharReceiver(void *pvParameters);

// The setup function runs once when you press reset or power on the board.
void setup() {
  // Initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
  while (!Serial) { delay(1000); }

  semHandle = xSemaphoreCreateMutex();  // Create the mutex

  if (semHandle == NULL) {
    Serial.println("Can't create semaphore.");
    while (true) { delay(1000); }
  }

  // Set up sender task in core 1
  xTaskCreatePinnedToCore(
    OneCharSender, "OneCharSender",
    8192,           // The stack size
    NULL,           // Pass reference to a variable describing the task number
    2,              // priority
    &taskHandle[0]  // Task handle is not used here - simply pass NULL
    , CORE_RECEIVER
  );

  // Temporarily suspend the task on core 1
  vTaskSuspend(taskHandle[0]);

  // Set up receiver task on core 0 and start immediately
  Serial.println("Start receiver");
  xTaskCreatePinnedToCore(
    OneCharReceiver, "OneCharReceiver",
    8192,           // The stack size
    NULL,           // Pass reference to a variable describing the task number
    1,              // priority
    &taskHandle[1]  // Task handle is not used here - simply pass NULL
    , CORE_RECEIVER
  );

  // Now start sender task on core 1
  Serial.println("Start sender");
  vTaskResume(taskHandle[0]);
}

void loop() {
  // Loop is free to do any other work
  delay(1000);
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/
void OneCharSender(void *pvParameters) {
  uint8_t bank = 0;

  while (true) {
    // some process
    PROCESS(PERIOD_SENDER);

    if (xSemaphoreTake(semHandle, portMAX_DELAY) == pdTRUE) {
      // Give the latest result
      buffer.bank = bank;
      buffer.flag[bank] = true;
      buffer.time[bank] = millis();

      Serial.println("S: bank = " + String(bank));

      // Take for the next
      bank = (bank + 1) % NUM_BANKS;
      if (buffer.flag[bank] == true) {  // Even if one is not free, 
        bank = (bank + 1) % NUM_BANKS;  // the other may always be free.
      }
      buffer.flag[bank] = false;

      // Granting access
      xSemaphoreGive(semHandle);
    }
  }
}

void OneCharReceiver(void *pvParameters) {
  uint8_t bank = 1;
  uint32_t time = millis();

  while (true) {
    // Try to take the mutex and wait indefinitely if needed
    if (xSemaphoreTake(semHandle, portMAX_DELAY) == pdTRUE) {
      // Release the previous bank
      buffer.flag[bank] = false;

      // Take the latest bank
      uint8_t received;
      bank = buffer.bank;

      // Check the bank is updated
      if (buffer.flag[bank] == true) {
        uint32_t t = millis();
        Serial.println("R: bank = " + String(bank) + " (cycle: " + String(t - time) + ", delay: " + String(t - buffer.time[bank]) + ")");
        time = t;
        received = true;
      } else {
        received = false;
        //Serial.println("Receiver: empty bank " + String(bank));
      }

      // Granting access
      xSemaphoreGive(semHandle);  // After accessing the shared resource give the mutex and allow other processes to access it

      // some process
      PROCESS(received ? PERIOD_RECEIVER : 100);
    }
  }
}