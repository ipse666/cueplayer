#ifndef __CUEPARSER_H__
#define __CUEPARSER_H__
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QRegExp>

class CueParser
{
public:
	CueParser(QString);
	QString getTitle();
	QString getTrackTitle(int);
	qint64 getTrackIndex(int);
	QString getSoundFile();
	int getTrackNumber();
private:
	struct ParsedFile {
		QString performer;
		QString album;
		QString soundfile;
	};
	struct Tracks {
		QString title;
		QString index;
	};
	Tracks tracks[100];
	ParsedFile parsedFile;
	QString line;
	QString audioPath;
	int trackNumber;
	qint64 indexMil;
};

#endif // __CUEPARSER_H__
