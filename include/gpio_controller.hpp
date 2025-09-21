/**
 * @file gpio_controller.hpp
 * @brief Modern C++20 GPIO control interface for Raspberry Pi 3B
 * 
 * This header provides a type-safe, RAII-based GPIO control interface
 * optimized for embedded Linux deployment on Pi 3B hardware.
 */

#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string_view>
#include <expected>
#include <chrono>
#include <concepts>

namespace embedded::gpio {

/**
 * @brief GPIO pin numbers for Raspberry Pi 3B
 */
enum class PinNumber : std::uint8_t {
    GPIO_2 = 2,   // I2C1 SDA
    GPIO_3 = 3,   // I2C1 SCL
    GPIO_4 = 4,   // GPCLK0
    GPIO_5 = 5,   // General purpose
    GPIO_6 = 6,   // General purpose
    GPIO_7 = 7,   // SPI0 CE1
    GPIO_8 = 8,   // SPI0 CE0
    GPIO_9 = 9,   // SPI0 MISO
    GPIO_10 = 10, // SPI0 MOSI
    GPIO_11 = 11, // SPI0 SCLK
    GPIO_12 = 12, // PWM0
    GPIO_13 = 13, // PWM1
    GPIO_14 = 14, // UART TXD
    GPIO_15 = 15, // UART RXD
    GPIO_16 = 16, // General purpose
    GPIO_17 = 17, // General purpose
    GPIO_18 = 18, // PWM0
    GPIO_19 = 19, // SPI1 MISO
    GPIO_20 = 20, // SPI1 MOSI
    GPIO_21 = 21, // SPI1 SCLK
    GPIO_22 = 22, // General purpose
    GPIO_23 = 23, // General purpose
    GPIO_24 = 24, // General purpose
    GPIO_25 = 25, // General purpose
    GPIO_26 = 26, // General purpose
    GPIO_27 = 27  // General purpose
};

/**
 * @brief GPIO pin direction
 */
enum class Direction {
    INPUT,
    OUTPUT
};

/**
 * @brief GPIO pin value
 */
enum class Value {
    LOW = 0,
    HIGH = 1
};

/**
 * @brief GPIO operation error types
 */
enum class GpioError {
    PIN_NOT_EXPORTED,
    INVALID_DIRECTION,
    READ_FAILED,
    WRITE_FAILED,
    PERMISSION_DENIED,
    DEVICE_BUSY
};

/**
 * @brief Result type for GPIO operations
 */
template<typename T>
using GpioResult = std::expected<T, GpioError>;

/**
 * @brief Convert GPIO error to string description
 */
constexpr std::string_view to_string(GpioError error) noexcept {
    switch (error) {
        case GpioError::PIN_NOT_EXPORTED: return "GPIO pin not exported";
        case GpioError::INVALID_DIRECTION: return "Invalid GPIO direction";
        case GpioError::READ_FAILED: return "Failed to read GPIO value";
        case GpioError::WRITE_FAILED: return "Failed to write GPIO value";
        case GpioError::PERMISSION_DENIED: return "Permission denied accessing GPIO";
        case GpioError::DEVICE_BUSY: return "GPIO device is busy";
        default: return "Unknown GPIO error";
    }
}

/**
 * @brief Concept for GPIO pin numbers
 */
template<typename T>
concept GpioPinType = std::same_as<T, PinNumber> || 
                     (std::integral<T> && sizeof(T) <= sizeof(std::uint8_t));

/**
 * @brief RAII GPIO pin controller for Raspberry Pi 3B
 * 
 * Provides safe, automatic GPIO pin management with modern C++20 features.
 * Automatically exports pins on construction and unexports on destruction.
 */
class GpioPin {
public:
    /**
     * @brief Construct and export a GPIO pin
     * @param pin Pin number to control
     * @param direction Initial direction (input/output)
     */
    explicit GpioPin(PinNumber pin, Direction direction = Direction::INPUT);
    
    /**
     * @brief Destructor - automatically unexports the pin
     */
    ~GpioPin() noexcept;
    
    // Disable copy construction and assignment
    GpioPin(const GpioPin&) = delete;
    GpioPin& operator=(const GpioPin&) = delete;
    
    // Enable move construction and assignment
    GpioPin(GpioPin&& other) noexcept;
    GpioPin& operator=(GpioPin&& other) noexcept;
    
    /**
     * @brief Set GPIO pin direction
     * @param direction New direction (input/output)
     * @return Success or error code
     */
    GpioResult<void> set_direction(Direction direction) noexcept;
    
    /**
     * @brief Get current GPIO pin direction
     * @return Current direction or error code
     */
    GpioResult<Direction> get_direction() const noexcept;
    
    /**
     * @brief Set GPIO pin value (for output pins)
     * @param value Value to set (HIGH/LOW)
     * @return Success or error code
     */
    GpioResult<void> set_value(Value value) noexcept;
    
    /**
     * @brief Get GPIO pin value
     * @return Current pin value or error code
     */
    GpioResult<Value> get_value() const noexcept;
    
    /**
     * @brief Toggle GPIO pin value (for output pins)
     * @return New pin value or error code
     */
    GpioResult<Value> toggle() noexcept;
    
    /**
     * @brief Check if pin is exported and accessible
     * @return true if pin is ready for use
     */
    bool is_ready() const noexcept;
    
    /**
     * @brief Get the pin number
     * @return Pin number
     */
    PinNumber pin_number() const noexcept { return pin_; }
    
    /**
     * @brief Pulse the pin HIGH then LOW
     * @param duration Duration to keep pin HIGH
     * @return Success or error code
     */
    GpioResult<void> pulse(std::chrono::microseconds duration) noexcept;
    
private:
    PinNumber pin_;
    std::filesystem::path gpio_path_;
    bool exported_ = false;
    
    /**
     * @brief Export the GPIO pin to userspace
     * @return Success or error code
     */
    GpioResult<void> export_pin() noexcept;
    
    /**
     * @brief Unexport the GPIO pin from userspace
     */
    void unexport_pin() noexcept;
    
    /**
     * @brief Write value to a GPIO sysfs file
     * @param filename Filename relative to GPIO pin directory
     * @param value Value to write
     * @return Success or error code
     */
    GpioResult<void> write_file(std::string_view filename, std::string_view value) noexcept;
    
    /**
     * @brief Read value from a GPIO sysfs file
     * @param filename Filename relative to GPIO pin directory
     * @return File contents or error code
     */
    GpioResult<std::string> read_file(std::string_view filename) const noexcept;
};

/**
 * @brief Helper function to create GPIO pin with type deduction
 * @param pin Pin number
 * @param direction Initial direction
 * @return Unique pointer to GPIO pin controller
 */
template<GpioPinType T>
std::unique_ptr<GpioPin> make_gpio_pin(T pin, Direction direction = Direction::INPUT) {
    if constexpr (std::same_as<T, PinNumber>) {
        return std::make_unique<GpioPin>(pin, direction);
    } else {
        return std::make_unique<GpioPin>(static_cast<PinNumber>(pin), direction);
    }
}

/**
 * @brief GPIO pin group for managing multiple pins together
 */
class GpioPinGroup {
public:
    /**
     * @brief Construct GPIO pin group
     * @param pins List of pin numbers to manage
     * @param direction Initial direction for all pins
     */
    template<std::ranges::range R>
    requires std::convertible_to<std::ranges::range_value_t<R>, PinNumber>
    GpioPinGroup(R&& pins, Direction direction = Direction::INPUT);
    
    /**
     * @brief Set direction for all pins in group
     * @param direction New direction
     * @return Success or first error encountered
     */
    GpioResult<void> set_all_direction(Direction direction) noexcept;
    
    /**
     * @brief Set value for all output pins in group
     * @param value Value to set
     * @return Success or first error encountered
     */
    GpioResult<void> set_all_value(Value value) noexcept;
    
    /**
     * @brief Get values for all pins in group
     * @return Vector of pin values or error code
     */
    GpioResult<std::vector<Value>> get_all_values() const noexcept;
    
    /**
     * @brief Get pin by index
     * @param index Pin index in group
     * @return Reference to GPIO pin or nullptr if invalid index
     */
    GpioPin* get_pin(std::size_t index) noexcept;
    
    /**
     * @brief Get number of pins in group
     * @return Number of pins
     */
    std::size_t size() const noexcept { return pins_.size(); }
    
private:
    std::vector<std::unique_ptr<GpioPin>> pins_;
};

} // namespace embedded::gpio