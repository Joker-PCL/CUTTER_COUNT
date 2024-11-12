#define LGFX_LCD4_3
// #define LGFX_LCD7_0

// กำหนดค่าการใช้งาน touch screen
// #define USE_TOUCH_SCREEN

#include "./gui/gui.h"
#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include <lv_conf.h>
#include <lvgl.h>

#define CUTTER_SENSOR 13

long PREV_CUTTER_SENSOR = 0;   // ค่าเซ็นเซอร์ก่อนหน้า
int CUTTER_SENSOR_DELAY = 300; // หน่วงเวลาอ่านเซ็นเซอร์ ms
unsigned int LED_CUTTER_SENSOR_TIME = 0;
unsigned long CYCLE_TIME = 0; // เก็บเวลาเวลาในแต่ละรอบ
unsigned long TIMEOUT = 0;    // เก็บเวลา TIMEOUT

bool isRunning = false;           // สถานะการทำงาน
int MAX_CPM = 300;                // กำหนดค่าสูงสุดจำนวนการตัดใน 1 นาที
unsigned long CPM_COUNT_TIME = 0; // เก็บเวลาตัวนับจำนวนใน 1 นาที
unsigned int LED_CPM_COUNT_TIME = 0;
int CPM_COUNT = 0; // เก็บจำนวนการตัดใน 1 นาที

struct Sensor {
    const uint8_t PIN;
    bool triggered;
};

Sensor sensor = {CUTTER_SENSOR, false};

void IRAM_ATTR irsSensor() {
    if (millis() - PREV_CUTTER_SENSOR > CUTTER_SENSOR_DELAY) {
        sensor.triggered = true;
        PREV_CUTTER_SENSOR = millis();
    }
}

// อัพเดท Count
void updateScrCount(lv_obj_t *ui_tens, lv_obj_t *ui_ones, int count) {
    // คำนวณค่าหลักสิบและหลักหน่วย
    int tens = count / 10;
    int ones = count % 10;

    // แสดงค่าหลักสิบและหลักหน่วยแยกกัน
    lv_roller_set_selected(ui_tens, tens, LV_ANIM_ON);
    lv_roller_set_selected(ui_ones, ones, LV_ANIM_ON);
}

void setup() {
    // ห้ามลบ
    Serial.begin(115200);
    gui_start();
    gfx.setBrightness(244);

    pinMode(CUTTER_SENSOR, INPUT_PULLUP);
    attachInterrupt(sensor.PIN, irsSensor, FALLING);
}

void loop() {
    // ห้ามลบ
    lv_timer_handler();

    if (sensor.triggered) {
        sensor.triggered = false;
        unsigned long CPS = millis() - CYCLE_TIME;       // เปรียบเทียบเวลาในรอบการตัดเป็น มิลลิวินาที
        float CPM_F = float(60UL * 1000UL) / float(CPS); // คำนวนจำนวนตัดต่อนาที 1 นาที หาร กับเวลาในรอบการตัด
        int CPM = ceil(CPM_F);                           // คำนวนจำนวนตัดต่อนาที 1 นาที หาร กับเวลาในรอบการตัด
        CYCLE_TIME = millis();                           // รีเซ็ตตัวจับเวลา
        TIMEOUT = millis();                              // รีเซ็ตเวลา timeout

        isRunning = true; // เปลี่ยนสถานะการทำงาน -> true
        CPM_COUNT++;      // เพิ่มค่าจำนวนการตัดใน 1 นาที

        // อัพเดทหน้าจอ CPM_COUNT
        lv_label_set_text(ui_CPMcount, String(CPM_COUNT).c_str());
        Serial.printf("CPS: %u ms, CPM_F: %.2f cut/m, CPM: %d cut/m, Total CPM: %d cut\n", CPS, CPM_F, CPM, CPM_COUNT);

        LED_CUTTER_SENSOR_TIME = millis();
        lv_obj_set_style_bg_color(ui_LedCPMcount, lv_color_hex(0x53E903), 0);
        if (CPM < MAX_CPM)
            updateScrCount(ui_CPM1, ui_CPM2, CPM); // อัพเดทหน้าจอ CPM
    }

    // ปิดไฟสถานะ CUTTER
    if (millis() - LED_CUTTER_SENSOR_TIME >= 150) {
        lv_obj_set_style_bg_color(ui_LedCPMcount, lv_color_hex(0x1E3D0E), 0);
    }

    // หากไม่มีสัญญานจากเซ็นเซอร์ภายใน 3 วินาที และ สถานะการทำงาน -> true
    // ให้รีเซ็ตค่า CPM, CPMtime, CPMcount เป็น 0 และ สถานะการทำงาน -> false
    if (millis() - TIMEOUT >= 3000UL && isRunning) {
        isRunning = false;                                 // เปลี่ยนสถานะการทำงาน -> false
        updateScrCount(ui_CPM1, ui_CPM2, 0);               // อัพเดทหน้าจอ CPM
        lv_label_set_text(ui_CPMtime, String(60).c_str()); // อัพเดทหน้าจอ CPM_COUNT
        lv_label_set_text(ui_CPMcount, String(0).c_str()); // อัพเดทหน้าจอ CPM_COUNT
    }

    //  เก็บค่าเซ็นต์เซอร์ ทุกๆ 1 นาที หาก สถานะการทำงาน -> true
    if (millis() - CPM_COUNT_TIME <= 60000UL && isRunning) {
        // เปิดไฟสถานะตัวจับเวลา
        if ((millis() - LED_CPM_COUNT_TIME) >= 1000) {
            lv_obj_set_style_bg_color(ui_LedCPMtime, lv_color_hex(0x53E903), 0);
            LED_CPM_COUNT_TIME = millis();
            lv_label_set_text(ui_CPMtime, String((millis() - CPM_COUNT_TIME) / 1000).c_str()); // อัพเดทหน้าจอ CPM_COUNT
        }
    } else {
        CPM_COUNT = 0;             // รีเซ็ตค่าจำนวนการตัดใน 1 นาที
        CPM_COUNT_TIME = millis(); // รีเซ็ตเวลาตัวนับจำนวนใน 1 นาที
    }

    // ปิดไฟสถานะตัวจับเวลา
    if (millis() - LED_CPM_COUNT_TIME >= 300) {
        lv_obj_set_style_bg_color(ui_LedCPMtime, lv_color_hex(0x1E3D0E), 0);
    }
}
