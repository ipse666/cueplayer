#include "cueparser.h"

CueParser::CueParser(QString s)
{
	int in = 0;
	QRegExp rxPerformer("^PERFORMER \"(.*)\"");
	QRegExp rxAlbum("^TITLE \"(.*)\"");
	QRegExp rxSoundfile("^FILE \"(.*)\"");
	QRegExp rxTitle("    TITLE \"(.*)\"");
	QRegExp rxIndex("    INDEX \\d\\d (.*)");
	QRegExp rxTrackNumber("  TRACK (\\d\\d) .*");
	QRegExp rxAudioPath("(.*/).*");
	QRegExp rxFileWav("(.*)(\\.wav)");

	rxAudioPath.indexIn(s);
	audioPath = rxAudioPath.cap(1);
	
	QFile cuefile(s);
	if (!cuefile.open(QIODevice::ReadOnly | QIODevice::Text))
                return;
	QTextStream cuetext(&cuefile);
	cuetext.setCodec("Windows-1251");

	do {
		line = cuetext.readLine();
		if (rxPerformer.indexIn(line) != -1)
			parsedFile.performer = rxPerformer.cap(1);
		else if (rxAlbum.indexIn(line) != -1)
			parsedFile.album = rxAlbum.cap(1);
		else if (rxSoundfile.indexIn(line) != -1)
			parsedFile.soundfile = rxSoundfile.cap(1);
		else if (rxTitle.indexIn(line) != -1)
			tracks[++in].title = rxTitle.cap(1);
		else if (rxIndex.indexIn(line) != -1)
		{
			tracks[in].index = rxIndex.cap(1);
			tracks[in].file = parsedFile.soundfile;
		}
		else if (rxTrackNumber.indexIn(line) != -1)
			trackNumber = rxTrackNumber.cap(1).toInt(0,10);
	} while (!line.isNull());
	cuefile.close();

	if (rxFileWav.indexIn(parsedFile.soundfile) != -1)
	{
		QString soundfile = audioPath + "/" + rxFileWav.cap(1);
		QFile sndfh;
		if (!sndfh.exists(soundfile + rxFileWav.cap(2)))
		{
			if (!sndfh.exists(soundfile + ".ape"))
				if (!sndfh.exists(soundfile + ".flac"))
					;//QMessageBox::critical(this, trUtf8("Ошибка"), trUtf8("Медиафайл не найден."));
				else
					parsedFile.soundfile = rxFileWav.cap(1) + ".flac";
			else
				parsedFile.soundfile = rxFileWav.cap(1) + ".ape";
		}
		sndfh.close();
	}
	for (int i = in; i > 0; i--)
	{
		if (rxFileWav.indexIn(tracks[i].file) != -1)
		{
			QString soundfile = audioPath + "/" + rxFileWav.cap(1);
			QFile sndfh;
			if (!sndfh.exists(soundfile + rxFileWav.cap(2)))
			{
				if (!sndfh.exists(soundfile + ".ape"))
					if (!sndfh.exists(soundfile + ".flac"))
						;//QMessageBox::critical(this, trUtf8("Ошибка"), trUtf8("Медиафайл не найден."));
					else
						tracks[i].file = rxFileWav.cap(1) + ".flac";
				else
					tracks[i].file = rxFileWav.cap(1) + ".ape";
			}
			sndfh.close();
		}
	}
}

QString CueParser::getTitle()
{
	return parsedFile.performer + " - " + parsedFile.album;
}

QString CueParser::getPerformer()
{
	return parsedFile.performer;
}

QString CueParser::getAlbum()
{
	return parsedFile.album;
}

QString CueParser::getTrackTitle(int ind)
{
	return tracks[ind].title;
}

qint64 CueParser::getTrackIndex(int ind)
{
	QRegExp rxIndexMil("(\\d\\d):(\\d\\d):(\\d\\d)");
	rxIndexMil.indexIn(tracks[ind].index);
	indexMil = rxIndexMil.cap(1).toInt(0,10) * 60000 +
	rxIndexMil.cap(2).toInt(0,10) * 1000 +
	rxIndexMil.cap(3).toInt(0,10) * 10;
	return indexMil;
}

QString CueParser::getSoundFile()
{
	return audioPath + parsedFile.soundfile;
}

int CueParser::getTrackNumber()
{
	return trackNumber;
}

QString CueParser::getTrackFile(int ind)
{
	return audioPath + tracks[ind].file;
}
