// a class to get all the Icons for a particular game supported by Retroachievements

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QImage>

#pragma once

class IconList : public QObject
{
	Q_OBJECT

public:
	IconList(QObject *parent = 0);
	~IconList();

	struct Icon
	{
		int id;
		QString url;
		QImage image;
		bool unlocked;
	};

	enum Loading
	{
		Idle = 0,
		List = 1,
		Icons = 2
	};

	void getIcons(int gameID, QString username, QString apiKey);
	void clear();
	QList<Icon>* getIconList() { return &icons; }
	int getPhase() { return loadphase; }

signals:
	void iconLoad();

public slots:
	void ListReceived(QNetworkReply *reply);
	void IconsReceived(QNetworkReply *reply);
	void PopulateIcons();

private:
	QNetworkAccessManager *manager;
	QList<Icon> icons;
	unsigned short game;
	bool loadicons;
	int loadphase;
	int pos;  // position in the icon list while downloading their images
};