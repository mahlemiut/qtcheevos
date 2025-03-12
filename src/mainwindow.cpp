#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QColorDialog>
#include <QFontDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <unistd.h>
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      tracker_visible(false),
      timer(new QTimer(this)),
      ui(new Ui::MainWindow),
      trackerData(new TrackerData)
{
    ui->setupUi(this);

    tracker = new TrackerWindow(trackerData, this->window());
    //tracker->setVisible(tracker_visible);
    tracker->setWindowTitle("Tracker");
    tracker->setWindowFlags(Qt::Window);
    connect(timer, &QTimer::timeout, tracker, &TrackerWindow::refresh);

    // read settings
    QSettings settings;
    ui->edit_username->setText(settings.value("username", "").toString());
    //ui->edit_key->setText(settings.value("apikey", "").toString());
    tracker->setUser(ui->edit_username->text());
    //tracker->setApikey(ui->edit_key->text());
    ui->show_gamename->setChecked(settings.value("show_gamename", true).toBool());
    ui->show_progress->setChecked(settings.value("show_progress", true).toBool());
    ui->show_icons->setChecked(settings.value("show_icons", true).toBool());
    tracker->setShowGameName(ui->show_gamename->isChecked());
    tracker->setShowProgress(ui->show_progress->isChecked());
    tracker->setShowIcons(ui->show_icons->isChecked());
    ui->choosebgcol->setPalette(QPalette(settings.value("bgcolour", QColor(255,0,255)).value<QColor>()));
    tracker->setBGColour(ui->choosebgcol->palette().button().color());
    ui->choosefont->setFont(settings.value("font", QFont("Arial", 12)).value<QFont>());
    tracker->setFont(ui->choosefont->font());
    ui->opt_bgcolour->setChecked(!settings.value("showbox", true).toBool());
    ui->opt_bgimage->setChecked(settings.value("showbox", true).toBool());
    tracker->setShowBox(settings.value("showbox", true).toBool());
	ui->show_rank->setChecked(settings.value("showrank", true).toBool());
	tracker->setShowRank(ui->show_rank->isChecked());
    ui->spin_refreshtime->setValue(settings.value("refreshtime", 5).toInt());
    timer->setInterval(ui->spin_refreshtime->value());
}

MainWindow::~MainWindow()
{
    // save settings
    QSettings settings;
    settings.setValue("username", ui->edit_username->text());
    //settings.setValue("apikey", ui->edit_key->text());
    settings.setValue("show_gamename", ui->show_gamename->isChecked());
    settings.setValue("show_progress", ui->show_progress->isChecked());
    settings.setValue("show_icons", ui->show_icons->isChecked());
    settings.setValue("bgcolour", ui->choosebgcol->palette().button().color());
    settings.setValue("font", ui->choosefont->font());
    settings.setValue("showbox", ui->opt_bgimage->isChecked());
	settings.setValue("showrank", ui->show_rank->isChecked());
    settings.setValue("refreshtime", ui->spin_refreshtime->value());
    delete ui;
}

void MainWindow::tracker_clicked()
{
    tracker_visible = !tracker_visible;
    //tracker->setVisible(tracker_visible);
    if (tracker_visible)
    {
        tracker->show();
        timer->start();
    }
    else
    {
        tracker->hide();
        timer->stop();
    }

    ui->toggle_tracker->setText(!tracker_visible ? "Show Tracker Window" : "Hide Tracker Window");
    
}

void MainWindow::choosebgcol_clicked()
{
    QColor color = QColorDialog::getColor(QColor(255,0,255), this);
    if (color.isValid()) 
    {
        QPalette palette = ui->choosebgcol->palette();
        tracker->setBGColour(color);
        palette.setColor(QPalette::Button, color);
        ui->choosebgcol->setPalette(palette);
    }
}

void MainWindow::choosefont_clicked()
{
    bool ok;
    QFont f = QFontDialog::getFont(&ok, this);
    if (ok) 
    {
        ui->choosefont->setFont(f);
        tracker->setFont(f);
    }
}

void MainWindow::user_updated()
{
    tracker->setUser(ui->edit_username->text());
}

void MainWindow::apikey_updated()
{
    tracker->setApikey(ui->edit_key->text());
}

void MainWindow::refresh_clicked()
{
    tracker->refresh();
}

void MainWindow::bgcolour_selected()
{
    tracker->setShowBox(false);
}

void MainWindow::bgimage_selected()
{
    tracker->setShowBox(true);
}

void MainWindow::showgame_clicked()
{
    tracker->setShowGameName(ui->show_gamename->isChecked());
}

void MainWindow::showprogress_clicked()
{
    tracker->setShowProgress(ui->show_progress->isChecked());
}

void MainWindow::showicons_clicked()
{
    tracker->setShowIcons(ui->show_icons->isChecked());
}

void MainWindow::showrank_clicked()
{
	tracker->setShowRank(ui->show_rank->isChecked());
}

void MainWindow::refreshtimer_changed()
{
    timer->setInterval(ui->spin_refreshtime->value() * 1000);
}

// Tracker Window implementation
TrackerWindow::TrackerWindow(TrackerData* d, QWidget *parent)
    : data(d),
      QWidget(parent),
      frame(new TrackerFrame(this))
{
    setGeometry(50, 50, 448, 800);
    setMaximumSize(QSize(448, 800));
    frame->setGeometry(0, 0, 448, 800);
    connect(frame, &TrackerFrame::dataReceived, this, &TrackerWindow::dataReceived);
    connect(data, &TrackerData::dataReady, frame, &TrackerFrame::handleData);
}

TrackerWindow::~TrackerWindow()
{
    delete frame;
}

void TrackerWindow::redraw()
{
    update();
    frame->update();
}

void TrackerWindow::refresh()
{
    if(frame->getCheevoLoadPhase() != IconList::Icons)  // Icons don't seem to always dowload if interrupted by a regular refresh
    {
        data->sendRank();
        frame->initData();
    }
    frame->update();
}

//void TrackerWindow::render(QPainter *painter)
//{
//}

bool TrackerWindow::event(QEvent *event)
{
//    if (event->type() == QEvent::UpdateRequest) 
//    {

//        frame->update();
//        return true;
    //}
    return QWidget::event(event);
}

void TrackerWindow::setFont(const QFont &f)
{
    frame->setFont(f);
    redraw();
}

void TrackerWindow::setBGColour(const QColor &c)
{
    frame->setBGColour(c);
    redraw();
}

void TrackerWindow::dataReceived(int step)
{
    switch(step)
    {
        case TrackerFrame::Idle:
            frame->update();
            break;
        case TrackerFrame::Rank:
            data->sendSummary();
            break;
        case TrackerFrame::Summary:
            data->sendBoxArt(frame->getBoxURL());
            break;
        case TrackerFrame::BoxArt:
            frame->getCheevos(data->getUser(), data->getApikey());
            break;
        case TrackerFrame::Cheevos:
            frame->update();
            break;
    }
}


// Tracker Frame implementation
TrackerFrame::TrackerFrame(QWidget *parent)
    : QFrame(parent),
	showgamename(true),
    showbox(false),
	showprogress(true),
    showcheevoicons(true),
    showrank(true),
    font(QFont("Arial", 12)),
    bgcol(QColor(255, 0, 255)),
    gameID(0),
    currentStep(Idle),
    iconlist(new IconList(this))
{
    setFrameStyle(QFrame::Panel);
    setLineWidth(0);
}

TrackerFrame::~TrackerFrame()
{
}

void TrackerFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRect bounds;
    QString text;
    int currentline = 0;
    if(boximage.isNull() || showbox == false || gamename.isEmpty())
    {
        painter.fillRect(0, 0, width(), height(), bgcol);
    }
    else
    {
        painter.drawImage(QRect(0, 0, width(), height()), boximage);
        painter.fillRect(0, 0, width(), height(), QColor(0,0,0,128));
    }

    if(gamename.isEmpty())
        return;

    painter.setPen(Qt::white);

    // draw total points / rank
    if(showrank)
    {
        font.setPointSize(16);
        painter.setFont(font);

        text = "Total Points: " + QString::number(overallScore) + " (Retro: " + QString::number(overallRetroScore) + ")";
        bounds = painter.boundingRect(QRect(0,currentline,width(),height()), Qt::AlignHCenter | Qt::TextWordWrap, text);
        painter.drawText(bounds, Qt::AlignHCenter | Qt::TextWordWrap, text);
        currentline = bounds.height()+5;

        text = "Rank: " + QString::number(rank) + "/" + QString::number(rankTotal);
        bounds = painter.boundingRect(QRect(0,currentline,width(),height()), Qt::AlignHCenter | Qt::TextWordWrap, text);
        painter.drawText(bounds, Qt::AlignHCenter | Qt::TextWordWrap, text);
        currentline += bounds.height()+5;
        painter.drawLine(0, currentline, width(), currentline);
        currentline += 5;
    }

    // draw game title
    if(showgamename)
    {
	    font.setPointSize(24);
    	painter.setFont(font);
    
		bounds = painter.boundingRect(QRect(0,currentline,width(),height()), Qt::AlignHCenter | Qt::TextWordWrap, gamename);
		painter.drawText(bounds, Qt::AlignHCenter | Qt::TextWordWrap, gamename);
		currentline += bounds.height()+5;
		painter.drawLine(0, currentline, width(), currentline);
		currentline += 5;
	}

    // draw achevement and score progress
	if(showprogress)
	{
		font.setPointSize(20);
		painter.setFont(font);

		text = "Achievements: " + QString::number(cheevoUnlocks) + "/" + QString::number(cheevoTotal);
		bounds = painter.boundingRect(QRect(0,currentline,width(),height()), Qt::AlignHCenter | Qt::TextWordWrap, text);
		painter.drawText(bounds, Qt::AlignHCenter | Qt::TextWordWrap, text);
		currentline += bounds.height()+5;

		text = "Score: " + QString::number(score) + "/" + QString::number(maxscore);
		bounds = painter.boundingRect(QRect(0,currentline,width(),height()), Qt::AlignHCenter | Qt::TextWordWrap, text);
		painter.drawText(bounds, Qt::AlignHCenter | Qt::TextWordWrap, text);
		currentline += bounds.height()+5;
		painter.drawLine(0, currentline, width(), currentline);
		currentline += 5;
	}

    // draw achievement icons
    if(showcheevoicons)
    {
        int iconsperline = width() / 64;

        if(iconlist->getIconList()->size() > 0)
        {
            int x = 0;
            int y = currentline;
            for (int i = 0; i < iconlist->getIconList()->size(); i++)
            {
                if (x >= iconsperline)
                {
                    x = 0;
                    y += 64;
                }
                painter.drawImage(x*64, y, iconlist->getIconList()->at(i).image);
                if(iconlist->getIconList()->at(i).unlocked == false)
                    painter.fillRect(x*64, y, 64, 64, QColor(0,0,0,196));
                x++;
            }
        }
    }

}

void TrackerFrame::setFont(const QFont &f)
{
    font = f;
}

void TrackerFrame::setBGColour(const QColor &c)
{
    bgcol = c;
}

void TrackerFrame::handleData(const QByteArray &n)
{
    switch(currentStep)
    {
		case Rank:
			handleRank(n);
			break;
        case Summary:
            handleSummary(n);
            break;
        case BoxArt:
            handleBoxArt(n);
            break;
        case Cheevos:
            handleIconList(n);
            break;
    }
    update();
}

void TrackerFrame::handleSummary(QByteArray n)
{
    QJsonDocument doc = QJsonDocument::fromJson(n);
    QJsonArray played = doc.array();
    int temp;
    gamename = played[0].toObject()["Title"].toString();
    boxurl = played[0].toObject()["ImageBoxArt"].toString();
    cheevoUnlocks = played[0].toObject()["NumAchieved"].toInt();
    cheevoTotal = played[0].toObject()["AchievementsTotal"].toInt();
    score = played[0].toObject()["ScoreAchieved"].toInt();
    maxscore = played[0].toObject()["PossibleScore"].toInt();
    temp = played[0].toObject()["GameID"].toInt();

    if(gameID == temp)
    {
        emit dataReceived(TrackerFrame::BoxArt);
        currentStep = Cheevos; // next step
    }
    else
    {
        gameID = temp;
        emit dataReceived(TrackerFrame::Summary);
        currentStep = BoxArt; // next step
    }
}

void TrackerFrame::handleBoxArt(QByteArray n)
{
    boximage.loadFromData(n);
    emit dataReceived(TrackerFrame::BoxArt);
    currentStep = Cheevos; // next step
    update();
}

void TrackerFrame::handleIconList(QByteArray n)
{
    emit dataReceived(TrackerFrame::Cheevos);
    currentStep = Idle; // done
    update();
}

void TrackerFrame::handleRank(QByteArray n)
{
    QJsonDocument doc = QJsonDocument::fromJson(n);
    QJsonObject r = doc.object();
    overallScore = r["TotalPoints"].toInt();
    overallRetroScore = r["TotalTruePoints"].toInt();
    rank = r["Rank"].toInt();
    rankTotal = r["TotalRanked"].toInt();
    emit dataReceived(TrackerFrame::Rank);
    currentStep = Summary; // next step
    update();
}
