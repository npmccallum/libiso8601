/* vim: set tabstop=8 shiftwidth=4 softtabstop=4 expandtab smarttab colorcolumn=80: */
/**
 * Copyright: 2017 Red Hat, Inc.
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
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

struct {
    iso8601_truncate truncate;
    iso8601_format format;
    uint8_t ydigits;
    uint32_t flags;

    iso8601_time time;
    const char *str;
} tests[] = {
    /* All truncations of normal format. */
    { ISO8601_TRUNCATE_NONE, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000-03-03T03:03:03.123456Z" },
    { ISO8601_TRUNCATE_SECOND, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000-03-03T03:03:03Z" },
    { ISO8601_TRUNCATE_MINUTE, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000-03-03T03:03Z" },
    { ISO8601_TRUNCATE_HOUR, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000-03-03T03Z" },
    { ISO8601_TRUNCATE_DAY, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000-03-03" },
    { ISO8601_TRUNCATE_MONTH, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000-03" },
    { ISO8601_TRUNCATE_YEAR, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000" },

    /* All truncations of normal format (basic mode). */
    { ISO8601_TRUNCATE_NONE, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "20000303T030303.123456Z" },
    { ISO8601_TRUNCATE_SECOND, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "20000303T030303Z" },
    { ISO8601_TRUNCATE_MINUTE, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "20000303T0303Z" },
    { ISO8601_TRUNCATE_HOUR, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "20000303T03Z" },
    { ISO8601_TRUNCATE_DAY, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "20000303" },
    { ISO8601_TRUNCATE_MONTH, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "200003" },
    { ISO8601_TRUNCATE_YEAR, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000" },

    /* All truncations of ordinal format. */
    { ISO8601_TRUNCATE_NONE, ISO8601_FORMAT_ORDINAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000-063T03:03:03.123456Z" },
    { ISO8601_TRUNCATE_SECOND, ISO8601_FORMAT_ORDINAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000-063T03:03:03Z" },
    { ISO8601_TRUNCATE_MINUTE, ISO8601_FORMAT_ORDINAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000-063T03:03Z" },
    { ISO8601_TRUNCATE_HOUR, ISO8601_FORMAT_ORDINAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000-063T03Z" },
    { ISO8601_TRUNCATE_DAY, ISO8601_FORMAT_ORDINAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000-063" },
    { ISO8601_TRUNCATE_ORDINAL, ISO8601_FORMAT_ORDINAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000-063" },
    { ISO8601_TRUNCATE_YEAR, ISO8601_FORMAT_ORDINAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000" },

    /* All truncations of ordinal format (basic mode). */
    { ISO8601_TRUNCATE_NONE, ISO8601_FORMAT_ORDINAL, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000063T030303.123456Z" },
    { ISO8601_TRUNCATE_SECOND, ISO8601_FORMAT_ORDINAL, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000063T030303Z" },
    { ISO8601_TRUNCATE_MINUTE, ISO8601_FORMAT_ORDINAL, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000063T0303Z" },
    { ISO8601_TRUNCATE_HOUR, ISO8601_FORMAT_ORDINAL, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000063T03Z" },
    { ISO8601_TRUNCATE_DAY, ISO8601_FORMAT_ORDINAL, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000063" },
    { ISO8601_TRUNCATE_ORDINAL, ISO8601_FORMAT_ORDINAL, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000063" },
    { ISO8601_TRUNCATE_YEAR, ISO8601_FORMAT_ORDINAL, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000" },

    /* All truncations of weekdate format. */
    { ISO8601_TRUNCATE_NONE, ISO8601_FORMAT_WEEKDATE, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000-W09-5T03:03:03.123456Z" },
    { ISO8601_TRUNCATE_SECOND, ISO8601_FORMAT_WEEKDATE, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000-W09-5T03:03:03Z" },
    { ISO8601_TRUNCATE_MINUTE, ISO8601_FORMAT_WEEKDATE, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000-W09-5T03:03Z" },
    { ISO8601_TRUNCATE_HOUR, ISO8601_FORMAT_WEEKDATE, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000-W09-5T03Z" },
    { ISO8601_TRUNCATE_DAY, ISO8601_FORMAT_WEEKDATE, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000-W09-5" },
    { ISO8601_TRUNCATE_MONTH, ISO8601_FORMAT_WEEKDATE, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000-W09" },
    { ISO8601_TRUNCATE_YEAR, ISO8601_FORMAT_WEEKDATE, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000" },

    /* All truncations of weekdate format (basic mode). */
    { ISO8601_TRUNCATE_NONE, ISO8601_FORMAT_WEEKDATE, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000W095T030303.123456Z" },
    { ISO8601_TRUNCATE_SECOND, ISO8601_FORMAT_WEEKDATE, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000W095T030303Z" },
    { ISO8601_TRUNCATE_MINUTE, ISO8601_FORMAT_WEEKDATE, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000W095T0303Z" },
    { ISO8601_TRUNCATE_HOUR, ISO8601_FORMAT_WEEKDATE, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000W095T03Z" },
    { ISO8601_TRUNCATE_DAY, ISO8601_FORMAT_WEEKDATE, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000W095" },
    { ISO8601_TRUNCATE_MONTH, ISO8601_FORMAT_WEEKDATE, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000W09" },
    { ISO8601_TRUNCATE_YEAR, ISO8601_FORMAT_WEEKDATE, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "2000" },

    /* All truncations of normal format with long years. */
    { ISO8601_TRUNCATE_NONE, ISO8601_FORMAT_NORMAL, 6, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "002000-03-03T03:03:03.123456Z" },
    { ISO8601_TRUNCATE_SECOND, ISO8601_FORMAT_NORMAL, 6, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "002000-03-03T03:03:03Z" },
    { ISO8601_TRUNCATE_MINUTE, ISO8601_FORMAT_NORMAL, 6, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "002000-03-03T03:03Z" },
    { ISO8601_TRUNCATE_HOUR, ISO8601_FORMAT_NORMAL, 6, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "002000-03-03T03Z" },
    { ISO8601_TRUNCATE_DAY, ISO8601_FORMAT_NORMAL, 6, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "002000-03-03" },
    { ISO8601_TRUNCATE_MONTH, ISO8601_FORMAT_NORMAL, 6, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "002000-03" },
    { ISO8601_TRUNCATE_YEAR, ISO8601_FORMAT_NORMAL, 6, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "002000" },

    /* All truncations of normal format with long years (no basic mode). */
    { ISO8601_TRUNCATE_NONE, ISO8601_FORMAT_NORMAL, 6, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "002000-03-03T03:03:03.123456Z" },
    { ISO8601_TRUNCATE_SECOND, ISO8601_FORMAT_NORMAL, 6, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "002000-03-03T03:03:03Z" },
    { ISO8601_TRUNCATE_MINUTE, ISO8601_FORMAT_NORMAL, 6, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "002000-03-03T03:03Z" },
    { ISO8601_TRUNCATE_HOUR, ISO8601_FORMAT_NORMAL, 6, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "002000-03-03T03Z" },
    { ISO8601_TRUNCATE_DAY, ISO8601_FORMAT_NORMAL, 6, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "002000-03-03" },
    { ISO8601_TRUNCATE_MONTH, ISO8601_FORMAT_NORMAL, 6, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "002000-03" },
    { ISO8601_TRUNCATE_YEAR, ISO8601_FORMAT_NORMAL, 6, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456 }, "002000" },

    /* All truncations of normal format with negative year. */
    { ISO8601_TRUNCATE_NONE, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { -2000, 3, 3, 3, 3, 3, 123456 }, "-2000-03-03T03:03:03.123456Z" },
    { ISO8601_TRUNCATE_SECOND, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { -2000, 3, 3, 3, 3, 3, 123456 }, "-2000-03-03T03:03:03Z" },
    { ISO8601_TRUNCATE_MINUTE, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { -2000, 3, 3, 3, 3, 3, 123456 }, "-2000-03-03T03:03Z" },
    { ISO8601_TRUNCATE_HOUR, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { -2000, 3, 3, 3, 3, 3, 123456 }, "-2000-03-03T03Z" },
    { ISO8601_TRUNCATE_DAY, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { -2000, 3, 3, 3, 3, 3, 123456 }, "-2000-03-03" },
    { ISO8601_TRUNCATE_MONTH, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { -2000, 3, 3, 3, 3, 3, 123456 }, "-2000-03" },
    { ISO8601_TRUNCATE_YEAR, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { -2000, 3, 3, 3, 3, 3, 123456 }, "-2000" },

    /* All truncations of normal format (basic mode). */
    { ISO8601_TRUNCATE_NONE, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_BASIC,
        { -2000, 3, 3, 3, 3, 3, 123456 }, "-20000303T030303.123456Z" },
    { ISO8601_TRUNCATE_SECOND, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_BASIC,
        { -2000, 3, 3, 3, 3, 3, 123456 }, "-20000303T030303Z" },
    { ISO8601_TRUNCATE_MINUTE, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_BASIC,
        { -2000, 3, 3, 3, 3, 3, 123456 }, "-20000303T0303Z" },
    { ISO8601_TRUNCATE_HOUR, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_BASIC,
        { -2000, 3, 3, 3, 3, 3, 123456 }, "-20000303T03Z" },
    { ISO8601_TRUNCATE_DAY, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_BASIC,
        { -2000, 3, 3, 3, 3, 3, 123456 }, "-20000303" },
    { ISO8601_TRUNCATE_MONTH, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_BASIC,
        { -2000, 3, 3, 3, 3, 3, 123456 }, "-200003" },
    { ISO8601_TRUNCATE_YEAR, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_BASIC,
        { -2000, 3, 3, 3, 3, 3, 123456 }, "-2000" },

    /* Timezones */
    { ISO8601_TRUNCATE_NONE, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456, false, 90 },
        "2000-03-03T03:03:03.123456+01:30" },
    { ISO8601_TRUNCATE_NONE, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456, false, 90 },
        "20000303T030303.123456+0130" },
    { ISO8601_TRUNCATE_DAY, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456, false, 90 },
        "2000-03-03" },
    { ISO8601_TRUNCATE_NONE, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456, false, -90 },
        "2000-03-03T03:03:03.123456-01:30" },
    { ISO8601_TRUNCATE_NONE, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456, false, -90 },
        "20000303T030303.123456-0130" },
    { ISO8601_TRUNCATE_DAY, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456, false, -90 },
        "2000-03-03" },
    { ISO8601_TRUNCATE_NONE, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456, true, 0 },
        "2000-03-03T03:03:03.123456" },
    { ISO8601_TRUNCATE_NONE, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456, true, 90 }, /* Offset is ignored. */
        "2000-03-03T03:03:03.123456" },
    { ISO8601_TRUNCATE_NONE, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_BASIC,
        { 2000, 3, 3, 3, 3, 3, 123456, true, 0 },
        "20000303T030303.123456" },
    { ISO8601_TRUNCATE_DAY, ISO8601_FORMAT_NORMAL, 4, ISO8601_FLAG_NONE,
        { 2000, 3, 3, 3, 3, 3, 123456, true, 0 },
        "2000-03-03" },

    {}
};

static void test_e2big(iso8601_format format)
{
    static const iso8601_time time = { 2000, 3, 3, 3, 3, 3, 123456, false, 90 };
    char tmp[1] = {};

    assert(iso8601_unparse(&time, ISO8601_FLAG_NONE, 4, format,
                           ISO8601_TRUNCATE_NONE, 0, NULL) == EINVAL);

    assert(iso8601_unparse(&time, ISO8601_FLAG_NONE, 4, format,
                           ISO8601_TRUNCATE_NONE, 0, tmp) == E2BIG);

    for (int i = 1, r = E2BIG; r == E2BIG; i++) {
        char buf[i];
        memset(buf, 0, i);
        r = iso8601_unparse(&time, ISO8601_FLAG_NONE, 4, format,
                            ISO8601_TRUNCATE_NONE, i, buf);
        assert(r == 0 || r == E2BIG);
    }
}

int main(int argc, const char **argv)
{
    for (size_t i = 0; tests[i].str; i++) {
        char buf[1024] = {};

        fprintf(stderr, "answer: %s\n", tests[i].str);
        assert(iso8601_unparse(&tests[i].time, tests[i].flags,
                               tests[i].ydigits, tests[i].format,
                               tests[i].truncate, sizeof(buf), buf) == 0);

        fprintf(stderr, "result: %s\n", buf);
        assert(strcmp(buf, tests[i].str) == 0);
    }

    /* Test all the buffer too small conditions. */
    test_e2big(ISO8601_FORMAT_NORMAL);
    test_e2big(ISO8601_FORMAT_ORDINAL);
    test_e2big(ISO8601_FORMAT_WEEKDATE);

    return 0;
}
