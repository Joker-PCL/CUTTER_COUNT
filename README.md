# Getting started with the Sunton ESP32-S3 7 inch display, LovyanGFX and LVGL

Simple demo application how to use this display with LovyanGFX and LVGL.

Some more explanation you can find on https://www.haraldkreuzer.net/en/news/getting-started-makerfabs-esp32-s3-7-inch-display-lovyangfx-and-lvgl


# Libaries
    LovyanGFX: 1.1.12
    LVGL: 8.3.11

# สูตรคำนวนความเร็วการตัดต่อนาที
    - หาความเร็วรอบโดยจับเวลาที่ใช้ในการตัดแต่ละรอบเช่น เวลาเริ่มต้น 100ms เวลาสิ้นสุด 1600ms = 1500ms
    - นำเวลาที่ได้ไปคำนวณเป็นการตัดต่อนาทีเช่น 1 นาทีเท่ากับ 60*1000ms / 1500ms = 40 ตัด/นาที
