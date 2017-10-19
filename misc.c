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

#include <assert.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

int iso8601_current(bool localtime, int16_t tzminutes, iso8601_time *out)
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) != 0)
        return errno;

    iso8601_from_timeval(&tv, localtime, tzminutes, out);
    return 0;
}

static void normalize(iso8601_time *time)
{
    /* Normalize where hour == 24. */
    iso8601_add_hours(time, 0);

    /* Normalize timezones. */
    if (!time->localtime) {
        iso8601_add_minutes(time, time->tzminutes * -1);
        time->tzminutes = 0;
    }

    /* Don't normalize leap seconds! */
}

static int compare(const iso8601_time *a, const iso8601_time *b)
{
    iso8601_time tmpa = *a;
    iso8601_time tmpb = *b;
    int diff;

    normalize(&tmpa);
    normalize(&tmpb);

    diff = tmpa.year - tmpb.year;
    if (diff != 0)
        return diff;

    diff = tmpa.month - tmpb.month;
    if (diff != 0)
        return diff;

    diff = tmpa.day - tmpb.day;
    if (diff != 0)
        return diff;

    diff = tmpa.hour - tmpb.hour;
    if (diff != 0)
        return diff;

    diff = tmpa.minute - tmpb.minute;
    if (diff != 0)
        return diff;

    diff = tmpa.second - tmpb.second;
    if (diff != 0)
        return diff;

    return tmpa.usecond - tmpb.usecond;
}

static int compare_tz(const iso8601_time *a, const iso8601_time *b)
{
    struct timeval tva, tvb;
#if SIZEOF_TIME_T <= 4
    uint16_t oa = 1, ob = 1;
    int diff;

    /*
     * Attempt to compare dates without comparing timezones.
     *
     * This bit of code is for platforms with a small time_t value. We try
     * to avoid calling iso8601_to_timeval() in this case because the
     * result might be an overflow. :( This problem is well known.
     */

    /* If there is more than one year difference, we can compare. */
    diff = a->year - b->year;
    if (abs(diff) > 1)
        return diff;

    /* If there is more than two days difference, we can compare. */
    ordinal_from_date(a->year, a->month, a->day, &oa);
    ordinal_from_date(b->year, b->month, b->day, &ob);
    if (a->year > b->year)
        oa += length_year_days(a->year);
    else if (a->year < b->year)
        ob += length_year_days(b->year);
    diff = oa - ob;
    if (abs(diff) > 2)
        return diff;

    /* Let iso8601_to_timeval() resolve the timezones... */
#endif

    iso8601_to_timeval(a, &tva);
    iso8601_to_timeval(b, &tvb);
    if (tva.tv_sec < tvb.tv_sec)
        return -1;
    else if (tva.tv_sec > tvb.tv_sec)
        return 1;
    else if (tva.tv_usec < tvb.tv_usec)
        return -1;
    else if (tva.tv_usec > tvb.tv_usec)
        return 1;

    return 0;
}

int iso8601_compare(const iso8601_time *a, const iso8601_time *b)
{
    if (a->localtime != b->localtime)
        return compare_tz(a, b);

    return compare(a, b);
}

void iso8601_to_tm(const iso8601_time *time, struct tm *tm)
{
    tm->tm_year = time->year - 1900;
    tm->tm_mon = time->month - 1;
    tm->tm_mday = time->day;
    tm->tm_hour = time->hour;
    tm->tm_min = time->minute;
    tm->tm_sec = time->second;
    tm->tm_isdst = time->localtime ? -1 : 0;
}

void iso8601_from_tm(const struct tm *tm, uint32_t usecond, bool localtime,
                     int16_t tzminutes, iso8601_time *time)
{
    time->year = tm->tm_year + 1900;
    time->month = tm->tm_mon + 1;
    time->day = tm->tm_mday;
    time->hour = tm->tm_hour;
    time->minute = tm->tm_min;
    time->second = tm->tm_sec;
    time->usecond = usecond;
    time->localtime = localtime;
    time->tzminutes = tzminutes;
}

void iso8601_to_timeval(const iso8601_time *time, struct timeval *tv)
{
    struct tm tm;
    iso8601_to_tm(time, &tm);

    tv->tv_usec = time->usecond;
    tv->tv_sec = mktime(&tm);
    if (!time->localtime)
        tv->tv_sec = timegm(&tm) - time->tzminutes * 60;
}

void iso8601_from_timeval(const struct timeval *tv, bool localtime,
                          int16_t tzminutes, iso8601_time *time)
{
    time_t seconds = tv->tv_sec;
    struct tm tm;

    /* FIXME: What is the error contract of these functions? */
    if (localtime) {
        assert(localtime_r(&seconds, &tm) != NULL);
    } else {
        seconds += tzminutes * 60;
        assert(gmtime_r(&seconds, &tm) != NULL);
    }

    iso8601_from_tm(&tm, tv->tv_usec, localtime, tzminutes, time);
}

void iso8601_to_time_t(const iso8601_time *time, time_t *timet)
{
    struct timeval tv;

    iso8601_to_timeval(time, &tv);
    *timet = tv.tv_sec;
}

void iso8601_from_time_t(time_t timet, uint32_t usecond, bool localtime,
                         int16_t tzminutes, iso8601_time *time)
{
    struct timeval tv = { .tv_sec = timet, .tv_usec = usecond };
    iso8601_from_timeval(&tv, localtime, tzminutes, time);
}
