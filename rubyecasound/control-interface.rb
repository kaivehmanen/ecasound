=begin
= ruby-ecasound
== ecasound control interface

== copyright
Copyright (C) 2003 Jan Weil <Jan.Weil@web.de>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
=end

require "ecasound"
require "timeout"

# File::which
# just a utility
class File
    
    def self::which(prog, path=ENV['PATH'])
        path.split(File::PATH_SEPARATOR).each do |dir|
            f = File::join(dir, prog)
            if File::executable?(f) && ! File::directory?(f)
                return f
            end
        end
    end

end # File

module Ecasound

class ControlInterface
    
    REQUIRED_VERSION = "2.2"
    
    @@ecasound = ENV['ECASOUND'] || File::which("ecasound")
    
    if not File::executable?(@@ecasound)
        raise(EcaError, "ecasound executable not found")
    else
        @@version = `#{@@ecasound} --version`.split("\n")[0][/\d\.\d\.\d/]
        # TODO: compare floats! 2.10 < 2.2 !
        if @@version < REQUIRED_VERSION
            raise(EcaError, "ecasound version #{REQUIRED_VERSION} or newer required, found: #{@@version}")
        end
    end
    
    @@timeout = 5   # seconds before sync is called 'lost'

    def initialize(args = "")
        @ecapipe = open("|-", "r+") # fork!
        
        if @ecapipe.nil?
            # child
            # stderr has to be redirected to avoid buffering problems 
            $stderr.reopen(open("/dev/null", "w"))
            exec("#{@@ecasound} -c -D " + args)
        else
            # parent
            @command = nil
            @response = {}
            @last_type = nil

            self.command("int-output-mode-wellformed")
        end
    end

    def command(cmd, f=nil)
        @command = cmd.strip()
        @ecapipe.write(@command + "\n")

        response = ""
        
        # TimeoutError is raised unless response is complete
        begin
            timeout(@@timeout) do
                loop do
                    response += read_eca()
                    if response =~ /256 ([0-9]{1,5}) (.+)\r\n(.*)\r\n\r\n/m
                        # we have a valid response
                        break
                    end
                end
            end
        rescue TimeoutError
            msg = "lost synchronisation to ecasound subprocess"
            raise(EcaError, msg)
        end

        @last_type = $~[2]
        case @last_type
            when "e"
                # an error occured!
                raise(EcaError, "command failed: '#{@command}'")
            when "S"
                @response["S"] = $~[3].split(",")
            when "f"
                @response["f"] = $~[3].to_f()
            when "i", "li"
                @response[@last_type] = $~[3].to_i()
            else
                @response[@last_type] = $~[3]
        end
        @response[@last_type]
    end

    private

    def read_eca()
        buffer = ""
        while select([@ecapipe], nil, nil, 0)
            buffer = buffer + @ecapipe.read(1)
        end
        buffer
    end

end # ControlInterface

end # Ecasound::
