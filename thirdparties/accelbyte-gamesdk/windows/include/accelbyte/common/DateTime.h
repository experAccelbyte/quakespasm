// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "String.h"
#include "exports.h"

#include <ctime>

namespace accelbyte {

/**
 * @brief a date time representation object
 **/
class DateTime {
public:
    /**
     * @brief default constructor.
     **/
    ACCELBYTE_COMMON_API DateTime() = default;

    /**
     * @brief copy constructor.
     **/
    ACCELBYTE_COMMON_API DateTime(const DateTime& other);

    /**
     * @brief constructor
     * @param time the time since epoch in SECONDS
     **/
    ACCELBYTE_COMMON_API DateTime(const std::time_t& time);

    /**
     * @brief constructor
     * @param value a string describing the time formated YYYY/MM/DD hh::mm::ss
     **/
    ACCELBYTE_COMMON_API DateTime(const String& value);

    /**
     * @brief constructor
     * @param value a string describing the time
     * @param format the format of the string (see std::get_time)
     **/
    ACCELBYTE_COMMON_API DateTime(const String& value, const String& format);

    /**
     * @brief constructor
     * @param year the year
     * @param month the months of the year - [1, 12]
     * @param day the day of the month - [1, 31]
     * @param hour the hours since midnight - [0, 23]
     * @param minutes the minutes after the hour - [0, 59]
     * @param seconds the seconds after the minute - [0, 59]
     **/
    ACCELBYTE_COMMON_API DateTime(int year, int month, int day, int hour, int minutes, int seconds);

    /**
     * @brief default detructor.
     **/
    ACCELBYTE_COMMON_API  ~DateTime() = default;

    ACCELBYTE_COMMON_API bool operator==(const DateTime& other) const;
    ACCELBYTE_COMMON_API bool operator!=(const DateTime& other) const;
    ACCELBYTE_COMMON_API bool operator<=(const DateTime& other) const;
    ACCELBYTE_COMMON_API bool operator<(const DateTime& other) const;
    ACCELBYTE_COMMON_API bool operator>=(const DateTime& other) const;
    ACCELBYTE_COMMON_API bool operator>(const DateTime& other) const;

    ACCELBYTE_COMMON_API DateTime& operator=(const DateTime& other);
    ACCELBYTE_COMMON_API DateTime operator-(const DateTime& other);
    ACCELBYTE_COMMON_API DateTime& operator-=(const DateTime& other);
    ACCELBYTE_COMMON_API DateTime operator+(const DateTime& other);
    ACCELBYTE_COMMON_API DateTime& operator+=(const DateTime& other);

    /**
     * @brief return the date formated to a string YYYY/MM/DD hh::mm::ss
     **/
    ACCELBYTE_COMMON_API String to_string() const;

    /**
     * @brief return the date formated to a string YYYY/MM/DD hh::mm::ss
     **/
    ACCELBYTE_COMMON_API String to_string(const String& format) const;

    /**
     * @brief extract the date from a sting
     * @param value the date formated YYYY/MM/DD hh::mm::ss
     **/
    ACCELBYTE_COMMON_API bool from_string(const String& value);

    /**
     * @brief extract the date from a sting
     * @param value a string describing the time
     * @param format the format of the string (see std::get_time)
     **/
    ACCELBYTE_COMMON_API bool from_string(const String& value, const String& format);

    ACCELBYTE_COMMON_API std::time_t time_since_epoch() const;

    /**
     * @brief set the date format to be used, default is YYYY/MM/DDThh::mm::ss
     **/
    ACCELBYTE_COMMON_API bool set_format(const String& format);

    ACCELBYTE_COMMON_API int year() const;
    ACCELBYTE_COMMON_API int month() const;
    ACCELBYTE_COMMON_API int day() const;
    ACCELBYTE_COMMON_API int hour() const;
    ACCELBYTE_COMMON_API int minutes() const;
    ACCELBYTE_COMMON_API int seconds() const;

    ACCELBYTE_COMMON_API static DateTime now();

private:
    std::time_t time_point_{0};
    String used_format_{"%Y-%m-%dT%H:%M:%S"};
};

} // namespace accelbyte