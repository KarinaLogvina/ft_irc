#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

#define GREEN       "\033[32m"
#define RESET "\033[0m"
#define CYAN "\033[36m"
#define MAGENTA     "\033[35m"

std::string toLower(const std::string &str);
bool containsOnlyASCII(const std::string &str);
std::string trim(const std::string &str);

#endif