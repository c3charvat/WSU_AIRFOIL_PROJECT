# Seeed_Arduino_ooFreeRTOS  [![Build Status](https://travis-ci.com/Seeed-Studio/Seeed_Arduino_ooFreeRTOS.svg?branch=master)](https://travis-ci.com/Seeed-Studio/Seeed_Arduino_ooFreeRTOS)

## Introduction
**Seeed_Arduino_ooFreeRTOS is fork from [freertos-addons](https://github.com/michaelbecker/freertos-addons).**

Seeed_Arduino_ooFreeRTOS is a collection of C++ wrappers encapsulating FreeRTOS functionality, And it working under the framework of Arduino.With it, you can easily apply FreeRTOS to your Arduino project!!!

## Usage
```C++
#include <cstring>
#include "thread.hpp"
using namespace cpp_freertos;

class BlinkThread : public Thread {
  
public:
  
  BlinkThread(int delayInSeconds)
    : Thread( 256, 1), 
      Id (i), 
      DelayInSeconds(delayInSeconds)
  {
    Start();
  };
  
protected:

  virtual void Run() {
    while (true) {
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      Delay(Ticks::SecondsToTicks(DelayInSeconds));
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
      Delay(Ticks::SecondsToTicks(DelayInSeconds));      
    }
  };

private:
  int DelayInSeconds;
};

void setup (void)
{
  // start up the serial interface
  Serial.begin(115200);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  static BlinkThread blink(1);
  Thread::StartScheduler();
}

void loop()
{
  // Empty. Things are done in Tasks.
}
```

----

This software is written by seeed studio<br>
and is licensed under [The MIT License](http://opensource.org/licenses/mit-license.php). Check License.txt for more information.<br>

Contributing to this software is warmly welcomed. You can do this basically by<br>
[forking](https://help.github.com/articles/fork-a-repo), committing modifications and then [pulling requests](https://help.github.com/articles/using-pull-requests) (follow the links above<br>
for operating guide). Adding change log and your contact into file header is encouraged.<br>
Thanks for your contribution.

Seeed Studio is an open hardware facilitation company based in Shenzhen, China. <br>
Benefiting from local manufacture power and convenient global logistic system, <br>
we integrate resources to serve new era of innovation. Seeed also works with <br>
global distributors and partners to push open hardware movement.<br>
