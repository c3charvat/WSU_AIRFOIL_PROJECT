/****************************************************************************
 *
 *  Copyright (c) 2017, Michael Becker (michael.f.becker@gmail.com)
 *
 *  This file is part of the FreeRTOS Add-ons project.
 *
 *  Source Code:
 *  https://github.com/michaelbecker/freertos-addons
 *
 *  Project Page:
 *  http://michaelbecker.github.io/freertos-addons/
 *
 *  On-line Documentation:
 *  http://michaelbecker.github.io/freertos-addons/docs/html/index.html
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files
 *  (the "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so,subject to the
 *  following conditions:
 *
 *  + The above copyright notice and this permission notice shall be included
 *    in all copies or substantial portions of the Software.
 *  + Credit is appreciated, but not required, if you find this project
 *    useful enough to include in your application, product, device, etc.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ***************************************************************************/

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
