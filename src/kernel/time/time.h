//
// Created by 赵明明 on 2020/2/7.
// https://wiki.osdev.org/CMOS#Getting_Current_Date_and_Time_from_RTC
//

#ifndef _TIME_H
#define _TIME_H

#include "../lib/defs.h"
#include "../lib/x86.h"

#define CMOS_ADDR   0x70
#define CMOS_DATA   0x71

uint8_t read_RTC_register(uint8_t reg) {
    io_out8(CMOS_ADDR, reg | 0x80U);
    return io_in8(CMOS_DATA);
}

struct Time {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
};

void get_time(struct Time *time) {
    cli();
    uint8_t century;
    do {
        time->second = read_RTC_register(0x00);
        time->minute = read_RTC_register(0x02);
        time->hour = read_RTC_register(0x04);
        time->day = read_RTC_register(0x07);
        time->month = read_RTC_register(0x08);
        time->year = read_RTC_register(0x09);
        century = read_RTC_register(0x32);
    } while (time->second != read_RTC_register(0x00));

    // Status Register B
    // bit 1: 24 hour mode if set
    // bit 2: binary mode if set
    uint8_t reg_B = read_RTC_register(0x0B);
    io_out8(CMOS_ADDR, 0);
    sti();

    // convert BCD to binary values
    if (!(reg_B & 0x04U)) {
        time->second = (time->second & 0x0FU) + (time->second / 16) * 10;
        time->minute = (time->minute & 0x0FU) + (time->minute / 16) * 10;
        time->hour = (time->hour & 0x0FU) + (((time->hour & 0x70U) / 16) * 10 | (time->hour & 0x80U));
        time->day = (time->day & 0x0FU) + (time->day / 16) * 10;
        time->month = (time->month & 0x0FU) + (time->month / 16) * 10;
        time->year = (time->year & 0x0FU) + (time->year / 16) * 10;
        century = (century & 0x0FU) + (century / 16) * 10;
    }
    // convert 12 hour clock to 24 hour clock
    if (!(reg_B & 0x02U) && time->hour & 0x80U) {
        time->hour += 12;
    }

    time->year += century * 100;
}

#endif //_TIME_H
