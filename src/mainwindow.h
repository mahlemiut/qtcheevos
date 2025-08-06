#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWindow>
#include <QBackingStore>
#include <QPainter>
#include <QImage>
#include <QTimer>
#include <QSettings>
#include <iostream>
#include "network.h"
#include "iconlist.h"
#include "build/QtCheevos_autogen/include/build/ui_mainwindow.h"

namespace Ui {
    class MainWindow;

}

class TrackerFrame : public QFrame
{
    Q_OBJECT

public:
    explicit TrackerFrame(QWidget *parent = nullptr);
    ~TrackerFrame() override;

    void setFont(const QFont &f);
    void setBGColour(const QColor &c);
    QColor getBG() { return bgcol; }
    QFont getFont() { return font; }
    QString getGameName() { return gamename; }
    int getGameID() { return gameID; }
    void setShowBox(bool b) { showbox = b; update(); }
	void initData() { currentStep = Rank; }
	QString getBoxURL() { return boxurl; }
    void showGameName(bool b) { showgamename = b; update(); }
    void showProgress(bool b) { showprogress = b; update(); }
    void showCheevoIcons(bool b) { showcheevoicons = b; update(); }
    void showRank(bool b) { showrank = b; update(); }
    void getCheevos(const QString &user, const QString &apikey) { iconlist->getIcons(gameID, user, apikey); }
    int getCheevoLoadPhase() { return iconlist->getPhase(); }

enum
{
	Idle = 0,
	Summary = 1,
    Rank,
	BoxArt,
	Cheevos
};

public slots:
	void handleData(const QByteArray &n);
signals:
	void dataReceived(int step);

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	void handleSummary(QByteArray n);
	void handleBoxArt(QByteArray n);
	void handleIconList(QByteArray n);
    void handleRank(QByteArray n);
    bool showgamename;
	bool showbox;
    bool showprogress;
    bool showcheevoicons;
    bool showrank;
    int ShowRank(QPainter &painter, int *currentline);
    int ShowGameName(QPainter &painter, int *currentline);
    int ShowProgress(QPainter &painter, int *currentline);
    int ShowCheevoIcons(QPainter &painter, int *currentline);
    QFont font;
    QColor bgcol;
    int gameID;
    QString gamename;
    QString systemname;
    QString boxurl;
	int cheevoUnlocks;
	int cheevoTotal;
    int score;
    int maxscore;
    QImage boximage;
    int rank;
    int rankTotal;
    int overallScore;
    int overallRetroScore;
    int currentStep;
    IconList* iconlist;
};

class TrackerWindow : public QWidget
{
    Q_OBJECT

public:
    explicit TrackerWindow(TrackerData* d, QWidget *parent = nullptr);
    ~TrackerWindow() override;
    void redraw();
	void setShowBox(bool b) { frame->setShowBox(b); }
    void setFont(const QFont &f);
    void setBGColour(const QColor &c);
    QColor getBG() { return frame->getBG(); }
    QFont getFont() { return frame->getFont(); }
    void setTracker(TrackerData* d) { data = d; }
    void setUser(const QString &u) { data->setUser(u); }
    void setApikey(const QString &api) { data->setApikey(api); }
    void setShowGameName(bool b) { frame->showGameName(b); }
    void setShowProgress(bool b) { frame->showProgress(b); }
    void setShowIcons(bool b) { frame->showCheevoIcons(b); }
    void setShowRank(bool b) { frame->showRank(b); }

public slots:
    void dataReceived(int step);
    void refresh();

protected:
    bool event(QEvent *event) override;

private:
//    void render(QPainter *painter);
    TrackerFrame* frame;
    TrackerData* data;
    QString boxurl;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void tracker_clicked();
    void choosebgcol_clicked();
    void choosefont_clicked();
    void user_updated();
    void apikey_updated();
    void refresh_clicked();
	void bgcolour_selected();
	void bgimage_selected();
    void showgame_clicked();
    void showprogress_clicked();
    void showicons_clicked();
    void showrank_clicked();
    void refreshtimer_changed();
    
private:
    Ui::MainWindow* ui;
    QTimer* timer;

    // Add private members for UI components here
    bool tracker_visible;

    // tracker window stuff
    TrackerWindow* tracker;
    TrackerData* trackerData;
};

#endif // MAINWINDOW_H