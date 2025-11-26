// =============================================================================
// NovaCore Engine - Result Type (Error Handling)
// =============================================================================
// Platform: NovaForge | Engine: NovaCore
// Company: WeNova Interactive (operating as Kayden Shawn Massengill)
//
// This file provides the Result<T, E> type for error handling without
// exceptions. Based on std::expected (C++23) with NovaCore-specific extensions.
//
// Design Goals:
// - No exceptions (exceptions disabled for performance)
// - Type-safe error handling
// - Zero-cost abstractions
// - Composable error propagation
// =============================================================================

#pragma once

#include <expected>
#include <string>
#include <string_view>
#include <source_location>
#include <utility>
#include <variant>
#include <type_traits>
#include <format>

#include "nova/core/types/types.hpp"

namespace nova {

// =============================================================================
// Error Categories
// =============================================================================

/// @brief Error category enumeration
enum class ErrorCategory : u8 {
    None = 0,           ///< No error
    System,             ///< System/OS level error
    Memory,             ///< Memory allocation error
    IO,                 ///< Input/output error
    Parse,              ///< Parsing error
    Validation,         ///< Validation error
    NotFound,           ///< Resource not found
    AlreadyExists,      ///< Resource already exists
    Permission,         ///< Permission denied
    Timeout,            ///< Operation timed out
    Cancelled,          ///< Operation was cancelled
    NotSupported,       ///< Operation not supported
    InvalidArgument,    ///< Invalid argument provided
    OutOfRange,         ///< Value out of valid range
    Logic,              ///< Logic/programming error
    Runtime,            ///< Runtime error
    Graphics,           ///< Graphics/rendering error
    Audio,              ///< Audio system error
    Network,            ///< Networking error
    Asset,              ///< Asset loading error
    Script,             ///< Scripting error
    Physics,            ///< Physics system error
    Unknown = 255       ///< Unknown error
};

/// @brief Convert error category to string
[[nodiscard]] constexpr std::string_view errorCategoryToString(ErrorCategory category) noexcept {
    switch (category) {
        case ErrorCategory::None: return "None";
        case ErrorCategory::System: return "System";
        case ErrorCategory::Memory: return "Memory";
        case ErrorCategory::IO: return "IO";
        case ErrorCategory::Parse: return "Parse";
        case ErrorCategory::Validation: return "Validation";
        case ErrorCategory::NotFound: return "NotFound";
        case ErrorCategory::AlreadyExists: return "AlreadyExists";
        case ErrorCategory::Permission: return "Permission";
        case ErrorCategory::Timeout: return "Timeout";
        case ErrorCategory::Cancelled: return "Cancelled";
        case ErrorCategory::NotSupported: return "NotSupported";
        case ErrorCategory::InvalidArgument: return "InvalidArgument";
        case ErrorCategory::OutOfRange: return "OutOfRange";
        case ErrorCategory::Logic: return "Logic";
        case ErrorCategory::Runtime: return "Runtime";
        case ErrorCategory::Graphics: return "Graphics";
        case ErrorCategory::Audio: return "Audio";
        case ErrorCategory::Network: return "Network";
        case ErrorCategory::Asset: return "Asset";
        case ErrorCategory::Script: return "Script";
        case ErrorCategory::Physics: return "Physics";
        case ErrorCategory::Unknown: return "Unknown";
    }
    return "Unknown";
}

// =============================================================================
// Error Type
// =============================================================================

/// @brief Rich error type with category, message, and source location
class Error {
public:
    /// Default constructor (no error)
    constexpr Error() noexcept
        : m_category(ErrorCategory::None)
        , m_code(0)
        , m_message()
        , m_location()
    {}
    
    /// Construct with category and message
    Error(ErrorCategory category, 
          std::string_view message,
          std::source_location location = std::source_location::current())
        : m_category(category)
        , m_code(0)
        , m_message(message)
        , m_location(location)
    {}
    
    /// Construct with category, code, and message
    Error(ErrorCategory category,
          i32 code,
          std::string_view message,
          std::source_location location = std::source_location::current())
        : m_category(category)
        , m_code(code)
        , m_message(message)
        , m_location(location)
    {}
    
    /// Get error category
    [[nodiscard]] constexpr ErrorCategory category() const noexcept {
        return m_category;
    }
    
    /// Get error code
    [[nodiscard]] constexpr i32 code() const noexcept {
        return m_code;
    }
    
    /// Get error message
    [[nodiscard]] const std::string& message() const noexcept {
        return m_message;
    }
    
    /// Get source location where error occurred
    [[nodiscard]] const std::source_location& location() const noexcept {
        return m_location;
    }
    
    /// Check if this represents an actual error
    [[nodiscard]] constexpr bool isError() const noexcept {
        return m_category != ErrorCategory::None;
    }
    
    /// Explicit conversion to bool (true if error)
    [[nodiscard]] constexpr explicit operator bool() const noexcept {
        return isError();
    }
    
    /// Format error for display/logging
    [[nodiscard]] std::string format() const {
        if (!isError()) {
            return "No error";
        }
        
        return std::format("[{}:{}] {} (code: {}) at {}:{}:{}",
            errorCategoryToString(m_category),
            static_cast<int>(m_category),
            m_message,
            m_code,
            m_location.file_name(),
            m_location.line(),
            m_location.column()
        );
    }
    
    /// Comparison operators
    [[nodiscard]] bool operator==(const Error& other) const noexcept {
        return m_category == other.m_category && m_code == other.m_code;
    }
    
    [[nodiscard]] bool operator!=(const Error& other) const noexcept {
        return !(*this == other);
    }
    
private:
    ErrorCategory m_category;
    i32 m_code;
    std::string m_message;
    std::source_location m_location;
};

// =============================================================================
// Error Factory Functions
// =============================================================================

namespace errors {
    /// Create a system error
    [[nodiscard]] inline Error system(std::string_view message, i32 code = 0,
                                       std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::System, code, message, loc};
    }
    
    /// Create a memory error
    [[nodiscard]] inline Error memory(std::string_view message, i32 code = 0,
                                       std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::Memory, code, message, loc};
    }
    
    /// Create an IO error
    [[nodiscard]] inline Error io(std::string_view message, i32 code = 0,
                                   std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::IO, code, message, loc};
    }
    
    /// Create a parse error
    [[nodiscard]] inline Error parse(std::string_view message, i32 code = 0,
                                      std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::Parse, code, message, loc};
    }
    
    /// Create a validation error
    [[nodiscard]] inline Error validation(std::string_view message, i32 code = 0,
                                           std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::Validation, code, message, loc};
    }
    
    /// Create a not found error
    [[nodiscard]] inline Error notFound(std::string_view message, i32 code = 0,
                                         std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::NotFound, code, message, loc};
    }
    
    /// Create an already exists error
    [[nodiscard]] inline Error alreadyExists(std::string_view message, i32 code = 0,
                                              std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::AlreadyExists, code, message, loc};
    }
    
    /// Create a permission error
    [[nodiscard]] inline Error permission(std::string_view message, i32 code = 0,
                                           std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::Permission, code, message, loc};
    }
    
    /// Create a timeout error
    [[nodiscard]] inline Error timeout(std::string_view message, i32 code = 0,
                                        std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::Timeout, code, message, loc};
    }
    
    /// Create a cancelled error
    [[nodiscard]] inline Error cancelled(std::string_view message, i32 code = 0,
                                          std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::Cancelled, code, message, loc};
    }
    
    /// Create a not supported error
    [[nodiscard]] inline Error notSupported(std::string_view message, i32 code = 0,
                                             std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::NotSupported, code, message, loc};
    }
    
    /// Create an invalid argument error
    [[nodiscard]] inline Error invalidArgument(std::string_view message, i32 code = 0,
                                                std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::InvalidArgument, code, message, loc};
    }
    
    /// Create an out of range error
    [[nodiscard]] inline Error outOfRange(std::string_view message, i32 code = 0,
                                           std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::OutOfRange, code, message, loc};
    }
    
    /// Create a logic error
    [[nodiscard]] inline Error logic(std::string_view message, i32 code = 0,
                                      std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::Logic, code, message, loc};
    }
    
    /// Create a runtime error
    [[nodiscard]] inline Error runtime(std::string_view message, i32 code = 0,
                                        std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::Runtime, code, message, loc};
    }
    
    /// Create a graphics error
    [[nodiscard]] inline Error graphics(std::string_view message, i32 code = 0,
                                         std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::Graphics, code, message, loc};
    }
    
    /// Create an audio error
    [[nodiscard]] inline Error audio(std::string_view message, i32 code = 0,
                                      std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::Audio, code, message, loc};
    }
    
    /// Create a network error
    [[nodiscard]] inline Error network(std::string_view message, i32 code = 0,
                                        std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::Network, code, message, loc};
    }
    
    /// Create an asset error
    [[nodiscard]] inline Error asset(std::string_view message, i32 code = 0,
                                      std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::Asset, code, message, loc};
    }
    
    /// Create a script error
    [[nodiscard]] inline Error script(std::string_view message, i32 code = 0,
                                       std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::Script, code, message, loc};
    }
    
    /// Create a physics error
    [[nodiscard]] inline Error physics(std::string_view message, i32 code = 0,
                                        std::source_location loc = std::source_location::current()) {
        return Error{ErrorCategory::Physics, code, message, loc};
    }
} // namespace errors

// =============================================================================
// Result Type Alias
// =============================================================================

/// @brief Result type for operations that may fail
/// @tparam T Success value type
/// @tparam E Error type (default: Error)
/// 
/// Usage:
/// @code
/// Result<int> divide(int a, int b) {
///     if (b == 0) {
///         return std::unexpected(errors::invalidArgument("Division by zero"));
///     }
///     return a / b;
/// }
/// 
/// auto result = divide(10, 2);
/// if (result) {
///     std::cout << "Result: " << *result << std::endl;
/// } else {
///     std::cout << "Error: " << result.error().format() << std::endl;
/// }
/// @endcode
template<typename T, typename E = Error>
using Result = std::expected<T, E>;

/// @brief Void result type for operations that only fail or succeed
using VoidResult = Result<void, Error>;

/// @brief Unexpected value factory for Result
/// @param error Error to wrap
/// @return std::unexpected with the error
template<typename E>
[[nodiscard]] constexpr auto fail(E&& error) noexcept {
    return std::unexpected(std::forward<E>(error));
}

/// @brief Create a successful void result
/// @return Successful VoidResult
[[nodiscard]] constexpr VoidResult ok() noexcept {
    return VoidResult{};
}

/// @brief Create a successful result with value
/// @param value Success value
/// @return Result containing the value
template<typename T>
[[nodiscard]] constexpr Result<std::decay_t<T>> ok(T&& value) noexcept {
    return Result<std::decay_t<T>>{std::forward<T>(value)};
}

// =============================================================================
// Result Helper Macros
// =============================================================================

/// @brief Propagate error if result is an error, otherwise unwrap value
/// Usage: auto value = NOVA_TRY(someOperation());
#define NOVA_TRY(expr) \
    ({ \
        auto&& _nova_result = (expr); \
        if (!_nova_result) { \
            return ::std::unexpected(_nova_result.error()); \
        } \
        std::move(*_nova_result); \
    })

/// @brief Propagate error if result is an error (for void results)
#define NOVA_TRY_VOID(expr) \
    do { \
        auto&& _nova_result = (expr); \
        if (!_nova_result) { \
            return ::std::unexpected(_nova_result.error()); \
        } \
    } while (false)

/// @brief Return error if condition is false
#define NOVA_ENSURE(condition, error) \
    do { \
        if (!(condition)) { \
            return ::std::unexpected(error); \
        } \
    } while (false)

// =============================================================================
// Optional Type Extensions
// =============================================================================

/// @brief Convert optional to result with error if empty
/// @param opt Optional value
/// @param error Error to use if empty
/// @return Result with value or error
template<typename T>
[[nodiscard]] Result<T> optionalToResult(std::optional<T>&& opt, const Error& error) {
    if (opt.has_value()) {
        return std::move(*opt);
    }
    return std::unexpected(error);
}

/// @brief Convert optional to result with error if empty (lvalue reference)
/// @param opt Optional value
/// @param error Error to use if empty
/// @return Result with value or error
template<typename T>
[[nodiscard]] Result<T> optionalToResult(const std::optional<T>& opt, const Error& error) {
    if (opt.has_value()) {
        return *opt;
    }
    return std::unexpected(error);
}

} // namespace nova
