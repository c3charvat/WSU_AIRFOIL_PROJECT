// #include <STM32FreeRTOS.h>
// #include <Arduino.h>

// TaskHandle_t Handle_aTask;
// TaskHandle_t Handle_bTask;

// static void ThreadA(void* pvParameters) {
//   Serial.println("Thread A: Started");

//   while (1) {
//       Serial.println("Hello World!");
//       delay(1000);
//   }
// }

// static void ThreadB(void* pvParameters) {
//   Serial.println("Thread B: Started");
//   while(1){
//     for (int i = 0; i < 10; i++) {
//         Serial.println("---This is Thread B---");
//         delay(2000);
//     }
//   }
// }

// void setup() {

//   Serial.begin(115200);
//   delay(1000);

//   while (!Serial) {
//   delay(.5); // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
//   }

//   Serial.println("");
//   Serial.println("******************************");
//   Serial.println("        Program start         ");
//   Serial.println("******************************");

//   // Create the threads that will be managed by the rtos
//   // Sets the stack size and priority of each task
//   // Also initializes a handler pointer to each task, which are important to communicate with and retrieve info from tasks
//   xTaskCreate(ThreadA,     "Task A",       256, NULL, tskIDLE_PRIORITY + 1, &Handle_aTask);
//   xTaskCreate(ThreadB,     "Task B",       256, NULL, tskIDLE_PRIORITY + 1, &Handle_bTask);

//   // Start the RTOS, this function will never return and will schedule the tasks.
//   vTaskStartScheduler();
// }

// void loop() {
//     // NOTHING
//}
#include <Arduino.h>
#include <STM32FreeRTOS.h>
#include <Seeed_Arduino_ooFreeRTOS.h>
#include "task.h"
#include "thread.hpp"
#include "ticks.hpp"
#include "queue.hpp"

// #include "freertos-addons.h"

using namespace cpp_freertos;

class SerialInputThread : public Thread
{

public:
    SerialInputThread(int i, Queue &q, Mutex &lock)
        : Thread("SerialInputThread", 256, 1),
          Id(i),
          MessageQueue(q),
          Lock(lock)
    {
        Start();
    };

protected:
    virtual void Run()
    {
        // Serial input thread setup
        Serial.print("Starting SerialInputThread ");
        Serial.println(Id);
        int Message = 0;
        // install shell
        while (true)
        {
            Serial.print("producer DelayInSeconds: ");
            Serial.println(DelayInSeconds);
            Delay(Ticks::SecondsToTicks(DelayInSeconds));
            for (int i = 0; i < BurstAmount; i++)
            {
                Lock.Lock();
                Serial.print("[P ");
                Serial.print(Id);
                Serial.print("] Sending  Message: ");
                Serial.println(Message);
                Lock.Unlock();

                MessageQueue.Enqueue(&Message);
                Message++;
            }
        }
    };

private:
    int Id;
    Queue &MessageQueue;
    Mutex &Lock;
};

class SerialOutputThread : public Thread
{

public:
    SerialOutputThread(int i, int delayInSeconds, Queue &q, Mutex &lock)
        : Thread("SerialOutputThread", 256, 2),
          Id(i),
          DelayInSeconds(delayInSeconds),
          MessageQueue(q),
          Lock(lock)
    {
        Start();
    };

protected:
    virtual void Run()
    {

        Serial.print("Starting SerialOutputThread ");
        Serial.println(Id);
        int Message;

        while (true)
        {

            Serial.print("consumer DelayInSeconds: ");
            Serial.println(DelayInSeconds);
            Delay(Ticks::SecondsToTicks(DelayInSeconds));

            MessageQueue.Dequeue(&Message);
            LockGuard guard(Lock);
            Serial.print("[C ");
            Serial.print(Id);
            Serial.print("] Received Message: ");
            Serial.println(Message);
            // guard.~LockGuard();   // automatic unlock, not needed
        }
    };

private:
    int Id;
    int DelayInSeconds;
    Queue &MessageQueue;
    Mutex &Lock;
};

// RTOS Ojects
Queue *gLuaQueue; // Queue between shell and lua containing lua code to be ran
Queue *gMessageQueue; // TODO break this out into Input/output queue
MutexStandard *gSerialMutex;

void setup(void)
{

    // start up the serial interface
    Serial.begin(115200);
    Serial.println("started");

    Serial.println("");
    Serial.println("******************************");
    Serial.println("        Program init          ");
    Serial.println("******************************");

    // Queues
    gMessageQueue = new Queue(5, sizeof(int));

    //  Mutex / Semaphores
    gSerialMutex = new MutexStandard();

    // Global Class instances
    SerialInputThread *p1;
    SerialOutputThread *p2;

    p1 = new SerialInputThread(1, *gMessageQueue, *gSerialMutex);
    p2 = new SerialOutputThread(2, 1, *gMessageQueue, *gSerialMutex);

    // TODO:
    //  Shell Task
    //  Lua Task
    //  Lcd Task
    //  Encoder Task?

    //  static SerialInputThread p1(10, 1, 10, *gMessageQueue,*gSerialMutex);
    //  static SerialOutputThread c1(20, 1, *gMessageQueue,*gSerialMutex);

    Serial.println("");
    Serial.println("******************************");
    Serial.println("        Program Start         ");
    Serial.println("******************************");
    delay(1000); // make sure the dust has settled
    Thread::StartScheduler();

    //
    //  We shouldn't ever get here unless someone calls
    //  Thread::EndScheduler()
    //

    Serial.println("Fatal Eror: Scheduler ended - Restart");
}

void loop()
{
    // Empty. Things are done in Tasks.
}

/* For As long As the Octopus Board is used under no circustances should this ever be modified !!!*/
/*
This section of code determines how the system clock is cofigured this is important for the
STM32F446ZET6 in this case our board runs at 168 MHz not the 8Mhz external clock the board expects by default
No need to understand, attempt to or even try to.
Include it in every version that is compiled form this platfrom IO envrioment/stm32dunio envrioment
Underclocked to 168mhz for stability?
*/

extern "C" void SystemClock_Config(void)
{
// #ifdef OCTOPUS_BOARD
#ifdef OCTOPUS_BOARD_FROM_HSI
    /* boot from HSI, internal 16MHz RC, to 168MHz. **NO USB POSSIBLE**, needs HSE! */
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 168;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    RCC_OscInitStruct.PLL.PLLR = 3;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }
#else
    /* boot from HSE, crystal oscillator (12MHz) */
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 6;
    RCC_OscInitStruct.PLL.PLLN = 168;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    RCC_OscInitStruct.PLL.PLLR = 3;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
    PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLQ;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
#endif
    // #else
    //   /* nucleo board, 8MHz external clock input, HSE in bypass mode */
    //   RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    //   RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    //   RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    //   /** Configure the main internal regulator output voltage
    //    */
    //   __HAL_RCC_PWR_CLK_ENABLE();
    //   __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    //   /** Initializes the RCC Oscillators according to the specified parameters
    //    * in the RCC_OscInitTypeDef structure.
    //    */
    //   RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    //   RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    //   RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    //   RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    //   RCC_OscInitStruct.PLL.PLLM = 4;
    //   RCC_OscInitStruct.PLL.PLLN = 168;
    //   RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    //   RCC_OscInitStruct.PLL.PLLQ = 7;
    //   RCC_OscInitStruct.PLL.PLLR = 2;
    //   if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    //   {
    //     Error_Handler();
    //   }
    //   /** Initializes the CPU, AHB and APB buses clocks
    //    */
    //   RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    //   RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    //   RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    //   RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    //   RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    //   if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    //   {
    //     Error_Handler();
    //   }
    //   PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
    //   PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLQ;
    //   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    //   {
    //     Error_Handler();
    //   }
    // #endif
}