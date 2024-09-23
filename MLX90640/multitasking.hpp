#include <Arduino.h>

#define CORE_TASK_INPUT   1
#define CORE_TASK_OUTPUT  1

// Define two tasks for reading and writing from and to the serial port.
void TaskInput(void *pvParameters);
void TaskOutput(void *pvParameters);

// Define Task and Queue handle
void (*taskInput)(uint8_t bank);
void (*taskOutput)(uint8_t bank);
TaskHandle_t taskHandle[2];
SemaphoreHandle_t semHandle;
QueueHandle_t queHandle;
typedef struct {
  uint8_t bank;
} MessageQueue_t;

#define HALT()  {for(;;) delay(1000);}

// The setup function runs once when you press reset or power on the board.
void task_setup(void (*task1)(uint8_t), void (*task2)(uint8_t)) {

  taskInput  = task1;
  taskOutput = task2;

  // To process tasks in parallel, the semaphore must have an initial count of 1.
  semHandle = xSemaphoreCreateCounting(1, CORE_TASK_INPUT != CORE_TASK_OUTPUT ? 1 : 0);
  queHandle = xQueueCreate(1, sizeof(MessageQueue_t));

  // Check if the queue was successfully created
  if (queHandle == NULL || semHandle == NULL) {
    Serial.println("Can't create queue or semaphore.");
    HALT();
  }

  // Set up sender task in core 1
  xTaskCreatePinnedToCore(
    TaskInput, "TaskInput",
    8192,           // The stack size
    NULL,           // Pass reference to a variable describing the task number
    1,              // priority
    &taskHandle[0]  // Task handle is not used here - simply pass NULL
    , CORE_TASK_OUTPUT
  );

  // Set up receiver task on core 0 and start immediately
  xTaskCreatePinnedToCore(
    TaskOutput, "TaskOutput",
    8192,           // The stack size
    NULL,           // Pass reference to a variable describing the task number
    1,              // priority
    &taskHandle[1]  // Task handle is not used here - simply pass NULL
    , CORE_TASK_OUTPUT
  );
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/
uint8_t GiveQueue(uint8_t bank) {
  MessageQueue_t queue = { bank };

  if (xQueueSend(queHandle, &queue, portMAX_DELAY) == pdTRUE) {
//  Serial.println("Give: " + String(queue.bank));
  } else {
    Serial.println("unable to send queue");
  }

  return CORE_TASK_INPUT != CORE_TASK_OUTPUT ? !bank : bank;
}

MessageQueue_t TakeQueue() {
  static MessageQueue_t queue;

  if (xQueueReceive(queHandle, &queue, portMAX_DELAY) == pdTRUE) {
//  Serial.println("Take: " + String(queue.bank));
  } else {
    Serial.println("Unable to receive queue.");
  }

  return queue;
}

void TakeSemaphore(void) {
  if (xSemaphoreTake(semHandle, portMAX_DELAY) == pdTRUE) {
//  Serial.println("Take semaphore.");
  } else {
    Serial.println("Unable to take semaphore.");
  }
}

void GiveSemaphore(void) {
  if (xSemaphoreGive(semHandle) == pdTRUE) {
    //Serial.println("Give semaphore.");
  } else {
    Serial.println("Unable to give semaphore.");
  }
}

void TaskInput(void *pvParameters) {
  uint8_t bank = 0;

  while (true) {
    // some process
    taskInput(bank);

    bank = GiveQueue(bank);

    TakeSemaphore();
  }
}

void TaskOutput(void *pvParameters) {
  while (true) {
    MessageQueue_t queue = TakeQueue();

    // some process
    taskOutput(queue.bank);

    GiveSemaphore();
  }
}