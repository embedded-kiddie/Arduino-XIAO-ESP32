/*================================================================================
 * Multitasking helper functions
 *================================================================================*/
#include <Arduino.h>

#define TASK1_CORE      1
#define TASK2_CORE      0

#define TASK1_STACK     4096
#define TASK2_STACK     8192

#define TASK1_PRIORITY  2
#define TASK2_PRIORITY  1

// Message queue sent from task 1 to task 2
typedef struct {
  uint8_t   bank;   // Exclusive bank numbers for Task 1 and Task 2
  uint32_t  start;  // Task 1 start time
  uint32_t  finish; // Task 1 Finish Time
} MessageQueue_t;

// Define two tasks on the core
void Task1(void *pvParameters);
void Task2(void *pvParameters);

// Define pointers to the tasks
static void (*Process1)(uint8_t bank);
static void (*Process2)(uint8_t bank, uint32_t start, uint32_t finish);

// Message queues and semaphores for handshaking
static TaskHandle_t taskHandle[2];
static QueueHandle_t queHandle;
static SemaphoreHandle_t semHandle;

#define HALT()  { for(;;) delay(1000); }

// The setup function runs once when press reset or power on the board
void task_setup(void (*task1)(uint8_t), void (*task2)(uint8_t, uint32_t, uint32_t)) {
  // Pointers to the tasks to be executed.
  Process1 = task1;
  Process2 = task2;

  // To process tasks in parallel, the semaphore must have an initial count of 1
  semHandle = xSemaphoreCreateCounting(1, TASK1_CORE != TASK2_CORE ? 1 : 0);
  queHandle = xQueueCreate(1, sizeof(MessageQueue_t));

  // Check if the queue or the semaphore was successfully created
  if (queHandle == NULL || semHandle == NULL) {
    Serial.println("Can't create queue or semaphore.");
    HALT();
  }

  // Set up sender task in core 1 and start immediately
  xTaskCreatePinnedToCore(
    Task1, "Task1",
    TASK1_STACK,    // The stack size
    NULL,           // Pass reference to a variable describing the task number
    TASK1_PRIORITY, // priority
    &taskHandle[0], // Pass reference to task handle
    TASK1_CORE
  );

  // Set up receiver task on core 0 and start immediately
  xTaskCreatePinnedToCore(
    Task2, "Task2",
    TASK2_STACK,    // The stack size
    NULL,           // Pass reference to a variable describing the task number
    TASK2_PRIORITY, // priority
    &taskHandle[1], // Pass reference to task handle
    TASK2_CORE
  );
}

/*--------------------------------------------------*/
/*------------------- Handshake --------------------*/
/*--------------------------------------------------*/
uint8_t SendQueue(uint8_t bank, uint32_t start, uint32_t finish) {
  MessageQueue_t queue = {
    bank, start, finish
  };

  if (xQueueSend(queHandle, &queue, portMAX_DELAY) == pdTRUE) {
//  Serial.println("Give queue: " + String(queue.bank));
  } else {
    Serial.println("unable to send queue");
  }

  return !bank;
}

MessageQueue_t ReceiveQueue() {
  MessageQueue_t queue;

  if (xQueueReceive(queHandle, &queue, portMAX_DELAY) == pdTRUE) {
//  Serial.println("Take queue: " + String(queue.bank));
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
//  Serial.println("Give semaphore.");
  } else {
    Serial.println("Unable to give semaphore.");
  }
}

/*--------------------------------------------------*/
/*--------------------- Tasks ----------------------*/
/*--------------------------------------------------*/
void Task1(void *pvParameters) {
  uint8_t bank = 0;

  while (true) {
    uint32_t start = millis();

    // some process
    Process1(bank);

//  Serial.println(millis() - start);

    bank = SendQueue(bank, start, millis());

    TakeSemaphore();
  }
}

void Task2(void *pvParameters) {
  while (true) {
    MessageQueue_t queue = ReceiveQueue();

    GiveSemaphore();

    // some process
    Process2(queue.bank, queue.start, queue.finish);
  }
}