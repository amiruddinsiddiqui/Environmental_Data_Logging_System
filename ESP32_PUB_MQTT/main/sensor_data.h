#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#define SENSOR_MSG_SIZE 80

typedef struct
{
    char data[SENSOR_MSG_SIZE];

} SensorMessage_t;

#endif