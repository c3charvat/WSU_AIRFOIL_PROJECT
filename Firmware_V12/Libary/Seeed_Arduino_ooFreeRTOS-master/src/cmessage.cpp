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

#include "message.hpp"

using namespace cpp_freertos;

Message::Message(UBaseType_t bufferSize)
{
    handle = xMessageBufferCreate(bufferSize);

    if (handle == NULL)
    {
#ifndef CPP_FREERTOS_NO_EXCEPTIONS
        throw MessageCreateException();
#else
        configASSERT(!"Queue Constructor Failed");
#endif
    }
}

Message::~Message()
{
    vMessageBufferDelete(handle);
}

size_t Message::Send(void *buff, size_t length, TickType_t Timeout)
{
    size_t xBytesSent;
    xBytesSent = xMessageBufferSend(handle, (void *)buff, length, Timeout);
    return xBytesSent;
}

size_t Message::Receive(void *buff, size_t length, TickType_t Timeout)
{
    size_t xReceivedBytes;
    xReceivedBytes = xMessageBufferReceive(handle, (void *)buff, length, Timeout);
    return xReceivedBytes;
}

size_t Message::SendFromISR(void *buff, size_t length, BaseType_t *pxHigherPriorityTaskWoken)
{
    size_t xBytesSent;
    xBytesSent = xMessageBufferSendFromISR(handle, (void *)buff, length, pxHigherPriorityTaskWoken);
    return xBytesSent;
}

size_t Message::ReceiveFromISR(void *buff, size_t length, BaseType_t *pxHigherPriorityTaskWoken)
{
    size_t xReceivedBytes;
    xReceivedBytes = xMessageBufferReceiveFromISR(handle, (void *)buff, length, pxHigherPriorityTaskWoken);
    return xReceivedBytes;
}

bool Message::IsEmpty()
{
    UBaseType_t cnt = xMessageBufferIsEmpty(handle);

    return cnt == pdTRUE ? true : false;
}

bool Message::IsFull()
{
    UBaseType_t cnt = xMessageBufferIsFull(handle);

    return cnt == pdTRUE ? true : false;
}

void Message::Flush()
{
    xMessageBufferReset(handle);
}

UBaseType_t Message::NumSpacesLeft()
{
    return xMessageBufferSpacesAvailable(handle);
}
