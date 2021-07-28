#ifndef DLEVENT_H
#define DLEVENT_H

#include <QString>

class DLEvent
{
public:
    DLEvent(int dNumber = -1, int time = -1, int size = -1);

    int dataSize() const;
    void setDataSize(int newDataSize);

    int transmitedData() const;
    void setTransmitedData(int newTDataSize);

    int deviceNumber() const;
    void setDeviceNumber(int newDeviceNumber);

    int startTransmissionTime() const;
    void setStartTransmissionTime(int newStartTransmissionTime);

private:
    int c_dataSize;
    int c_deviceNumber;
    int c_transmitedData;
    int c_startTransmissionTime;
};

#endif // DLEVENT_H
