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

#include "iso8601.h"

#include <errno.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* These two MUST be the same. */
#define TZ "EST+5"
#define OFFSET 5

#define MINUTES(m) ((m) * 60)
#define HOURS(h) MINUTES(h * 60)
#define DAYS(d) HOURS(d * 24)
#define WEEKS(w) DAYS(w * 7)
#define VAL(val, add) ((val) == 0 ? 0 : ((val) + (add)))
#define UTC_OFFSET(val, hours) VAL(val, 0 - HOURS(OFFSET) - HOURS(hours))

#define GMT(str, val) \
    {str,     val}, \
    {str "Z", UTC_OFFSET(val, 0)}

#define ZONED(str, val) \
    GMT(str,       val), \
    {str "+06",    UTC_OFFSET(val, 6)   }, \
    {str "+06:30", UTC_OFFSET(val, 6.5) }, \
    {str "-06",    UTC_OFFSET(val, -6)  }, \
    {str "-06:30", UTC_OFFSET(val, -6.5)}

#define FRACTION(str, seconds, val) \
    ZONED(str, val), \
    ZONED(str ".5", VAL(val, seconds))

/* NOTE: The first line of the DATE() macro generates a non-standard timezone.
 *       We parse it successfully in the interest of receiver liberality. */
#define DATE(str, val) \
    ZONED(str, val), \
    FRACTION(str "T09",    1800, VAL(val, HOURS(9))), \
    FRACTION(str "T22",    1800, VAL(val, HOURS(22))), \
    FRACTION(str "T09:28",   30, VAL(val, HOURS(9)  + MINUTES(28))), \
    FRACTION(str "T22:59",   30, VAL(val, HOURS(22) + MINUTES(59))), \
    FRACTION(str "T09:28:12", 0, VAL(val, HOURS(9)  + MINUTES(28) + 12)), \
    FRACTION(str "T22:59:32", 0, VAL(val, HOURS(22) + MINUTES(59) + 32))

#define WEEKDATE(str, val) \
    DATE(str "-W01", val), \
    DATE(str "-W01-1", val), \
    DATE(str "-W01-4", VAL(val, DAYS(3))), \
    DATE(str "-W32",   VAL(val, WEEKS(31))), \
    DATE(str "-W32-1", VAL(val, WEEKS(31))), \
    DATE(str "-W32-4", VAL(val, WEEKS(31) + DAYS(3)))

struct test_data {
    const char *string;
    const time_t value;
    const bool notrunct;
};

static const struct test_data TEST_DATA[] = {
    /* Test year only. */
    GMT("1965", -157748400L), /* Zone non-standard. */
    GMT("1989",  599634000L), /* Zone non-standard. */

    /* Test year + month. */
    GMT("1965-02", -155070000L), /* Zone non-standard. */
    GMT("1989-02",  602312400L), /* Zone non-standard. */

    /* Test full date (before epoch). */
    DATE("1965-02-12", -154119600L), /* Standard format. */
    DATE("1965-234",   -137617200L), /* Ordinal format. */

    /* Test full date (after epoch). */
    DATE("1989-02-12", 603262800L), /* Standard format. */
    DATE("1989-234",   619765200L), /* Ordinal format. */

    /* Test weekday format (before epoch). */
    WEEKDATE("1965", -157489200L), /* Current year. */
    WEEKDATE("1963", -220993200L), /* Previous year. */

    /* Test weekday format (after epoch). */
    WEEKDATE("1989", 599720400L), /* Current year. */
    WEEKDATE("1985", 473317200L), /* Previous year. */

    /* Test extended years. */
#if SIZEOF_TIME_T > 4
#if !defined(__APPLE__) && !defined(__MACH__)
    /* macOS mktime() can't handle years before 1900 */
    DATE("-0001-01-01", -62198737200LL),
    WEEKDATE("-0001", -62198478000LL),
#endif
    DATE("+9999-01-01", 253370782800LL),
    WEEKDATE("+9999", 253371042000LL),
#endif
    DATE("+1971-01-01", 31554000L),
    WEEKDATE("+1971", 31813200L),

    /* Test midnight on the same day. */
    {"2000-01-01T00:00:00Z", 946684800L}, /* These should both... */
    {"1999-12-31T24:00:00Z", 946684800L}, /* ... be the same.     */

    /* Test truncated dates where T is present. */
    {"2000T00:00:00Z",     946684800L, true},
    {"2000-01T00:00:00Z",  946684800L, true},
    {"1965T00:00:00Z",    -157766400L, true},
    {"1965-01T00:00:00Z", -157766400L, true},

    /* Test leap second. */
    {"2000-06-30T23:59:59Z", 962409599L},
    {"2000-06-30T23:59:60Z", 962409600L},

    /* Test invalid dates/times. */
    GMT("2000-00", 0),
    GMT("2000-13", 0),
    DATE("2000-01-00", 0),
    DATE("2000-01-32", 0),
    DATE("2012-04-31", 0),
    FRACTION("2000-01-01T25", 0, 0),
    ZONED("2000-01-01T24.05", 0),
    FRACTION("2000-01-01T24:01", 0, 0),
    FRACTION("2000-01-01T24:00:01", 0, 0),
    ZONED("2000-01-01T24:00:00.1", 0),
    FRACTION("2000-01-01T00:60", 0, 0),
    FRACTION("2000-01-01T00:00:61", 0, 0),
    {"2000-01-01T00:00:00+25"},
    {"2000-01-01T00:00:00-25"},
    {"2000-01-01T00:00:00+00:60"},
    {"2000-01-01T00:00:00-00:60"},
    {"2000-367T00:00:00Z"},
    {"2000-W53-1T00:00:00Z"},
    {"2000-W52-8T00:00:00Z"},
    {"2000-01-01T00:00:00Z0"},
    {"2000+01+01T00:00:00Z"},
    {"2000!01!01T00:00:00Z"},
    {"2000-01-01T00:00:00.0.0Z"},
    {"2000-01-W01T00:00:00Z"},
    {""},
    {"20000000000000000000000000000000000000000000000000000000000000000000000"},
    {"W2000-01-01"},
    {"2000--W"},


    /* Test leap year. */
    DATE("1989-02-29", 0),
    DATE("1988-02-29", 573109200L),

    {},
};

static bool
is_ext_year(const char *iso8601)
{
    if (iso8601[0] == '-')
        return true;

    if (iso8601[0] == '+')
        return true;

    return false;
}

static void
test_one(const char *iso8601, time_t expected)
{
    iso8601_time time;
    time_t tmp = 0;
    int err;

    fprintf(stderr, "string: %s\n", iso8601);
    fprintf(stderr, "answer: %ld\n", expected);

    err = iso8601_parse(iso8601, &time);
    fprintf(stderr, "return: %d\n", err);
    assert((err == 0) == (expected != 0));
    assert(err == 0 || err == EINVAL);
    if (expected == 0)
        return;

    iso8601_to_time_t(&time, &tmp);
    fprintf(stderr, "result: %ld\n\n", tmp);
    assert(tmp == expected);
}

static void
test(const struct test_data *data)
{
    char basict[1024];
    char basic[1024];
    int i, j;
    const char *all[] = {
        data->string,
        basict,
        data->notrunct ? NULL : basic,
        NULL
    };

    /* Basic mode. */
    strncpy(basict, data->string, sizeof(basict));
    j = i = is_ext_year(basict) ? 1 : 0;
    do {
        basict[j] = basict[i];
        switch (basict[j]) {
        case ':':
            continue;
        case '-':
            /*
             * Handle the weekday case which requires an odd number of digits
             * after the dash. In the most common case, this number will be 0
             * or 1. But if 'T' is not present, it can be all the way to the
             * end of the time section. Take for example, the case of midnight
             * on January first in local time: 1989W011000000
             */
            if (j == 7 && basict[4] == 'W') {
                int digits;
                for (digits = 0; basict[i + digits + 1] != '\0'; digits++) {
                    if (!isdigit(basict[i + digits + 1]))
                        break;
                }
                if (digits % 2 == 1)
                    continue;
            }

            /* - is removed from the date but retained in the TZ. */
            if (j < 7)
                continue;
        }

        j++;
    } while (basict[i++] != '\0');

    /* Basic mode w/o T. */
    strncpy(basic, basict, sizeof(basic));
    j = i = is_ext_year(basic) ? 1 : 0;
    do {
        basic[j] = basic[i];
        if (basic[i] != 'T')
            j++;
    } while (basict[i++] != '\0');

    /* Do all tests. */
    for (i = 0; all[i] != NULL; i++) {
        if (i > 0 && strcmp(all[i], all[i -1 ]) == 0)
            continue; /* Skip duplicates. */

        test_one(all[i], data->value);
    }
}

static char
hex(char v)
{
    if (v < 10)
        return '0' + v;

    return 'A' + v - 10;
}

int
main(int argc, const char **argv)
{
    static const int iter = 50000;
    FILE *rnd = NULL;

    setenv("TZ", TZ, 1);

    assert(iso8601_parse(NULL, NULL) == EINVAL);

    for (int i = 0; TEST_DATA[i].string != NULL; i++)
        test(&TEST_DATA[i]);

    rnd = fopen("/dev/urandom", "r");
    assert(rnd);

    for (int i = 0; i < iter; i++) {
        char buf[64] = {};
        iso8601_time out;
        size_t len;

        len = rand() % (sizeof(buf) - 1);
        assert(fread(buf, 1, len, rnd) == len);

        fprintf(stderr, "Fuzzing: ");
        for (size_t j = 0; j < len; j++) {
            fputc(hex((buf[j] & 0xf0) >> 4), stderr);
            fputc(hex((buf[j] & 0x0f) >> 0), stderr);
        }
        fputc('\n', stderr);

        (void) iso8601_parse(buf, &out);
    }

    fclose(rnd);
    return 0;
}
