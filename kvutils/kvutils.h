#ifndef INCLUDED_KVUTILS_H
#define INCLUDED_KVUTILS_H

#include <vector>
#include <string>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef USE_CXX_STD_NAMESPACE
using namespace std;
#endif

/**
 * Case-insensitive string compare. Ignores preceding and 
 * trailing white space.
 */
bool string_icmp(const std::string& a, const std::string& b);

/**
 * Converts a string to a vector of strings (words).
 * Whitespace is used as the separator.
 */
std::vector<std::string> string_to_words(const std::string& s);

/**
 * Converts a string to a vector of strings.
 *
 * @param str string to be converted
 * @param separator character to be used for separating items
 */
std::vector<std::string> string_to_vector(const std::string& str, const std::string::value_type separator);

/**
 * Converts a string to a vector of integers.
 *
 * @param str string to be converted
 * @param separator character to be used for separating items
 */
std::vector<int> string_to_int_vector(const std::string& str, const std::string::value_type separator);

/**
 * Return a new string, where all 'from' characters are
 * replaced with 'to' characters.
 */
std::string string_search_and_replace(const std::string& a, 
				 const std::string::value_type from,
				 const std::string::value_type to);

/**
 * Converts a vector of strings to a single string.
 *
 * @param str vector of strings to be converted
 * @param separator string that is inserted between items
 */
std::string vector_to_string(const std::vector<std::string>& str, 
			const std::string& separator);

/**
 * Removes all trailing white space
 */
std::string remove_trailing_spaces(const std::string& a);

/**
 * Removes all preciding white space
 */
std::string remove_preceding_spaces(const std::string& a);

/**
 * Removes all surrounding white spaces
 */
std::string remove_surrounding_spaces(const std::string& a);

/**
 * Converts string to uppercase using toupper(int)
 */
std::string convert_to_uppercase(const std::string& a);

/**
 * Converts string to lowercase using tolower(int)
 */
std::string convert_to_lowercase(const std::string& a);

/**
 * Converts string to uppercase using toupper(int)
 * Modifies the parameter object.
 */
void to_uppercase(std::string& a);

/**
 * Converts string to lowercase using tolower(int)
 * Modifies the parameter object.
 */
void to_lowercase(std::string& a);

/**
 * Returns the nth argument from a formatted string
 *
 * @param number the argument number 
 * @param argu a formatted string: "something:arg1,arg2,...,argn"
 *
 * require:
 *  number >= 1
 */
std::string get_argument_number(int number, const std::string& argu);

/**
 * Returns a vector of all arguments from a formatted string
 *
 * @param argu a formatted string: "something:arg1,arg2,...,argn"
 */
std::vector<std::string> get_arguments(const std::string& argu);

/** 
 * Returns number of arguments in formatted string 'argu'.
 */
int get_number_of_arguments(const std::string& argu);

/**
 * Get the prefix part of a string argument
 * @param argument format used is -prefix:arg1, arg2, ..., argN
 *
 * require:
 *   argu.find('-') != string::npos
 *
 * ensure:
 *   argu.size() >= 0
 */
std::string get_argument_prefix(const std::string& argument);

/**
 * Prints a time stamp to stderr
 */
void print_time_stamp(void);

#endif
