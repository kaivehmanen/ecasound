"""Native python ECI (ecasound control interface) implementation"""

authors="""Kai Vehmanen, Eric S. Tiedemann and Janne Halttunen."""

import re
from popen2 import Popen3
from select import select
import os
import signal
import string

_ecasound=None

type_override={}

class ECA_CONTROL_INTERFACE:
    
    def __init__(I, verbose=1):
	"""Instantiates new ECI session
	
	verbose: set this false to get rid of startup-messages
	"""
	I.verbose=verbose
	
	I._cmd=''
	I._type=''
	I._resp={}
	I.initialize()
	
	
    def __call__(I, cmd, f=None):
        if f != None:
	    val=I.command_float_arg(cmd, f)
        else:
	    cmds=string.split(cmd, '\n')
	    if len(cmds) > 1:
		v=[]
		for c in cmds:
		    c=string.strip(c)
		    if c:
			v.append(I.command(c))
		    
			if I.error():
			    raise Exception(v[-1])
		    
		val=string.join(map(str, v), '\n')
	    else:
		val=I.command(cmd)
		    
	if I.error():
	    raise Exception(val)
	
	return val	    

    def _readline(I):
	return string.strip(_ecasound.fromchild.readline())
	
    def _read_eca(I):
	buffer=''
	while select([_ecasound.fromchild.fileno()],[],[],0)[0]:
	    buffer=buffer+_ecasound.fromchild.read(1)
	return buffer
    
    def _parse_response(I):
	tm=''; r=()
	while 1:
	    
	    s=I._read_eca()
##	    if s:
##		print s.strip()
	    tm=tm+s
	    
	    try:
		m=expand_eiam_response(tm)
		r=parse_eiam_response(tm, m)
		break
	    except Exception, e:
		pass

	if not r:
	    I._resp['e']='-'	    
	    I._type='e'
	    r=None
	else:
	    I._type=r[0]
	    
	    if I._cmd in type_override.keys():
		I._type=type_override[I._cmd]
	    
	    if I._type == 'S':
	    	I._resp[I._type]=string.split(r[1], ',')	    
	    elif I._type == 'Sn':
	    	I._resp[I._type]=string.split(r[1], '\n')
	    elif I._type == 'f':
	    	I._resp[I._type]=float(r[1])
	    elif I._type == 'i':
	    	I._resp[I._type]=int(r[1])
	    elif I._type == 'li':
	    	I._resp[I._type]=long(r[1])
	    else:
	    	I._resp[I._type]=r[1]
		
	return I._resp[I._type]

    
    def initialize(I):
	"""Reserve resources"""
	
	global _ecasound
	_ecasound=Popen3('ecasound -c', 1, 0)
	
	if I.verbose:
	    print I._readline()
	    print I._readline()
	    print I._readline()
	    print __doc__
	    print 'by', authors
	    print '\n(to get rid of this message, pass zero to instance init)'
	    
	I._read_eca()
	I.command('int-output-mode-wellformed')	
	
    def cleanup(I):
	"""Free all reserved resources"""
	
	_ecasound.tochild.write('quit\n')

	os.kill(_ecasound.pid, signal.SIGTERM)
		
	signal.signal(signal.SIGALRM, handler)
	signal.alarm(2)
	
	try:
	    return I.eca.wait()
	except:
	    pass
	
	signal.alarm(0)
	os.kill(_ecasound.pid, signal.SIGKILL)
	
	
    def command(I,cmd):
	"""Issue an EIAM command"""
	
	cmd=string.strip(cmd)
	if cmd:
	    I._cmd=cmd
	    _ecasound.tochild.write(cmd+'\n')
	    return I._parse_response()
	
    def command_float_arg(I,cmd,f=None):
	"""Issue an EIAM command
	
	This function can be used instead of command(string), 
	if the command in question requires exactly one numerical parameter."""
	
	cmd=string.strip(cmd)
	if cmd:
	    I._cmd=cmd
	    if f:
	    	_ecasound.tochild.write('%s %f\n' % (cmd,f))
	    else:
	    	_ecasound.tochild.write(cmd+'\n')
	    return I._parse_response()
	    
    def error(I):
	"""Returns true if error has occured during the execution of last EIAM command"""
	
	if I._type=='e': return 1
	
    def last_error(I):
	"""Returns a string describing the last error"""
	
	if I.error():
	    return I._resp.get('e')
	else: 
	    return ''
	
    def last_float(I):
	"""Returns the last floating-point return value"""
	return I._resp.get('f')
    
    def last_integer(I):
	"""Returns the last integer return value
	
	This function is also used to return boolean values."""
	return I._resp.get('i')
    
    def last_long_integer(I):
	"""Returns the last long integer return value
	
	Long integers are used to pass values like 'length_in_samples' 
	and 'length_in_bytes'.  It's implementation specific whether there's 
	any real difference between integers and long integers."""
	return I._resp.get('li')
    
    def last_string(I):
	"""Returns the last string return value"""
	return I._resp.get('s')
    
    def last_string_list(I):
	"""Returns the last collection of strings (one or more strings)"""
	return I._resp.get('S')
    
    def last_type(I):
	return I._type
    
    def current_event(I):
	"""** not implemented **"""
	pass
    def events_available(I): 
	"""** not implemented **"""
	pass
    def next_event(I): 
	"""** not implemented **"""
	pass


def handler(*args):
    print 'AARGH!'
    raise Exception, 'killing me not so softly'

    
expand=re.compile('256 ([0-9]{1,5}) (.+)\r\n(.*)\r\n\r\n.*', re.MULTILINE | re.S)

def expand_eiam_response(st):
    """Checks wheter 'str' is a valid EIAM response.

    @return Regex match object.
    """

    m = expand.search(st)
    return m

parse=re.compile('256 ([0-9]{1,5}) (.+)\r\n(.*)', re.MULTILINE | re.S)

def parse_eiam_response(st, m=None):
    """Parses a valid EIAM response.

    @param m Valid regex match object.
    @param str The whole EIAM response.

    @return tuple of return value type and value
    """

    if not m:
        m = parse.search(st)
	if not m:
            raise Exception, 'Regexp failed!'

    if m and len(m.groups()) == 0:
        print "(pyeca) Matching groups failed: %s" % str(m.groups())
	
    
    if m and len(m.groups()) == 3:
        #print 'received=', len(m.group(3)), ', expected=', m.group(1)
        if int(m.group(1)) != len(m.group(3)):
	    print "(pyeca) Response length error."
	    
    if m:
        return (m.group(2), m.group(3))

    return ('e','')


class base:
    def __init__(I, eci, cmd):
	I.eci=eci
	I.cmd=string.replace(cmd, '_', '-')
	
    def __call__(I):
	return I.eci(I.cmd)

class string_argument(base):
    def __call__(I, s):
	return I.eci('%s %s' % (I.cmd,s))
	

class eci:
    def __init__(I):
	I._eci=ECA_CONTROL_INTERFACE(0)
	I._cmds=I._eci('int-cmd-list')
	
	for c in I._cmds:
	    c=string.replace(c, '-', '_')
	    if string.count(c, 'add'):
		I.__dict__[c]=string_argument(I._eci,c)
	    else:
		I.__dict__[c]=base(I._eci,c)

def main():
    e=ECA_CONTROL_INTERFACE()
    print e.command('c-add huppaa')
    print e.command('c-list')
    
    print e("""
    
    c-list
    c-status
    """)

    print e.cleanup()

if __name__ == '__main__':
    main()
