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


#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_
#include "ooFreeRTOS_config.h"
/**
 *  C++ exceptions are used by default when constructors fail.
 *  If you do not want this behavior, define the following in your makefile
 *  or project. Note that in most / all cases when a constructor fails,
 *  it's a fatal error. In the cases when you've defined this, the new 
 *  default behavior will be to issue a configASSERT() instead.
 */
#ifndef CPP_FREERTOS_NO_EXCEPTIONS
#include <exception>
#include <cstdio>
#include <string>
#ifdef CPP_FREERTOS_NO_CPP_STRINGS
#error "FreeRTOS-Addons require C++ Strings if you are using exceptions"
#endif
#endif
#include "FreeRTOS.h"
#include "message_buffer.h"


namespace cpp_freertos {


#ifndef CPP_FREERTOS_NO_EXCEPTIONS
/**
 *  This is the exception that is thrown if a Message constructor fails.
 */
class MessageCreateException : public std::exception {

    public:
        /**
         *  Create the exception.
         */
        MessageCreateException()
        {
            sprintf(errorString, "Message Constructor Failed");
        }

        /**
         *  Create the exception.
         */
        explicit MessageCreateException(const char *info)
        {
            snprintf(errorString, sizeof(errorString),
                        "Message Constructor Failed %s", info);
        }

        /**
         *  Get what happened as a string.
         *  We are overriding the base implementation here.
         */
        virtual const char *what() const throw()
        {
            return errorString;
        }

    private:
        /**
         *  A text string representing what failed.
         */
        char errorString[80];
};
#endif


/**
 *  Message class wrapper for FreeRTOS Messages. This class provides send Message
 *  and recive Message operations.
 *
 *  @note It is expected that an application will instantiate this class or
 *        one of the derived classes and use that. It is not expected that
 *        a user or application will derive from these classes.
 */
class Message {

    /////////////////////////////////////////////////////////////////////////
    //
    //  Public API
    //
    /////////////////////////////////////////////////////////////////////////
    public:
        /**
         *  Our constructor.
         *
         *  @throws MessageCreateException
         *  @param buffSize of an item in a Message.
         *  @note FreeRTOS Messages use a memcpy / fixed size scheme for Messages.
         */
        Message(UBaseType_t buffSize);

        /**
         *  Our destructor.
         */
        virtual ~Message();

        /**
         *  Sends a discrete message to a message buffer.
         *
         *  @param item A pointer to the message that is to be copied into the message buffer. 
         *  @param Timeout How long to wait to add the item to the Message if
         *          the Message is currently full.
         *  @return The number of bytes written to the message buffer. 
         */
        virtual size_t Send(void *buff, size_t length, TickType_t Timeout = portMAX_DELAY);

        /**
         *  Remove an item from the front of the Message.
         *
         *  @param buff A pointer to the message that is to be copied into the message buffer. 
         *  @param Timeout How long to wait to remove an item if the Message
         *         is currently empty.
         *  @return true if an item was removed, false if no item was removed.
         */
        virtual size_t Receive(void *buff, size_t length, TickType_t Timeout = portMAX_DELAY);

        /**
         *  Add an item to the back of the Message in ISR context.
         *
         *  @param item The item you are adding.
         *  @param pxHigherPriorityTaskWoken Did this operation result in a
         *         rescheduling event.
         *  @return true if the item was added, false if it was not.
         */
        virtual size_t SendFromISR(void *buff, size_t length, BaseType_t *pxHigherPriorityTaskWoken);

        /**
         *  Make a copy of an item from the front of the Message. This will
         *  not remove it from the head of the Message.
         *
         *  @param item Where the item you are removing will be returned to.
         *  @return true if an item was copied, false if no item was copied.
         */
        size_t ReceiveFromISR(void *buff, size_t length, BaseType_t *pxHigherPriorityTaskWoken);
        /**
         *  Is the Message empty?
         *  @return true if the Message was empty when this was called, false if
         *  the Message was not empty.
         */
        bool IsEmpty();

        /**
         *  Is the Message full?
         *  @return true if the Message was full when this was called, false if
         *  the Message was not full.
         */
        bool IsFull();

        /**
         *  Remove all objects from the Message.
         */
        void Flush();

        /**
         *  How many empty spaves are currently left in the Message.
         *  @return the number of remaining spaces.
         */
        UBaseType_t NumSpacesLeft();

    /////////////////////////////////////////////////////////////////////////
    //
    //  Protected API
    //  Not intended for use by application code.
    //
    /////////////////////////////////////////////////////////////////////////
    protected:
        /**
         *  FreeRTOS Message handle.
         */
        MessageBufferHandle_t handle;
};


}
#endif
