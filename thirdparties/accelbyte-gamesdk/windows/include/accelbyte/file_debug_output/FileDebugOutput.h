// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/common/String.h"
#include "accelbyte/debug_handler/DebugOutput.h"
#include "accelbyte/file_debug_output/file_debug_output_exports.h"

// STL
#include <fstream>
#include <mutex>

namespace accelbyte {
namespace file_debug_output {

/**
 * @brief interface to a debug output
 **/
class FileDebugOutput : public debug_handler::DebugOutput {
public:
    AB_CNL_FILE_DEBUG_OUTPUT_EXPORT FileDebugOutput();
    AB_CNL_FILE_DEBUG_OUTPUT_EXPORT explicit FileDebugOutput(String const& path);

    /**
     * @brief default detructor.
     **/
    AB_CNL_FILE_DEBUG_OUTPUT_EXPORT ~FileDebugOutput() override;

    /**
     * @brief execute the request and return the value in form of raw answer. Should handle 300 code
     **/
    AB_CNL_FILE_DEBUG_OUTPUT_EXPORT void output_debug(String const& output) const override;

private:
    void open_file(String const& path);
    mutable std::fstream file_;
    mutable std::mutex mutex_;
    mutable bool first_log_{true};
};

} // namespace file_debug_output
} // namespace accelbyte