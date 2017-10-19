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
#include "internal.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

static bool validate(const iso8601_time *in)
{
    if (in == NULL)
        return false;

    /* Date */
    if (in->year < -99999 || in->year > 99999)
        return false;

    if (in->month < 1 || in->month > 12)
        return false;

    if (in->day < 1 || in->day > length_month_days(in->year, in->month))
        return false;

    /* Time */
    if (in->hour > 24)
        return false;

    if (in->hour == 24 && (in->minute != 0 ||
                           in->second != 0 ||
                           in->usecond != 0))
        return false;

    if (in->minute > 59 || in->second > 60 || in->usecond > 999999)
        return false;

    if (!in->localtime && in->tzminutes > 24 * 60)
        return false;

    return true;
}

static bool unparse_year(int32_t year, uint8_t ydigits, size_t len, char *out)
{
    const char *sign = "";
    char fmt[] = "%s%0Xd";

    /* Create the format string. */
    if (ydigits < 2 || ydigits > 9)
        return false;
    fmt[4] = '0' + ydigits;

    /* Figure out the sign. */
    if (year < 0)
        sign = "-";
    else if (year > 9999)
        sign = "+";

    return snprintf(out, len, fmt, sign, abs(year))
                >= (ssize_t) (ydigits + strlen(sign));
}

static bool concat(char *out, size_t len, ssize_t chars, const char *fmt, ...)
{
    size_t size;
    va_list ap;
    int ret;

    size = strlen(out);

    va_start(ap, fmt);
    ret = vsnprintf(out + size, len - size, fmt, ap);
    va_end(ap);

    return ret >= chars;
}

int iso8601_unparse(const iso8601_time *in, uint32_t flags, uint8_t ydigits,
                    iso8601_format format, iso8601_truncate truncate,
                    size_t len, char *out)
{
    const bool basic = (flags & ISO8601_FLAG_BASIC) && ydigits == 4;
    const char *tsep = basic ? "" : ":";
    const char *dsep = basic ? "" : "-";

    /* Validate input. */
    if (!validate(in))
        return EINVAL;
    if (ydigits < 2 || ydigits > 9)
        return EINVAL;
    if (out == NULL)
        return EINVAL;
    out[0] = '\0';

    /* Write the date. */
    switch (format) {
    case ISO8601_FORMAT_NORMAL:
        if (!unparse_year(in->year, ydigits, len, out))
            return E2BIG;
        if (truncate == ISO8601_TRUNCATE_YEAR)
            return 0;

        if (!concat(out, len, basic ? 2 : 3, "%s%02hhu", dsep, in->month))
            return E2BIG;
        if (truncate == ISO8601_TRUNCATE_MONTH)
            return 0;

        if (!concat(out, len, basic ? 2 : 3, "%s%02hhu", dsep, in->day))
            return E2BIG;
        break;

    case ISO8601_FORMAT_WEEKDATE: {
        int32_t year;
        uint8_t week;
        uint8_t day;

        if (!weekdate_from_date(in->year, in->month, in->day,
                                &year, &week, &day))
            return EINVAL;

        if (!unparse_year(in->year, ydigits, len, out))
            return E2BIG;
        if (truncate == ISO8601_TRUNCATE_YEAR)
            return 0;

        if (!concat(out, len, basic ? 3 : 4, "%sW%02hhu", dsep, week))
            return E2BIG;
        if (truncate == ISO8601_TRUNCATE_WEEK)
            return 0;

        if (!concat(out, len, basic ? 1 :2, "%s%hhu", dsep, day))
            return E2BIG;
        break;
    }

    case ISO8601_FORMAT_ORDINAL: {
        uint16_t ordinal;

        if (!ordinal_from_date(in->year, in->month, in->day, &ordinal))
            return EINVAL;

        if (!unparse_year(in->year, ydigits, len, out))
            return E2BIG;
        if (truncate == ISO8601_TRUNCATE_YEAR)
            return 0;

        if (!concat(out, len, basic ? 3 : 4, "%s%03hu", dsep, ordinal))
            return E2BIG;
        if (truncate == ISO8601_TRUNCATE_YEAR ||
            truncate == ISO8601_TRUNCATE_MONTH)
            return 0;
        break;
    }

    default:
        return EINVAL;
    }
    if (truncate == ISO8601_TRUNCATE_DAY)
        return 0;

    /* Write the time. */
    if (!concat(out, len, 3, "T%02hhu", in->hour))
        return E2BIG;
    if (truncate != ISO8601_TRUNCATE_HOUR) {
        if (!concat(out, len, basic ? 2 : 3, "%s%02hhu", tsep, in->minute))
            return E2BIG;
        if (truncate != ISO8601_TRUNCATE_MINUTE) {
            if (!concat(out, len, basic ? 2 : 3, "%s%02hhu", tsep, in->second))
                return E2BIG;
            if (truncate != ISO8601_TRUNCATE_SECOND && in->usecond != 0) {
                if (!concat(out, len, 7, ".%06u", in->usecond))
                    return E2BIG;
            }
        }
    }

    /* Write the timezone. */
    if (in->localtime)
        return 0;

    if (in->tzminutes == 0)
        return concat(out, len, 1, "%s", "Z") ? 0 : E2BIG;

    if (!concat(out, len, 3, "%s%02hhu",
                in->tzminutes > 0 ? "+" : "-",
                in->tzminutes / 60))
        return E2BIG;
    if (!basic || in->tzminutes % 60 != 0) {
        if (!concat(out, len, basic ? 2 : 3, "%s%02hhu",
                    tsep, in->tzminutes % 60))
            return E2BIG;
    }

    return 0;
}
