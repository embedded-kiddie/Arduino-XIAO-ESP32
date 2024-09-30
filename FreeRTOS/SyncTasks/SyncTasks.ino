#include <Arduino.h>

/*=============================================================
 * Step 1: Select whether to multitask or not
 *=============================================================*/
#define ENA_MULTITASKING  true

/*=============================================================
 * Step 2: Configure expected processing time
 *=============================================================*/
#define RANDOMIZE false

#if RANDOMIZE
#define PROCESS(x) delay(random(x))
#else
#define PROCESS(x) delay(x)
#endif

#define PROCESSING_TIME_INPUT   2000
#define PROCESSING_TIME_OUTPUT  1000

// Function prototype defined in multitasking.cpp
void task_setup(void (*task1)(uint8_t), void (*task2)(uint8_t, uint32_t, uint32_t));

void ProcessInput(uint8_t bank) {
  PROCESS(PROCESSING_TIME_INPUT);
}

void ProcessOutput(uint8_t bank, uint32_t inputStart, uint32_t inputFinish) {
  static uint32_t prevFinish;
  uint32_t outputStart = millis();

  PROCESS(PROCESSING_TIME_OUTPUT);

  uint32_t outputFinish = millis();

  Serial.printf("Input:  %d\nOutput: %d (%d)\nCycle:  %d\n",
    (inputFinish  - inputStart ),
    (outputFinish - outputStart), bank,
    (outputFinish - prevFinish )
  );

  prevFinish = outputFinish;
}

void setup() {
  Serial.begin(115200);

  // Start tasks
#if ENA_MULTITASKING
  void task_setup(void (*task1)(uint8_t), void (*task2)(uint8_t, uint32_t, uint32_t));
  task_setup(ProcessInput, ProcessOutput);
#endif
}

void loop() {
#if ENA_MULTITASKING
  delay(1000);
#else
  uint32_t inputStart = millis();
  ProcessInput(0);
  ProcessOutput(0, inputStart, millis());
#endif
}