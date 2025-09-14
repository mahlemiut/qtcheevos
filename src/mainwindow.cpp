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
	  trackerData(new TrackerData),
	  sortWindow(new SortWindow(this))
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
	timer->setInterval(ui->spin_refreshtime->value() * 1000);

	// read display group order
	for(int i = 0; i < LAST; i++)
	{
		tracker->getOrder()[i].order = settings.value(QString("order_%1").arg(i), i).toInt();		
		switch(tracker->getOrder()[i].order)  // fill in group names
		{
			case Rank:
				sortWindow->addItem(tracker->getOrder()[i].order, "Ranking");
				break;
			case GameName:
				sortWindow->addItem(tracker->getOrder()[i].order, "Game Name");
				break;
			case Progress:
				sortWindow->addItem(tracker->getOrder()[i].order, "Progress");
				break;
			case Icons:
				sortWindow->addItem(tracker->getOrder()[i].order, "Achievement Icons");
				break;
		}
	}
	//sortWindow->sortItems();
	connect(sortWindow, &SortWindow::closed, this, &MainWindow::sortwindow_closed);
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

	// save group order
	for (int i = 0; i < LAST; i++) // order
	{
		settings.setValue(QString("order_%1").arg(i), tracker->getOrder()[i].order);
	}
	
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
	QFont f = QFontDialog::getFont(&ok, ui->choosefont->font(), this);
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

void MainWindow::showsortwindow_clicked()
{
	if(sortWindow->isVisible())
		sortWindow->hide();
	else
	{
		sortWindow->show();
		sortWindow->raise();
		sortWindow->activateWindow();
	}
}

void MainWindow::sortwindow_closed()
{
	// Handle the sort window closed event
	for(int x=0;x<LAST;x++)
	{
		tracker->getOrder()[x].order = sortWindow->getIndex(x);
		//tracker->getOrder()[x].group_name = sortWindow->getItemText(x);
	}
	sortWindow->hide();
}

// Tracker Window implementation
TrackerWindow::TrackerWindow(TrackerData* d, QWidget *parent)
	: data(d),
	  QWidget(parent),
	  frame(new TrackerFrame(this))
{
	setGeometry(50, 50, 448, 800);
	setMinimumSize(QSize(448, 800));
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
	if(frame->getCheevoLoadPhase() != IconList::Icons)  // Icons don't seem to always download if interrupted by a regular refresh
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
	iconlist(new IconList(this)),
	errorState(false),
	errorString("")
{
	setFrameStyle(QFrame::Panel);
	setLineWidth(0);
}

TrackerFrame::~TrackerFrame()
{
}

/**
 * @brief Handles the paint event for the TrackerFrame.
 * 
 * This function is responsible for rendering the contents of the TrackerFrame.
 * It uses QPainter to draw various elements such as background, game information,
 * achievement progress, and achievement icons based on the current state and settings.
 * 
 * @param event Pointer to the QPaintEvent object containing event data.
 * 
 * The function performs the following tasks:
 * - Fills the background with a specified color or an image if available.
 * - Draws the total points and rank if the showrank flag is true.
 * - Draws the game title and system name if the showgamename flag is true.
 * - Draws the achievement and score progress if the showprogress flag is true.
 * - Draws the achievement icons if the showcheevoicons flag is true.
 */
void TrackerFrame::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
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

	if(errorState)
	{
		font.setPointSize(24);
		painter.setFont(font);
		painter.setPen(Qt::yellow);
		painter.drawText(QRect(0, 0, width(), height()), Qt::AlignCenter | Qt::TextWordWrap, errorString);
		return;
	}

	if(gamename.isEmpty())
		return;

	painter.setPen(Qt::white);

	for(int x = 0;x < LAST; x++)
	{
		int g = order[x].order;
		{
			switch(g)
			{
				case Groups::Rank:
					ShowRank(painter, &currentline);
					break;
				case Groups::GameName:
					ShowGameName(painter, &currentline);
					break;
				case Groups::Progress:
					ShowProgress(painter, &currentline);
					break;
				case Groups::Icons:
					ShowCheevoIcons(painter, &currentline);
					break;
			}
		}
	}
}

int TrackerFrame::ShowRank(QPainter &painter, int *currentline)
{
	if(showrank)
	{
		QRect bounds;
		QString text;
		font.setPointSize(16);
		painter.setFont(font);

		text = "Total Points: " + QString::number(overallScore) + " (Retro: " + QString::number(overallRetroScore) + ")";
		bounds = painter.boundingRect(QRect(0,*currentline,width(),height()), Qt::AlignHCenter | Qt::TextWordWrap, text);
		painter.drawText(bounds, Qt::AlignHCenter | Qt::TextWordWrap, text);
		*currentline += bounds.height()+5;

		text = "Rank: " + QString::number(rank) + "/" + QString::number(rankTotal);
		bounds = painter.boundingRect(QRect(0,*currentline,width(),height()), Qt::AlignHCenter | Qt::TextWordWrap, text);
		painter.drawText(bounds, Qt::AlignHCenter | Qt::TextWordWrap, text);
		*currentline += bounds.height()+5;
		painter.drawLine(0, *currentline, width(), *currentline);
		*currentline += 5;
	}
	return *currentline;
}

int TrackerFrame::ShowGameName(QPainter &painter, int *currentline)
{
	if(showgamename)
	{
		QRect bounds;
		QString text;
		font.setPointSize(24);
		painter.setFont(font);
	
		bounds = painter.boundingRect(QRect(0,*currentline,width(),height()), Qt::AlignHCenter | Qt::TextWordWrap, gamename);
		painter.drawText(bounds, Qt::AlignHCenter | Qt::TextWordWrap, gamename);
		*currentline += bounds.height();

		font.setPointSize(12);
		painter.setFont(font);
		bounds = painter.boundingRect(QRect(0,*currentline,width(),height()), Qt::AlignHCenter | Qt::TextWordWrap, systemname);
		painter.drawText(bounds, Qt::AlignHCenter | Qt::TextWordWrap, systemname);
		*currentline += bounds.height() + 5;
		painter.drawLine(0, *currentline, width(), *currentline);
		*currentline += 5;
	}

	return *currentline;
}

int TrackerFrame::ShowProgress(QPainter &painter, int *currentline)
{
	if(showprogress)
	{
		QRect bounds;
		QString text;
		font.setPointSize(20);
		painter.setFont(font);

		text = "Achievements: " + QString::number(cheevoUnlocks) + "/" + QString::number(cheevoTotal);
		bounds = painter.boundingRect(QRect(0,*currentline,width(),height()), Qt::AlignHCenter | Qt::TextWordWrap, text);
		painter.drawText(bounds, Qt::AlignHCenter | Qt::TextWordWrap, text);
		*currentline += bounds.height()+5;

		text = "Score: " + QString::number(score) + "/" + QString::number(maxscore);
		bounds = painter.boundingRect(QRect(0,*currentline,width(),height()), Qt::AlignHCenter | Qt::TextWordWrap, text);
		painter.drawText(bounds, Qt::AlignHCenter | Qt::TextWordWrap, text);
		*currentline += bounds.height()+5;
		painter.drawLine(0, *currentline, width(), *currentline);
		*currentline += 5;
	}

	return *currentline;
}

int TrackerFrame::ShowCheevoIcons(QPainter &painter, int *currentline)
{
	if(showcheevoicons)
	{
		int iconSize = 64;
		int iconsperline = width() / iconSize;
		int totalLines = (iconlist->getIconList()->size() + iconsperline - 1) / iconsperline;
		if (*currentline + totalLines * iconSize > height())
		{
			iconSize = 32; // Reduce icon size to fit
			iconsperline = width() / iconSize;
			totalLines /= 2;
		}

		if(iconlist->getIconList()->size() > 0)
		{
			int x = 0;
			int y = *currentline;
			for (int i = 0; i < iconlist->getIconList()->size(); i++)
			{
				if (x >= iconsperline)
				{
					x = 0;
					y += iconSize;
				}
				QRect iconRect(x * iconSize, y, iconSize, iconSize);
				painter.drawImage(iconRect, iconlist->getIconList()->at(i).image);
				if(iconlist->getIconList()->at(i).unlocked == false)
					painter.fillRect(iconRect, QColor(0,0,0,196));
				x++;
			}
		}
		*currentline += (totalLines * iconSize) + 5; // Add some space after the icons
		painter.drawLine(0, *currentline, width(), *currentline);
		*currentline += 5;
	}

	return *currentline;
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


/**
 * @brief Handles the summary data received in JSON format.
 *
 * This function processes the JSON data received, extracts relevant information
 * about the game, and updates the member variables accordingly. It then emits
 * signals based on whether the game ID has changed or not, and sets the next
 * step in the process.
 *
 * @param n The JSON data received as a QByteArray.
 */
void TrackerFrame::handleSummary(QByteArray n)
{
	QJsonDocument doc = QJsonDocument::fromJson(n);
	QJsonArray played = doc.array();
	int temp;

	if(played.isEmpty())
	{
		QJsonObject obj = doc.object();
		errorState = true;
		if (obj.contains("message"))
			errorString = obj["message"].toString();
		else
			errorString = "Error accessing API, check key.";
		currentStep = Idle;
		emit dataReceived(TrackerFrame::Idle);
		return;
	}

	errorState = false;
	gamename = played[0].toObject()["Title"].toString();
	systemname = played[0].toObject()["ConsoleName"].toString();
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
