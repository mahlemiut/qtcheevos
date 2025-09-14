// sort window hearder

#include <QListView>
#include <QDialog>
#include <QVBoxLayout>
#include <QStringListModel>
#include <QMimeData>
#include <iostream>

#pragma once

class QMoveStringListModel : public QStringListModel
{
	Q_OBJECT

public:
	explicit QMoveStringListModel(QObject *parent = nullptr) : QStringListModel(parent) {}

	Qt::DropActions supportedDropActions() const override
	{
		return Qt::MoveAction;
	}

	bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

	QList<int> getIndexList() const
	{
		return indexList;
	}

	void setIndexList(const QList<int> &list)
	{
		indexList = list;
	}
	QList<int> indexList;  // index tied to group name

signals:
	void orderChanged();
};

class SortWindow : public QDialog
{
	Q_OBJECT

public:
	explicit SortWindow(QWidget *parent = nullptr);
	~SortWindow() {};

	void addItem(int row, QString text);
	int getIndex(int row) const
	{
		QMoveStringListModel *m = qobject_cast<QMoveStringListModel *>(listView->model());
		return m->getIndexList().at(row);
	}
	QString getItemText(int row) const
	{
		QMoveStringListModel *m = qobject_cast<QMoveStringListModel *>(listView->model());
		return m->stringList().at(row);
	}

signals:
	void closed();

protected:
	void closeEvent(QCloseEvent *event) override
	{
		emit closed();
		QDialog::closeEvent(event);
	}

private:
	QListView *listView;
};
