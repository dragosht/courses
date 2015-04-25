#!/usr/bin/perl

#Script Perl care testeaza daca argumentul primit este mai mic decat 
#valoarea de pe prima citita de la STDIN
#Scriptul este folosit la testul 2

$line = <STDIN>;
chop $line;
@elem = split(/ /, $line);
$r = 0 + $elem[1];
if ($ARGV[0] > $r)
{
	exit -1;
}

