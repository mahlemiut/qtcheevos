// RA API handling

#pragma once

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QString>


class TrackerData : public QObject
{
	Q_OBJECT

public:
	TrackerData();
	~TrackerData();

	void setUser(const QString &u) { user = u; }
	void setApikey(const QString &api) { apikey = api; }
	QString getUser() { return user; }
	QString getApikey() { return apikey; }
	void sendSummary();
	void sendBoxArt(QString url);
	void sendRank();
	QByteArray getResponse() { return response; }
	bool isBusy() { if (reply) return reply->isFinished(); else return false; }

signals:
	void dataReady(const QByteArray &data);

protected slots:
	void transferDone(QNetworkReply *reply);
	void transferError(QNetworkReply::NetworkError error);
	void transferSSLError(QNetworkReply* reply, QList<QSslError> errors);

private:
	QNetworkAccessManager *manager;
	QString user;
	QString apikey;
	QString url;
	QByteArray response;
	QNetworkReply *reply;
	bool busy;
};
