#ifndef UWS_LOG_H
#define UWS_LOG_H
#include <functional>
#include <iostream>
#include <string>

namespace uWS {

/*!
 * \brief Log a message. The user of this lib is free to override this function object with a custom logger.
 * \param [in] message The message to be logged.
 * \param [in] logLevel The severity of the message. 0 is error, 1 is warning, 2 is info. With each increment the severity decrements.
 */
typedef std::function<void(const std::string& message, int logLevel)> LogFunction;

inline LogFunction log = [](const std::string& message, int logLevel) -> void {
    if(logLevel <= 1) {
        std::cerr << message << std::endl;
    }
    else {
        std::cout << message << std::endl;
    }
};

}  // namespace uWS

#endif  // UWS_LOG_H

