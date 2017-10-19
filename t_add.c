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
#include "internal.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

struct xform {
    iso8601_time before;
    int increment;
    iso8601_time after;
};

static const struct xform years[] = {
    {{2000, 1, 1},     1, {2001, 1, 1}},
    {{2000, 1, 1},     2, {2002, 1, 1}},
    {{2000, 1, 1},  2000, {4000, 1, 1}},
    {{2000, 1, 1},  2001, {4001, 1, 1}},
    {{2000, 1, 1},    -1, {1999, 1, 1}},
    {{2000, 1, 1},    -2, {1998, 1, 1}},
    {{2000, 1, 1}, -2000, {   0, 1, 1}},
    {{2000, 1, 1}, -2001, {  -1, 1, 1}},
};

static const struct xform months[] = {
#define XX(year) \
    {{year, 1, 1},   0, {year,    1, 1}}, \
    {{year, 1, 1},   1, {year,    2, 1}}, \
    {{year, 1, 1},  -1, {year-1, 12, 1}}, \
    {{year, 1, 1},  12, {year+1,  1, 1}}, \
    {{year, 1, 1}, -12, {year-1,  1, 1}}
    XX(2000),
    XX(1979),
    XX(0),
#undef XX
    {{2000, 13, 1}, 0, {2001,  1, 1}},
    {{2000, 25, 1}, 0, {2002,  1, 1}},
};

static const struct xform days[] = {
#define XX(year) \
    {{year,    1,  1},   0, {year,    1,  1}}, \
    {{year,    1,  1},   1, {year,    1,  2}}, \
    {{year-1, 12, 31},   1, {year,    1,  1}}, \
    {{year,    1,  1},  -1, {year-1, 12, 31}}, \
    {{year,    1,  1},  31, {year,    2,  1}}, \
    {{year,    1,  1}, -31, {year-1, 12,  1}}, \
    {{year-1, 12,  1},  31, {year,    1,  1}}
    XX(2000),
    XX(1979),
    XX(0),
#undef XX
#define XX(year, month, last) \
    {{year, month, last},   1, {year, month + 1, 1}}, \
    {{year, month + 1, 1}, -1, {year, month, last}}
    XX(2000,  1, 31),
    XX(2000,  3, 31),
    XX(2000,  4, 30),
    XX(2000,  5, 31),
    XX(2000,  6, 30),
    XX(2000,  7, 31),
    XX(2000,  8, 31),
    XX(2000,  9, 30),
    XX(2000, 10, 31),
    XX(2000, 11, 30),
#undef XX
#define LEAP(year) ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
#define XX(year) \
    {{year, 3, 1}, -1, {year, 2, LEAP(year) ? 29 : 28}}, \
    {{year, 2, 28}, 1, {year, LEAP(year) ? 2 : 3, LEAP(year) ? 29 : 1}}
    XX(1996),
    XX(1900),
    XX(2000),
#undef XX
#undef LEAP
};

static const struct xform hours[] = {
    /* Verify that normalization works. */
    {{2000, 1,  1,  0},   0, {2000,  1,  1, 0}},
    {{2000, 1,  1, 24},   0, {2000,  1,  2, 0}},
    {{2000, 1,  1, 25},   0, {2000,  1,  2, 1}},
    {{2000, 1,  1, 49},   0, {2000,  1,  3, 1}},

    /* Verify basic addition and subtraction. */
    {{2000, 1,  1,  0},   1, {2000,  1,  1,  1}},
    {{2000, 1,  1,  0},  24, {2000,  1,  2,  0}},
    {{2000, 1,  1,  0},  -1, {1999, 12, 31, 23}},
    {{2000, 1,  1,  0}, -24, {1999, 12, 31,  0}},

    /* Verify addition and subtraction over non-leap days. */
#define XX(year) \
    {{year, 3,  1,  0},  -1, {year,  2, 28, 23}}, \
    {{year, 2, 28, 23},   1, {year,  3, 1,   0}}
    XX(1999),
    XX(1900),
#undef XX

    /* Verify addition and subtraction over leap days. */
#define XX(year) \
    {{year, 2, 28, 23},   1, {year,  2, 29,  0}}, \
    {{year, 2, 29, 23},   1, {year,  3, 1,   0}}, \
    {{year, 2, 28, 23},  25, {year,  3, 1,   0}}, \
    {{year, 3,  1,  0},  -1, {year,  2, 29, 23}}, \
    {{year, 2, 29,  0},  -1, {year,  2, 28, 23}}, \
    {{year, 3,  1,  0}, -25, {year,  2, 28, 23}}
    XX(1996),
    XX(2000),
#undef XX
};

static const struct xform minutes[] = {
    /* Verify that normalization works. */
    {{2000,  1,  1,  0,   0},   0, {2000,  1,  1,  0,  0}},
    {{2000,  1,  1,  0,  60},   0, {2000,  1,  1,  1,  0}},
    {{2000,  1,  1,  0, 120},   0, {2000,  1,  1,  2,  0}},

    /* Verify basic addition and subtraction. */
    {{2000,  1,  1,  0,   0},   1, {2000,  1,  1,  0,  1}},
    {{2000,  1,  1,  0,   0},  60, {2000,  1,  1,  1,  0}},
    {{2000,  1,  1,  0,   0},  -1, {1999, 12, 31, 23, 59}},
    {{2000,  1,  1,  0,   0}, -60, {1999, 12, 31, 23,  0}},
};

static const struct xform seconds[] = {
    /* Verify that normalization works. */
    {{2000,  1,  1,  0,  0,   0},   0, {2000,  1,  1,  0,  0,  0}},
    {{2000,  1,  1,  0,  0,  60},   0, {2000,  1,  1,  0,  1,  0}},
    {{2000,  1,  1,  0,  0, 120},   0, {2000,  1,  1,  0,  2,  0}},

    /* Verify basic addition and subtraction. */
    {{2000,  1,  1,  0,  0,   0},   1, {2000,  1,  1,  0,  0,  1}},
    {{2000,  1,  1,  0,  0,   0},  60, {2000,  1,  1,  0,  1,  0}},
    {{2000,  1,  1,  0,  0,   0},  -1, {1999, 12, 31, 23, 59, 59}},
    {{2000,  1,  1,  0,  0,   0}, -60, {1999, 12, 31, 23, 59,  0}},
};

static const struct xform useconds[] = {
    /* Verify that normalization works. */
    {{2000,  1,  1, .usecond = 0},       0, {2000,  1,  1}},
    {{2000,  1,  1, .usecond = 1000000}, 0, {2000,  1,  1, .second = 1}},
    {{2000,  1,  1, .usecond = 2000000}, 0, {2000,  1,  1, .second = 2}},

    /* Verify basic addition and subtraction. */
    {{2000,  1,  1},        1, {2000,  1,  1,  0,  0,  0,      1}},
    {{2000,  1,  1},  1000000, {2000,  1,  1,  0,  0,  1,      0}},
    {{2000,  1,  1},  2000000, {2000,  1,  1,  0,  0,  2,      0}},
    {{2000,  1,  1},       -1, {1999, 12, 31, 23, 59, 59, 999999}},
    {{2000,  1,  1}, -1000000, {1999, 12, 31, 23, 59, 59,      0}},
    {{2000,  1,  1}, -2000000, {1999, 12, 31, 23, 59, 58,      0}},
};

static const struct {
    void (*afunc)(iso8601_time *, int);
    const struct xform *xform;
    const char *name;
    size_t count;
} tests[] = {
#define NITEMS(array) (sizeof(array) / sizeof(*array))
#define XX(name) { iso8601_add_ ## name, name, # name, NITEMS(name) }
    XX(years),
    XX(months),
    XX(days),
    XX(hours),
    XX(minutes),
    XX(seconds),
    XX(useconds),
#undef XX
#undef NITEMS
    {}
};

int main(int argc, const char **argv)
{
    for (size_t i = 0; tests[i].name; i++) {
        fprintf(stderr, "TEST: %s\n", tests[i].name);
        for (size_t j = 0; j < tests[i].count; j++) {
            iso8601_time time = tests[i].xform[j].before;
            char before[128] = {};
            char answer[128] = {};
            char result[128] = {};

            tests[i].afunc(&time, tests[i].xform[j].increment);

            iso8601_unparse(&tests[i].xform[j].before, ISO8601_FLAG_NONE, 4,
                            ISO8601_FORMAT_NORMAL, ISO8601_TRUNCATE_NONE,
                            sizeof(before), before);
            iso8601_unparse(&tests[i].xform[j].after, ISO8601_FLAG_NONE, 4,
                            ISO8601_FORMAT_NORMAL, ISO8601_TRUNCATE_NONE,
                            sizeof(answer), answer);
            iso8601_unparse(&time, ISO8601_FLAG_NONE, 4, ISO8601_FORMAT_NORMAL,
                            ISO8601_TRUNCATE_NONE, sizeof(result), result);

            fprintf(stderr, "increment: %d\n", tests[i].xform[j].increment);
            fprintf(stderr, "before: %s\n", before);
            fprintf(stderr, "answer: %s\n", answer);
            fprintf(stderr, "result: %s\n\n", result);

            assert(memcmp(&time, &tests[i].xform[j].after, sizeof(time)) == 0);
        }
    }
}
