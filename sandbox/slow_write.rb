#!/usr/bin/env ruby

$stdin.each_char do |byte| 
	print byte; 
	sleep( ARGV[0].nil?? 0.001 : ARGV[0].to_f )
end

