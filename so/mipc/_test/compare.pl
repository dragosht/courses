#!/usr/bin/perl

#Script Perl care compara ce citeste din fisierul dat ca parametru cu ce citeste 
#de la STDIN

open(IN, "<$ARGV[0]");
$line_no = 0;
while (<IN>)
{
	$line_no ++;
	$test_line = $_;
	chop $test_line;
	$line = <STDIN>;
	chop $line;
	$line =~ s/[ \t]+/ /g;
	$line =~ s/ $//;
	if ($test_line ne $line)
	{
		print "Eroare output lina $line_no\n";
		exit -1;
	}
}

if (<STDIN>)
{
	print "Eroare output: output-ul contine linii in plus\n";
	exit -1;
}

