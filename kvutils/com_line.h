#ifndef _COM_LINE_H
#define _COM_LINE_H

#include <string>
#include <vector>

class COMMAND_LINE {

private:
    
    vector<string> cparams;

    vector<string>::size_type current;

public:

    COMMAND_LINE(int argc, char *argv[]);
    COMMAND_LINE(const vector<string>& params);
    
    string operator[](string::size_type n) { return(cparams[n]); }
    string::size_type size() const { return(cparams.size()); }

    void push_back(const string& argu);

    string next_non_argument(void);
    string next_argument(void);
    string next(void);
    string previous(void);
    bool has(char option);
    bool has(const string& option);
    
    void back_to_start(void) { current = 0; }
    bool ready(void) { if (current >= cparams.size()) return(false); else return (true); }
};

#endif
