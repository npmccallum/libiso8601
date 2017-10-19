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

#ifndef LIBISO8601_H_
#define LIBISO8601_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define ISO8601_FLAG_NONE  (0 << 0)
#define ISO8601_FLAG_BASIC (1 << 0)

typedef struct {
    int32_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint32_t usecond;
    bool localtime;
    int16_t tzoffset;
} iso8601_time;

typedef enum {
    ISO8601_FORMAT_NORMAL = 0,
    ISO8601_FORMAT_WEEKDATE,
    ISO8601_FORMAT_ORDINAL
} iso8601_format;

typedef enum {
    ISO8601_TRUNCATE_NONE = 0,

    ISO8601_TRUNCATE_YEAR,
    ISO8601_TRUNCATE_MONTH,
    ISO8601_TRUNCATE_DAY,
    ISO8601_TRUNCATE_HOUR,
    ISO8601_TRUNCATE_MINUTE,
    ISO8601_TRUNCATE_SECOND,

    ISO8601_TRUNCATE_WEEK = ISO8601_TRUNCATE_MONTH
} iso8601_truncate;

/**
 * Parse an ISO 8601 string into a time structure.
 *
 * @return 0: success
 * @return EINVAL: input is invalid
 */
int iso8601_parse(const char *in, iso8601_time *out);

/**
 * Unparse a time structure into an ISO 8601 string.
 *
 * The year can be represented in ydigits number of digits, between 2 and 9.
 * However, any number besides 4 internally disables ISO8601_FLAG_BASIC.
 *
 * @return 0: success
 * @return EINVAL: input is invalid
 * @return E2BIG: the output buffer is too small to handle the output
 */
int iso8601_unparse(const iso8601_time *in, uint32_t flags, uint8_t ydigits,
                    iso8601_format format, iso8601_truncate truncate,
                    size_t len, char *out);

/**
 * Returns the current time as a time structure.
 *
 * @return 0: success
 * @return errno: gettimeofday() failed to obtain current time
 */
int iso8601_current(bool localtime, int16_t tzoffset, iso8601_time *out);

/**
 * Compare two time structures.
 *
 * @return 0: a == b
 * @return <0: a < b
 * @return >0: a > b
 */
int iso8601_compare(const iso8601_time *a, const iso8601_time *b);

/**
 * Convert a time structure to a tm structure.
 */
void iso8601_to_tm(const iso8601_time *time, struct tm *tm);

/**
 * Convert a tm structure to a time structure.
 */
void iso8601_from_tm(const struct tm *tm, uint32_t usecond, bool localtime,
                     int16_t tzoffset, iso8601_time *time);

/**
 * Convert a time structure to a timeval structure.
 */
void iso8601_to_timeval(const iso8601_time *time, struct timeval *tv);

/**
 * Convert a timeval structure to a time structure.
 */
void iso8601_from_timeval(const struct timeval *tv, bool localtime,
                          int16_t tzoffset, iso8601_time *time);

/**
 * Convert a time structure to time_t.
 */
void iso8601_to_time_t(const iso8601_time *time, time_t *timet);

/**
 * Convert time_t to a time structure.
 */
void iso8601_from_time_t(time_t timet, uint32_t usecond, bool localtime,
                         int16_t tzoffset, iso8601_time *time);

/**
 * Add the specified number of months to the time.
 */
void iso8601_add_months(iso8601_time *time, int months);

/**
 * Add the specified number of days to the time.
 */
void iso8601_add_days(iso8601_time *time, int days);

/**
 * Add the specified number of hours to the time.
 */
void iso8601_add_hours(iso8601_time *time, int hours);

/**
 * Add the specified number of minutes to the time.
 */
void iso8601_add_minutes(iso8601_time *time, int minutes);

/**
 * Add the specified number of seconds to the time.
 */
void iso8601_add_seconds(iso8601_time *time, int seconds);

/**
 * Add the specified number of useconds to the time.
 */
void iso8601_add_useconds(iso8601_time *time, int useconds);

#endif /* LIBISO8601_H_ */
