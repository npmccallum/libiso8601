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

#include "iso8601.h"
#include "internal.h"

#include <errno.h>
#include <ctype.h>
#include <string.h>

/*
 * NOTE: According to ISO 8601:2004, decimal length is potentially unlimited.
 *       However, the assumed decimal max below exceeds IEEE 754 precision and
 *       should thus be acceptable for our uses.
 */
#define ISO8601_DEC 35
#define ISO8601_MAX (28 + ISO8601_DEC)
#define ISO8601_MIN 4

static bool convert(char *str, int offset, int digits,
                    int min, int max, const char *end,
                    int32_t *out)
{
    int32_t tmp = 0;

    for (int i = 0; i < digits; i++) {
        if (!isdigit(str[offset + i]))
            return false;
        tmp *= 10;
        tmp += str[offset + i] - '0';
    }

    if (tmp < min || (max >= 0 && tmp > max))
        return false;

    /* Consume the characters converted. */
    if (end != NULL && strncmp(&str[offset + digits], end, strlen(end)) == 0)
        offset += strlen(end);
    memmove(str, &str[offset + digits], strlen(&str[offset + digits]) + 1);
    *out = tmp;
    return true;
}

static bool convert_uint8(char *str, int offset, int digits,
                          int min, int max, const char *end,
                          uint8_t *out)
{
    int32_t tmp;
    bool retval;

    retval = convert(str, offset, digits, min, max, end, &tmp);
    *out = tmp;
    return retval;
}

static bool parse_ordinal(char *buf, iso8601_time *time)
{
    int32_t ordinal;

    if (!convert(buf, 0, 3, 1, length_year_days(time->year), "T", &ordinal))
        return false;

    return ordinal_to_date(time->year, ordinal, &time->month, &time->day);
}

static bool parse_weekdate(char *buf, iso8601_time *time)
{
    int32_t wday = 1;
    int32_t week = 1;

    /* Convert the week. */
    if (!convert(buf, 1, 2, 1, length_year_weeks(time->year), "-", &week))
        return EINVAL;

    switch (buf[0]) {
    case 'T':
        memmove(&buf[0], &buf[1], strlen(&buf[1]) + 1); /* Consume the T. */
        break;
    case '\0':
        break;
    default:
        /* Convert the week day, if specified. */
        if ((strlen(buf) % 2 == 1) == (strchr(buf, 'T') == NULL)) {
            if (!convert(buf, 0, 1, 1, 7, "T", &wday))
                return EINVAL;
        }
        break;
    }

    return weekdate_to_date(time->year, week, wday,
                            &time->year, &time->month, &time->day);
}

static bool parse_date(char *buf, iso8601_time *time)
{
    switch (buf[0]) {
    case 'T':
        memmove(&buf[0], &buf[1], strlen(&buf[1]) + 1); /* Consume the T. */
        return true;
    case '\0':
        return true;
    case 'W':
        /* Weekdate Format */
        return parse_weekdate(buf, time);
    }

    /* Ordinal Format */
    if ((strlen(buf) % 2 == 1) == (strchr(buf, 'T') == NULL))
        return parse_ordinal(buf, time);

    /* Standard Format */
    if (!convert_uint8(buf, 0, 2, 1, 12, "-", &time->month))
        return false;

    switch (buf[0]) {
    case 'T':
        memmove(&buf[0], &buf[1], strlen(&buf[1]) + 1); /* Consume the T. */
        return true;
    case '\0':
        return true;
    }

    return convert_uint8(buf, 0, 2, 1,
                         length_month_days(time->year, time->month),
                         "T", &time->day);
}

static bool parse_time(char *buf, double decimal, iso8601_time *time)
{
    time->hour = 0;
    time->minute = 0;
    time->second = 0;
    if (buf[0] == '\0')
        return true;

    /* Convert hours, possibly including decimal. */
    if (!convert_uint8(buf, 0, 2, 0, 24, ":", &time->hour))
        return false;
    if (time->hour == 24 && decimal != 0)
        return false;
    if (buf[0] == '\0') {
        time->minute = 60 * decimal;
        return true;
    }

    /* Convert minutes, possibly including decimal. */
    if (!convert_uint8(buf, 0, 2, 0, time->hour == 24 ? 0 : 59, ":", &time->minute))
        return false;
    if (buf[0] == '\0') {
        time->second = 60 * decimal;
        return true;
    }

    /* Convert seconds, including decimal. */
    if (!convert_uint8(buf, 0, 2, 0, time->hour == 24 ? 0 : 60, NULL, &time->second))
        return false;
    time->usecond = decimal * 1000000;
    return true;
}

static bool parse_end_timezone(char *buf, iso8601_time *time)
{
    int multiplier = 1;
    int hoff = 0;
    int moff = 0;
    size_t len;

    /* We have a date. */
    len = strlen(buf);
    if (len < 1)
        goto local;

    /* Look for Z. */
    if (buf[len - 1] == 'Z') {
        buf[len - 1] = '\0';
        time->tzminutes = 0;
        time->localtime = false;
        return true;
    }

    /* Parse the minutes if specified. */
    if (len >= 5) {
        switch (buf[len - 5]) {
        case '-': /* fallthrough */
        case '+':
            if (!convert(&buf[len - 2], 0, 2, 0, 59, NULL, &moff))
                return false;
            len -= 2;
            break;
        }
    }

    /* Parse the hours. */
    if (len >= 3) {
        switch (buf[len - 3]) {
        case '-':
            multiplier = -1;
            /* fallthrough */
        case '+':
            if (!convert(&buf[len - 3], 1, 2, 0, 24, NULL, &hoff))
                return false;

            time->tzminutes = ((hoff * 60) + moff) * multiplier;
            time->localtime = false;
            return true;
        }
    }

local:
    time->tzminutes = 0; /* TODO: Set the UTC offset? */
    time->localtime = true;
    return true;
}

static bool parse_end_decimal(char *buf, double *out)
{
    int len = strlen(buf);
    for (int i = len - 1; i >= 0 && i >= len - ISO8601_DEC; i--) {
        if (isdigit(buf[i]))
            continue;

        if (buf[i] != '.')
            break;

        *out = strtod(&buf[i], NULL);
        buf[i] = '\0';
        return true;
    }

    *out = 0;
    return true;
}

static bool normalize_iso8601(char *buf)
{
    int i = 0, j = 0;
    int decimals = 0;

    do {
        buf[j] = buf[i];

        switch (buf[j]) {
        case '-':
            /*
             * Handle the weekday case which requires an odd number of digits
             * after the dash. In the most common case, this number will be 0
             * or 1. But if 'T' is not present, it can be all the way to the
             * end of the time section. Take for example, the case of midnight
             * on January first in local time: 1989W011000000
             */
            if (j == 3 && buf[0] == 'W') {
                int digits;
                for (digits = 0; buf[i + digits + 1] != '\0'; digits++) {
                    if (!isdigit(buf[i + digits + 1]))
                        break;
                }
                if (digits % 2 == 1)
                    continue;
            }

            /* - is removed from the date but retained in the TZ. */
            if (j < 3)
                continue;
            break;

        case '+':
            /* + can only appear in the TZ. */
            if (j < 3)
                return false;
            break;

        case 'W':
            /* W can only appear immediately after the year. */
            if (j != 0)
                return false;
            break;

        case 'Z':
            /* Z can only come at the end. */
            if (buf[i + 1] != '\0')
                return false;
            break;

        case ':':
            continue;

        case '.':
            /* Only one permitted. */
            if (decimals++ > 0)
                return false;
            break;

        case 'T':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '\0':
            break;

        default:
            return false;
        }

        j++;
    } while (buf[i++] != '\0');

    buf[j] = '\0';
    return true;
}

static bool parse_year(char *buf, iso8601_time *time)
{
    int multiplier = 1;
    int digits = 4;
    int sign = 0;
    int sep = 0;

    /* Parse year multiplier. */
    switch (buf[0]) {
    case '-':
        multiplier = -1;
        /* fallthrough */
    case '+':
        sign++;
        digits = 0;
        break;
    default:
        break;
    }

    /* Count the number of digits. */
    for (int i = sign; i > 0; i++) {
        switch (buf[i]) {
        case 'W':
            if (sep < 2)
                goto convert;
            return false;

        case '\0':
        case ':':
        case '.':
        case '+':
        case 'Z':
            digits = 4;
            goto convert;

        case 'T':
            if (sep != 2)
                digits = 4;
            goto convert;

        case '-':
            sep++;
            break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            if (sep == 0)
                digits++;
            break;

        default:
            return false;
        }
    }

convert:
    /* Convert the year. */
    if (!convert(buf, sign, digits, 0, -1, "-", &time->year))
        return false;

    time->year *= multiplier;
    time->month = 1;
    time->day = 1;

    return true;
}

int iso8601_parse(const char *in, iso8601_time *out)
{
    char buf[ISO8601_MAX + 1];
    iso8601_time time = {};
    double decimal = 0;

    if (in == NULL)
        return EINVAL;

    /* Copy the string into a local buffer. */
    if (strlen(in) < ISO8601_MIN || strlen(in) > ISO8601_MAX)
        return EINVAL;
    strcpy(buf, in);

    /* Parse the year. */
    if (!parse_year(buf, &time))
        return EINVAL;

    /* Normalize the string. */
    if (!normalize_iso8601(buf))
        return EINVAL;

    /* Parse the timezone off the end. */
    if (!parse_end_timezone(buf, &time))
        return EINVAL;

    /* Parse the decimal off the end. */
    if (!parse_end_decimal(buf, &decimal))
        return EINVAL;

    /* Parse the date. */
    if (!parse_date(buf, &time))
        return EINVAL;

    /* Parse the time. */
    if (!parse_time(buf, decimal, &time))
        return EINVAL;

    *out = time;
    return 0;
}
