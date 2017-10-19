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

#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * Get the length of the given year in days.
 *
 * @return number of days in the year
 */
uint16_t length_year_days(int32_t year);

/**
 * Get the length of the given year in weekdate weeks.
 *
 * @return number of weekdate weeks in the year
 */
uint8_t length_year_weeks(int32_t year);

/**
 * Get the length of the given month in days.
 *
 * @return number of days in the month
 */
uint8_t length_month_days(int32_t year, uint8_t month);

/**
 *  Convert an ordinal into a calendar date.
 *
 *  @return true on success; false on invalid input
 */
bool ordinal_to_date(int32_t year, uint16_t ordinal,
                     uint8_t *month, uint8_t *day);

/**
 * Convert a calendar date into an ordinal.
 *
 * @return true on success; false on invalid input
 */
bool ordinal_from_date(int32_t year, uint8_t month, uint8_t day,
                       uint16_t *ordinal);

/**
 * Convert a weekdate to a calendar date.
 *
 * @return true on success; false on invalid input
 */
bool weekdate_to_date(int32_t wyear, uint8_t week, uint8_t wday,
                      int32_t *year, uint8_t *month, uint8_t *day);

/**
 * Convert a calendar date into a weekdate.
 *
 * @return true on success; false on invalid input
 */
bool weekdate_from_date(int32_t year, uint8_t month, uint8_t day,
                        int32_t *wyear, uint8_t *week, uint8_t *wday);
