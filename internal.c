/* vim: set tabstop=8 shiftwidth=4 softtabstop=4 expandtab smarttab colorcolumn=80: */
/**
 * Copyright: 2013 Red Hat, Inc.
 * Author: Nathaniel McCallum <npmccallum@redhat.com>
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

#include "internal.h"
#include <assert.h>
#include <string.h>
#include <time.h>

static bool is_leapyear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

/* Return the offset from Jan 1 to the start of week 1 (may be negative). */
static int8_t weekdate_offset(int32_t year)
{
    struct tm tmp = { .tm_year = year - 1900, .tm_mday = 1 };

    /* This should never fail. */
    assert(mktime(&tmp) != -1);

    switch (tmp.tm_wday) {
    case 5: /* Friday */
    case 6: /* Saturday */
        return 8 - tmp.tm_wday;
    default:
        return 1 - tmp.tm_wday;
    }
}

uint16_t length_year_days(int32_t year)
{
    return is_leapyear(year) ? 366 : 365;
}

uint8_t length_year_weeks(int32_t year)
{
    return (length_year_days(year) - weekdate_offset(year) + 3) / 7;
}

uint8_t length_month_days(int32_t year, uint8_t month)
{
    static const uint8_t ndays[12] = {
        31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };

    if (month > 12 || month < 1)
        return 0;

    if (month != 2)
        return ndays[month - 1];

    return is_leapyear(year) ? 29 : 28;
}

bool ordinal_to_date(int32_t year, uint16_t ordinal,
                     uint8_t *month, uint8_t *day)
{
    if (ordinal < 1 || ordinal > length_year_days(year))
        return false;

    for (*month = 1; ordinal > length_month_days(year, *month); (*month)++)
        ordinal -= length_month_days(year, *month);

    *day = ordinal;
    return true;
}

bool ordinal_from_date(int32_t year, uint8_t month, uint8_t day,
                       uint16_t *ordinal)
{
    uint16_t ord = 0;

    if (month < 1 || month > 12)
        return false;
    if (day < 1 || day > length_month_days(year, month))
        return false;

    for (; month > 1; month--)
        ord += length_month_days(year, month - 1);

    *ordinal = ord + day;
    return true;
}

bool weekdate_to_date(int32_t wyear, uint8_t week, uint8_t wday,
                      int32_t *year, uint8_t *month, uint8_t *day)
{
    int16_t ordinal;

    if (week > length_year_weeks(wyear))
        return false;

    if (wday < 1 || wday > 7)
        return false;

    ordinal = --week * 7 + --wday + weekdate_offset(wyear) + 1;
    if (ordinal > 0) {
        if (ordinal > length_year_days(wyear))
            ordinal -= length_year_days(wyear++);
        return ordinal_to_date(*year = wyear, ordinal, month, day);
    }

    ordinal += length_year_days(--wyear);
    return ordinal_to_date(*year = wyear, ordinal, month, day);
}

bool weekdate_from_date(int32_t year, uint8_t month, uint8_t day,
                        int32_t *wyear, uint8_t *week, uint8_t *wday)
{
    uint16_t ordinal = 0;
    if (!ordinal_from_date(year, month, day, &ordinal))
        return false;

    if (ordinal <= weekdate_offset(year))
        ordinal += length_year_days(--year);

    *wyear = year;
    *week = (ordinal - weekdate_offset(year) - 1) / 7 + 1;
    *wday = (ordinal - weekdate_offset(year) - 1) % 7 + 1;
    if (*week > length_year_weeks(year)) {
        (*wyear)++;
        *week -= length_year_weeks(year);
    }

    return true;
}
