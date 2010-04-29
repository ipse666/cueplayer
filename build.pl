#!/usr/bin/perl

use IO::File;

$fh = new IO::File;
if ($fh->open("< ./.svn/entries")) {
	while (<$fh>)
	{
		if($_ =~ /(\d{4})-(\d{2})-(\d{2}).*/)
		{
			if ($1 >= $savegod)
			{
				($saveden, $savemes) = 0 if ($1 > $savegod);
				$savegod = $1;
				if ($2 >= $savemes)
				{
					$saveden = 0 if ($2 > $savemes);
					$savemes = $2;
					if ($3 >= $saveden)
					{
						$saveden = $3;
						$readnextline = 1;
					}
					else
					{
						$readnextline = 0;
					}
				}
				else
				{
					$readnextline = 0;
				}
			}
			else
			{
				$readnextline = 0;
			}
		}
		elsif ($readnextline && $_ =~ /^\d+$/)
		{
			$readnextline = 0;
			$rev = $_ if $rev < $_;
		}
	}
	print $saveden .'-'. $savemes .'-'. $savegod . "\n";
	print 'r'.$rev;
   $fh->close;
}

# Создание define файла;
if ($fh->open("> defines.h")) {
  if ($rev)
  {
	chomp($rev);
	print $fh "#ifndef DEFINES_H\n";
	print $fh "#define DEFINES_H\n";
	print $fh "#define REVISION \"$rev\"\n";
	print $fh "#define DATA \"$saveden-$savemes-$savegod\"\n";
	print $fh "#endif\n";
	$dataver = 'DATAVER';
  }
  else
  {
	print $fh "#ifndef DEFINES_H\n";
	print $fh "#define DEFINES_H\n";
	print $fh "#define REVISION \"0\"\n";
	print $fh "#define DATA \"0\"\n";
	print $fh "#endif\n";
  }
  $fh->close;
}

chomp($rev);
if ($#ARGV != -1)
{
  if ($ARGV[0] =~ /DEFINES/)
  {
	  $argu = "@ARGV";
	  $argu =~ s/DEFINES *\+*= *//;
	  $argu = "\"DEFINES += BUILDER $argu $dataver\"";
  }
  else
  {
	  $argu = "\"DEFINES += BUILDER $dataver @ARGV\"";
  }
}
else
{
  $argu = "\"DEFINES += BUILDER\"";
}
print "$argu\n";
system qmake, $argu;
exec(make);
