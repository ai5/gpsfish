$:.unshift File.dirname(__FILE__)

$osl_test_long = (!ENV['OSL_TEST_LONG'].nil?)


require 'tc_osl'
require 'tc_benchmark' if $osl_test_long

