#ifndef YOUTUBEDL_H
#define YOUTUBEDL_H

#include <QFile>
#include <QHttp>
#include <QUrl>
#include <QFileInfo>
#include <QDebug>
#include <QDir>

class VideoInfos
{
public:
	QString author;
	QString token;
	QString thumbnailUrl;
	QString title;
};

class YouTubeDL : public QObject
{
Q_OBJECT
public:
	YouTubeDL(QString);
	void abortDownload();

	QHttp *http;
	int httpGetId;
	bool httpRequestAborted;
	QString reasonPhrase;

private:
	void downloader(QString img_url, QString argument, QString dest);
	void startdownload();
	void downResponse(bool, QString);
	QString extractVideoId();
	VideoInfos extract_get_video_info_Infos();
	void extractVideoUrl();

	QFile *file;
	QString currentFileName;
	QStringList listLnk;
	QString lineUrl;

private slots:
	void httpRequestFinished(int requestId, bool error);
	void readResponseHeader(const QHttpResponseHeader &responseHeader);

signals:
	void putUrl(QStringList);
};

#endif // YOUTUBEDL_H
