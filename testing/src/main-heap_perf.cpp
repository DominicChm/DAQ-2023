#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <vector>

using namespace std;

QueueHandle_t alloc_small() {
    QueueHandle_t h = xQueueCreate(1000, sizeof(vector<uint8_t>*));

    for (int i = 0; i < 1000; i++) {
        xQueueSendToBack(h, new vector<uint8_t>(128), 0);
    }

    return h;
}

QueueHandle_t alloc_big() {
    QueueHandle_t h = xQueueCreate(100, sizeof(vector<uint8_t>*));

    for (int i = 0; i < 100; i++) {
        xQueueSendToBack(h, new vector<uint8_t>(1024), 0);
    }

    return h;
}

void setup() {
    clock_t t1, t2;
    QueueHandle_t h;

    Serial.begin(115200);

    t1 = clock();
    h = alloc_small();
    t2 = clock();
    while (uxQueueMessagesWaiting(h) != 0) {
        vector<uint8_t>* p;
        xQueueReceive(h, &p, 100);
        delete p;
    }
    vQueueDelete(h);

    Serial.printf("Small Allocs: %lf ms", difftime(t2, t1));
    Serial.printf("\n");

    t1 = clock();
    h = alloc_big();
    t2 = clock();
    while (uxQueueMessagesWaiting(h)) {
        vector<uint8_t>* p;
        xQueueReceive(h, &p, 0);
        delete p;
    }
    vQueueDelete(h);

    Serial.printf("Big Allocs: %lf ms", difftime(t2, t1));
    Serial.printf("\n");
    // return 0;
}

void loop() {
}