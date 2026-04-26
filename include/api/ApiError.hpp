#ifndef API_API_ERROR_HPP
#define API_API_ERROR_HPP

#include <stdexcept>
#include <string>

class ApiError : public std::runtime_error {
public:
    ApiError(int statusCode, std::string message)
        : std::runtime_error(std::move(message)), statusCode_(statusCode) {}

    int statusCode() const noexcept {
        return statusCode_;
    }

private:
    int statusCode_;
};

#endif
