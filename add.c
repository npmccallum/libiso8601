/* vim: set tabstop=8 shiftwidth=4 softtabstop=4 expandtab smarttab colorcolumn=80: */
/**
 * Copyright: 2013 Red Hat, Inc.
 * Author: Nathaniel McCallum <npmccallum@redhat.com>
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "iso8601.h"
#include "internal.h"

void iso8601_add_months(iso8601_time *time, int months)
{
    months = ((int) time->month) - 1 + months;
    time->year += months / 12;
    time->month = months % 12 + 1;
}

void iso8601_add_days(iso8601_time *time, int days)
{
    days = ((int) time->day) + days;

    if (days > 0) {
        while (days > length_month_days(time->year, time->month)) {
            days -= length_month_days(time->year, time->month);
            iso8601_add_months(time, 1);
        }
    } else {
        do {
            iso8601_add_months(time, -1);
            days += length_month_days(time->year, time->month);
        } while (days < 1);
    }

    time->day = days;
}

void iso8601_add_hours(iso8601_time *time, int hours)
{
    hours = ((int) time->hour) + hours;
    iso8601_add_days(time, hours / 24);
    time->hour = hours % 24;
}

void iso8601_add_minutes(iso8601_time *time, int minutes)
{
    minutes = ((int) time->minute) + minutes;
    iso8601_add_hours(time, minutes / 60);
    time->minute = minutes % 60;
}

void iso8601_add_seconds(iso8601_time *time, int seconds)
{
    seconds = ((int) time->second) + seconds;
    iso8601_add_minutes(time, seconds / 60);
    time->second = seconds % 60;
}

void iso8601_add_useconds(iso8601_time *time, int useconds)
{
    useconds = ((int) time->usecond) + useconds;
    iso8601_add_seconds(time, useconds / 1000000);
    time->usecond = useconds % 1000000;
}
