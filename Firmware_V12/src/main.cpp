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
#include "message.hpp"

//#include "freertos-addons.h"

using namespace cpp_freertos;

class ProducerThread : public Thread {
  
public:
  
  ProducerThread(int i, int delayInSeconds, Message &m, Mutex &lock)
    : Thread("ProducerThread", 1024, 1), 
      Id (i), 
      DelayInSeconds(delayInSeconds),
      Mail(m),
      Lock(lock)
  {
            Start();
  };
  
protected:

  virtual void Run() {
    
    Serial.print("Starting ProducerThread ");
    Serial.println(Id);
    uint8_t Message[] = "asdasd";
    
    while (true) {
      Delay(Ticks::SecondsToTicks(DelayInSeconds));
      Lock.Lock();
      Serial.printf("Send: %s length: %d\n\r", Message, strlen((const char *)Message));
      Lock.Unlock();
      Mail.Send(Message, strlen((const char *)Message));
      }
  };

private:
  int Id;
  int DelayInSeconds;
  Message &Mail;
  Mutex& Lock;
};


class ConsumerThread : public Thread {
  
public:

  ConsumerThread(int i,  Message &m, Mutex &lock)
    : Thread("ConsumerThread", 1024, 2), 
      Id (i), 
      Mail(m),
      Lock(lock)
  {
    
    Start();
  };
  
protected:
  
  virtual void Run() {
    
    Serial.print("Starting ConsumerThread ");
    Serial.println(Id);
    size_t nums;
    while (true) {
      nums = Mail.Receive(&buff, 256);
      LockGuard guard(Lock);
      Serial.printf("Receive: ");
      for(int i = 0; i < nums; i++)
      {
        Serial.write(buff[i]);
      }
      Serial.printf(", length: %d\n\r", nums);
      //guard.~LockGuard();   // automatic unlock, not needed
    }
  };
  
private:
  int Id;
  Message &Mail;
  uint8_t buff[256];
  Mutex& Lock;
};


void setup (void)
{
  
  // start up the serial interface
  Serial.begin(115200);
  while(!Serial);
  Serial.println("started");

  Serial.println("Testing FreeRTOS C++ wrappers");
  Serial.println("Queues Simple Producer / Consumer");

  delay(1000);
  
  Message *Mail;
  //
  //  These parameters may be adjusted to explore queue 
  //  behaviors.
  //
  Mail = new Message(256);

  MutexStandard *SharedLock;
  SharedLock = new MutexStandard();
   
  ProducerThread *p1;
  ConsumerThread *p2;
  p1 = new ProducerThread(1, 4, *Mail,*SharedLock);
  p2 = new ConsumerThread(2, *Mail,*SharedLock);
  
  Thread::StartScheduler();
  
  //
  //  We shouldn't ever get here unless someone calls 
  //  Thread::EndScheduler()
  //
  
  Serial.println("Scheduler ended!");

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