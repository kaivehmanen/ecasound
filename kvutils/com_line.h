#ifndef _COM_LINE_H
#define _COM_LINE_H

#include <string>
#include <vector>

/**
 * Class representation of command line arguments
 */
class COMMAND_LINE {

private:
    
    vector<string> cparams;

    mutable vector<string>::size_type current_rep;

public:
   
    /**
     * Number of elements
     */
    string::size_type size() const { return(cparams.size()); }

    /**
     * Sets the first argument active. This is usually program's
     * name.
     */
    void begin(void) { current_rep = 0; }

    /**
     * Moves to the next argument.
     */
    void next(void) { ++current_rep; }

    /**
     * Moves to the previous argument.
     */
    void previous(void) { --current_rep; }

    /** 
     * Returns true if we've past the last argument.
     */
    bool end(void) const { if (current_rep >= cparams.size()) return(true); else return (false); }
    
    /** 
     * Returns the current argument
     *
     * require:
     *  end() == false
     */
    const string& current(void) const { return(cparams[current_rep]); }

    /**
     * Is '-option' is among the arguments?
     *
     * ensure:
     *  current() == old current()
     */
    bool has(char option) const;

    /**
     * Is '-option' is among the arguments?
     */
    bool has(const string& option) const;

    /**
     * Make sure that all option tokens start with a '-' sign
     */
    void combine(void);

    /**
     * Static version of <code>combine</code>
     */
    static vector<string> combine(const vector<string>& source);

    /**
     * Adds 'argu' to the arguments.
     */
    void push_back(const string& argu);

    COMMAND_LINE(int argc, char *argv[]);
    COMMAND_LINE(const vector<string>& params);
};

#endif
