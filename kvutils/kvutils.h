#ifndef _KVUTILS_H
#define _KVUTILS_H

#include <vector>
#include <string>

#include "message_item.h"
#include "com_line.h"
#include "command_queue.h"
#include "value_queue.h"
// #include <ring_buffer.h>
#include "kvu_numtostr.h"

/**
 * Case-insensitive string compare
 *
 * Ignores preceding and trailing white space
 */
bool string_icmp(const string& a, const string& b);

/**
 * Converts a string to a vector of strings (words).
 * Whitespace is used as the separator.
 */
vector<string> string_to_words(const string& s);

/**
 * Converts a string to a vector of strings.
 *
 * @param str string to be converted
 * @param separator character to be used for separating items
 */
vector<string> string_to_vector(const string& str, const string::value_type separator);

/**
 * Return a new string, where all 'from' characters are
 * replaced with 'to' characters.
 */
string string_search_and_replace(const string& a, 
				 const string::value_type from,
				 const string::value_type to);

/**
 * Converts a vector of strings to a single string.
 *
 * @param str vector of strings to be converted
 * @param separator string that is inserted between items
 */
string vector_to_string(const vector<string>& str, 
			const string& separator);

/**
 * Removes all trailing white space
 */
string remove_trailing_spaces(const string& a);

/**
 * Removes all preciding white space
 */
string remove_preceding_spaces(const string& a);

/**
 * Removes all surrounding white spaces
 */
string remove_surrounding_spaces(const string& a);

/**
 * Converts string to uppercase using toupper(int)
 */
string convert_to_uppercase(const string& a);

/**
 * Converts string to lowercase using tolower(int)
 */
string convert_to_lowercase(const string& a);

/**
 * Converts string to uppercase using toupper(int)
 * Modifies the parameter object.
 */
void to_uppercase(string& a);

/**
 * Converts string to lowercase using tolower(int)
 * Modifies the parameter object.
 */
void to_lowercase(string& a);

/**
 * Returns the nth argument from a formatted string
 *
 * @param number the argument number 
 * @param argu a formatted string: "something:arg1,arg2,...,argn"
 *
 * require:
 *  number >= 1
 */
string get_argument_number(int number, const string& argu);

/**
 * Returns a vector of all arguments from a formatted string
 *
 * @param argu a formatted string: "something:arg1,arg2,...,argn"
 */
vector<string> get_arguments(const string& argu);

/** 
 * Returns number of arguments in formatted string 'argu'.
 */
int get_number_of_arguments(const string& argu);

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
string get_argument_prefix(const string& argument);

#endif

