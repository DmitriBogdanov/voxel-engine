// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/GSE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Source repo: https://github.com/DmitriBogdanov/voxel-engine
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#pragma once

// _______________________ INCLUDES _______________________

#include "libassert/assert.hpp"
#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/LogFunctions.h"
#include "quill/Logger.h"
#include "quill/sinks/ConsoleSink.h"
#include "quill/sinks/FileSink.h"

// ____________________ DEVELOPER DOCS ____________________

// Header exposing a simple macro-free logging API.
// Uses 'quill' as a high-performance async backend.
// Exposes assertions from 'libassert' (mainly 'ASSERT()') and sends their output to the logger.
// Log formatting follows the 'fmt' notation as it is the formatting backend of 'quill'

// ____________________ IMPLEMENTATION ____________________

namespace rl::log::impl {

inline void assert_failure_handler(const libassert::assertion_info& info);

inline quill::Logger* logger() {

    static quill::Logger* logger = [] {
        // Setup
        libassert::set_failure_handler(assert_failure_handler);

        quill::Backend::start();

        // Sinks
        quill::FileSinkConfig config;
        config.set_open_mode('w');
        config.set_filename_append_option(quill::FilenameAppendOption::StartCustomTimestampFormat,
                                          "_%Y-%m-%d_%H-%M-%S");

        auto file_sink = quill::Frontend::create_or_get_sink<quill::FileSink>("temp/main.log", config);

        auto console_sink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>("console_sink");
        console_sink->set_log_level_filter(quill::LogLevel::Info);

        // Logger
        quill::PatternFormatterOptions pattern{
            "%(time) | %(thread_id) | %(short_source_location:<20) | %(log_level:<5) | %(message)", "%H:%M:%S.%Qms"};

        return quill::Frontend::create_or_get_logger("main", {std::move(console_sink), std::move(file_sink)}, pattern);
    }();

    return logger;
}

template <class... Args>
void trace(char const* fmt, Args&&... args) {
    quill::tracel1(logger(), fmt, std::forward<Args>(args)...);
}

template <class... Args>
void info(char const* fmt, Args&&... args) {
    quill::info(logger(), fmt, std::forward<Args>(args)...);
}

template <class... Args>
void note(char const* fmt, Args&&... args) {
    quill::notice(logger(), fmt, std::forward<Args>(args)...);
}

template <class... Args>
void warn(char const* fmt, Args&&... args) {
    quill::warning(logger(), fmt, std::forward<Args>(args)...);
}

template <class... Args>
void error(char const* fmt, Args&&... args) {
    quill::error(logger(), fmt, std::forward<Args>(args)...);
}

inline void assert_failure_handler(const libassert::assertion_info& info) {
    error("-----------------------\n{0}\n-----------------------", info.to_string(0, libassert::color_scheme::blank));
    logger()->flush_log();
    libassert::default_failure_handler(info);
}

} // namespace rl::log::impl

// ______________________ PUBLIC API ______________________

namespace rl::log {

using impl::trace;
using impl::info;
using impl::note;
using impl::warn;
using impl::error;

} // namespace rl::log