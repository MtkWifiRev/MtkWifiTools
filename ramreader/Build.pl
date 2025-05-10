#!/usr/bin/perl

### Made by Edoardo Mantovani, 2025

use strict;
use warnings;
use Term::ANSIColor qw(color);

my $PPTitle	= "
▄▄▄   ▄▄▄· • ▌ ▄ ·. ▄▄▄  ▄▄▄ . ▄▄▄· ·▄▄▄▄  ▄▄▄ .▄▄▄  
▀▄ █·▐█ ▀█ ·██ ▐███▪▀▄ █·▀▄.▀·▐█ ▀█ ██▪ ██ ▀▄.▀·▀▄ █·
▐▀▀▄ ▄█▀▀█ ▐█ ▌▐▌▐█·▐▀▀▄ ▐▀▀▪▄▄█▀▀█ ▐█· ▐█▌▐▀▀▪▄▐▀▀▄ 
▐█•█▌▐█ ▪▐▌██ ██▌▐█▌▐█•█▌▐█▄▄▌▐█ ▪▐▌██. ██ ▐█▄▄▌▐█•█▌
.▀  ▀ ▀  ▀ ▀▀  █▪▀▀▀.▀  ▀ ▀▀▀  ▀  ▀ ▀▀▀▀▀•  ▀▀▀ .▀  ▀
";

system("clear");
print(color("red"));
print($PPTitle);
undef($PPTitle);
print(color("reset"));

my $PPGCC			= "gcc";
my $PPSTRIP			= "strip";
my $PPLIBSFOLDER		= "./libs";

my $PP_finalcmdline 		= undef;
my $PP_outputfolder		= undef;
my $PP_finalexecname		= undef;
my $PP_finalfile		= undef;
my $PP_srcfolder		= undef;
my $PP_finalstaticlibs		= undef;
my $PP_libsfolder		= undef;
my $PP_sourcecode		= undef;
my @PP_staticlibs		= ("libopcodes.a", "libbfd.a", "libiberty.a", "libz.a");
my @PP_cmdopts			= ("-Wno-format", "-Wno-incompatible-pointer-types", "-Wno-format-truncation", "-Wno-unused-result", " ");
my @PP_optimizations		= ("--static -O3 ");
my @PP_includepath		= ("./include ");
$PP_finalexecname		= "ramreader";
$PP_outputfolder		= "./build/";
$PP_srcfolder			= "./src/";
$PP_libsfolder			= "./libs/";
$PP_finalfile			= $PP_outputfolder . $PP_finalexecname;
$PP_sourcecode			= $PP_srcfolder    . $PP_finalexecname . ".c"; 

foreach( @PP_staticlibs ){
	$PP_finalstaticlibs    .= $PP_libsfolder . $_ . " ";
}

system("$PPGCC @PP_cmdopts @PP_optimizations  -I@PP_includepath -o  $PP_finalfile $PP_sourcecode $PP_finalstaticlibs");
system("$PPSTRIP $PP_finalfile");
