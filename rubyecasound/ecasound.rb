=begin
= ruby-ecasound

Example:

require "ecasound"
eci = Ecasound::ControlInterface.new(ecasound_args)
ecasound-response = eci.command("iam-command-here")
...

TODO:
Is there a chance that the ecasound process gets zombified?

= copyright
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

require "timeout"

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

class VersionString < String
    attr_reader :numbers

    def initialize(str)
        if str.split(".").length() != 3
            raise("Versioning scheme must be major.minor.micro")
        end
        super(str)
        @numbers = []
        str.split(".").each {|s| @numbers.push(s.to_i())}
    end
    
    def <=>(other)
        numbers.each_index do |i|
            if numbers[i] < other.numbers[i]
                return -1
            elsif numbers[i] > other.numbers[i]
                return 1
            elsif i < 2
                next
            end
        end
        return 0
    end
end # VersionString

module Ecasound

REQUIRED_VERSION = VersionString.new("2.2.0")
TIMEOUT = 5 # seconds before sync is called 'lost'

class EcasoundError < RuntimeError; end

class ControlInterface
    @@ecasound = ENV['ECASOUND'] || File::which("ecasound")
    
    if not File::executable?(@@ecasound)
        raise("ecasound executable not found")
    else
        @@version = VersionString.new(`#{@@ecasound} --version`.split("\n")[0][/\d\.\d\.\d/])
        if @@version < REQUIRED_VERSION
            raise("ecasound version #{REQUIRED_VERSION} or newer required, found: #{@@version}")
        end
    end
    
    def initialize(args = "")
        @ecapipe = IO.popen("-", "r+") # fork!
        
        if @ecapipe.nil?
            # child
            # stderr has to be redirected to avoid buffering problems 
            $stderr.reopen(open("/dev/null", "w"))
            exec("#{@@ecasound} -c -D -d:256" + args)
        else
            # parent
            command("int-output-mode-wellformed")
        end
    end

    def command(cmd)
        cmd.strip!()
        @ecapipe.write(cmd + "\n")

        response = ""
        
        # TimeoutError is raised unless response is complete
        begin
            timeout(TIMEOUT) do
                loop do
                    response += read()
                    if response =~ /256 ([0-9]{1,5}) (\-|i|li|f|s|S|e)\r\n(.*)\r\n\r\n/m
                        # we have a valid response
                        break
                    end
                end
            end
        rescue TimeoutError
            raise(EcasoundError, "lost synchronisation to ecasound subprocess")
        end
        
        content = $~[3][0, $~[1].to_i()]

        #print "command: #{cmd}\n"
        #print "type: '#{$~[2]}'\n"
        #print "length: #{$~[1]}\n"
        #print "content: #{content}\n"

        case $~[2]
            when "e"
                raise(EcasoundError, "command '#{cmd}' failed: #{content}")
            when "-"
                return nil
            when "s"
                return content
            when "S"
                return content.split(",")
            when "f"
                return content.to_f()
            when "i", "li"
                return content.to_i()
            else
                raise(EcasoundError, "parsing of ecasound's output produced an unknown return type")
        end
    end

    private

    def read()
        buffer = ""
        while select([@ecapipe], nil, nil, 0)
            buffer = buffer + @ecapipe.read(1)
        end
        buffer
    end
end # ControlInterface

end # Ecasound::
