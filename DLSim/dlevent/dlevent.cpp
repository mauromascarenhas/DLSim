#include "dlevent.h"

DLEvent::DLEvent(int dNumber, int time, int size)
{
    c_dataSize = size;
    c_deviceNumber = dNumber;
    c_transmitedData = 0;
    c_startTransmissionTime = time;
}

int DLEvent::dataSize() const {
    return c_dataSize;
}

void DLEvent::setDataSize(int newDataSize){
    c_dataSize = newDataSize;
}

int DLEvent::transmitedData() const{
    return c_transmitedData;
}
void DLEvent::setTransmitedData(int newTDataSize){
    c_transmitedData = newTDataSize;
}

int DLEvent::deviceNumber() const
{
    return c_deviceNumber;
}

void DLEvent::setDeviceNumber(int newDeviceNumber)
{
    c_deviceNumber = newDeviceNumber;
}

int DLEvent::startTransmissionTime() const
{
    return c_startTransmissionTime;
}

void DLEvent::setStartTransmissionTime(int newStartTransmissionTime)
{
    c_startTransmissionTime = newStartTransmissionTime;
}

