#!/usr/bin/perl

#Script Perl folosit pentru a verifica daca output-ul unui program client este corect (corespunde
#unui arbore binar de cautare ce contine numai elemente valide)

sub parcurgeInordine
{
	my ($nod, $vectref) = @_;
	unless ($nod->{nul})
	{
		parcurgeInordine($nod->{stanga}, $vectref);
		push(@$vectref, $nod->{valoare});
		parcurgeInordine($nod->{dreapta}, $vectref);
	}
}

$root = {};
$root->{nul} = 1;
@queue = ($root);
$lno = 0;
do
{
	$element_added = 0;
	$lno ++;
	$line = <STDIN>;
	chop $line;
	if ($line =~ /[^- \t+0-9*]/)
	{
		print "Eroare output linia $lno: Output-ul contine caractere invalide\n";
		exit -1;
	}
	@elemente = split(/[ \t]+/, $line);
	@next_queue = ();
	for ($i = 0; $i <= $#elemente; $i ++)
	{
		if ($#queue < 0)
		{
			print "Eroare output linia $lno: Output-ul contine elemente in plus\n";
			exit -1;
		}
		if ($elemente[$i] eq '*')
		{
			 pop(@queue);
		}
		elsif ($elemente[$i] =~ /^[+-]?\d+$/)
		{
			$element_added = 1;
			$val = $elemente[$i] + 0;
			$nod = pop(@queue);
			$nod->{valoare} = $val;
			$nod->{nul} = 0;
			$nod->{stanga} = {};
			($nod->{stanga})->{nul} = 1;
			$nod->{dreapta} = {};
			($nod->{dreapta})->{nul} = 1;
			unshift(@next_queue, $nod->{stanga});
			unshift(@next_queue, $nod->{dreapta});
		}
		else
		{
			print "Eroare output linia $lno: Output-ul contine elemente invalide\n";
			exit -1;
		}
	}
	if ($#queue >= 0)
	{
		print "Eroare output linia $lno: Output-ul este incomplet\n";
		exit -1;
	}
	@queue = @next_queue;
} while ($element_added > 0);
@vector = ();
parcurgeInordine($root, \@vector);
#print "@vector\n";
for ($i = 1; $i <= $#vector; $i ++)
{
	if ($vector[$i - 1] > $vector[$i])
	{
		print "Eroare output: Arborele nu este arbore binar de cautare\n";
		exit -1;
	}
}
if (<STDIN>)
{
	print "Eroare output: output-ul contine linii in plus\n";
	exit -1;
}

open(IN, "<$ARGV[0]");
$line = <IN>;
chop $line;
@v_min = split(/[ \t]+/, $line);
$line = <IN>;
chop $line;
@v_max = split(/[ \t]+/, $line);
$j = 0;
$k = 0;
for ($i = 0; $i <= $#vector; $i ++)
{
	if ($j <= $#v_min)
	{
		if ($v_min[$j] < $vector[$i])
		{
			print "Eroare output: arborele nu contine toate valorile introduse\n";
			exit -1;
		}
		elsif ($v_min[$j] == $vector[$i])
		{
			$j ++;
		}
	}
	if ($k > $#v_max)
	{
		print "Eroare output: arborele contine valori care nu au fost adaugate\n";
		exit -1;
	}
	else
	{
		while ($v_max[$k] < $vector[$i] && $k <= $#v_max)
		{
			$k ++;
		}
		if ($k > $#v_max)
		{
			print "Eroare output: arborele contine valori care nu au fost adaugate\n";
			exit -1;
		}
		if ($v_max[$k] > $vector[$i])
		{
			print "Eroare output: arborele contine valori care nu au fost adaugate\n";
			exit -1;
		}
		else
		{
			$k ++;
		}
	}
}
if ($j <= $#v_min)
{
	print "Eroare output: arborele nu contine toate valorile introduse\n";
	exit -1;
}
