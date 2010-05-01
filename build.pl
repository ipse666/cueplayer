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

# Поиск gst плагинов
&gst_plugins_check('gstreamer-0.10');
&gst_plugins_check('gstreamer-base-0.10');
&gst_plugins_check('gstreamer-interfaces-0.10');
&gst_plugins_check('gstreamer-plugins-base-0.10');

# Проверка элементов
&gst_elements_check('autoaudiosink');

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
	$dataver = ' DATAVER';
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
	  $argu = "\"DEFINES += BUILDER$dataver $argu\"";
  }
  else
  {
	  $argu = "\"DEFINES += BUILDER$dataver @ARGV\"";
  }
}
else
{
  $argu = "\"DEFINES += BUILDER$dataver\"";
}

if (`qmake-qt4 -v`)
{
  $qmake = qmake-qt4;
}
elsif (`qmake -v` =~ /qt4/)
{
  $qmake = qmake;
}
else
{
  warn "qmake не найден\n";
  exit 1;
}
print "qmake $argu\n";
system $qmake, $argu;
exec(make);

# Подпрограмма поиска gst плагинов
sub gst_plugins_check {
	$modul = shift;
	$result = `pkg-config --modversion --silence-errors $modul`;
	if ($result =~ /0\.10\.\d+/)
	{
		print "Обнаружен $modul\tверсии $result";
	}
	else
	{
		warn "$modul не обнаружен!\n";
		exit 1;
	}
}

# Подпрограмма проверки gst элементов
sub gst_elements_check {
	$elem = shift;
	@out = `gst-inspect-0.10 $elem`;
	if ($#out)
	{
		print "Обнаружен элемент $elem\n";
	}
	else
	{
		warn "
			Элемент $elem не обнаружен!
			установите gst-plugins-good
			http://www.gstreamer.net/src/gst-plugins-good/\n";
		exit 1;
	}
}