#ifndef _ECA_ERROR_H
#define _ECA_ERROR_H

#include <string>

class ECA_ERROR {

public:
  
  // ------------------------------
  // error type / suggested action:
  // ------------------------------
  // stop = a critical error (can't allocate memory, etc), program 
  //        should exit at once
  // retry = action failed (badly formatted data, invalid user input,
  //         etc), no need to stop the whole program
  // notice = action succeeded but something unusual occured

  enum ACTION { stop, retry, notice };
 
private:
  string esection;
  string eerrormsg;
  
  ACTION eaction;

public:

  virtual const string& error_section(void) { return(esection); }
  virtual const string& error_msg(void) { return(eerrormsg); }
  virtual const ACTION& error_action(void) { return(eaction); }
  
  ECA_ERROR(const string& section, const string& errormsg, const
	    ACTION action = ECA_ERROR::retry) {
    esection = section;
    eerrormsg = errormsg;
    eaction = action;
  }
  
  virtual ~ECA_ERROR(void) { }
};

#endif
