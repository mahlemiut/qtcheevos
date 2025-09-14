// Sort order settings window

#include "sortwindow.h"

SortWindow::SortWindow(QWidget *parent) :
	QDialog(parent),
	listView(new QListView(this))
{
	setWindowTitle("Sort Order Settings");
	setMinimumSize(300, 200);

	listView->setModel(new QMoveStringListModel(this));
	listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	listView->setSelectionMode(QAbstractItemView::SingleSelection);
	listView->setAlternatingRowColors(true);
	listView->setDefaultDropAction(Qt::MoveAction);
	listView->setDragDropMode(QAbstractItemView::InternalMove);
	listView->setFlow(QListView::TopToBottom);
	listView->setMovement(QListView::Free);
	listView->setGridSize(QSize(width(), 30));

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(listView);
	
	setLayout(layout);
//    auto model = qobject_cast<QMoveStringListModel*>(listView->model());
//	    if (model)
//			connect(model, &QMoveStringListModel::orderChanged, this, &SortWindow::sortItems);
}

void SortWindow::addItem(int row, QString text)
{
	QMoveStringListModel *model = qobject_cast<QMoveStringListModel *>(listView->model());
	if (model) 
	{
		QStringList list = model->stringList();
		QList<int> indexList = model->getIndexList();
		list.append(text);
		indexList.append(row);

		model->setStringList(list);
		model->setIndexList(indexList);
	}
}

bool QMoveStringListModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	if (action == Qt::MoveAction && data->hasFormat("application/x-qabstractitemmodeldatalist"))
	{
		QByteArray encoded = data->data("application/x-qabstractitemmodeldatalist");
		QDataStream stream(encoded);
		QStringList items = stringList();
		QList<int> indexList = getIndexList();

		int src;
		QString str;
		int r = row;

		stream >> src;

		if(r == -1)
			r = parent.row();

//			if(src < r)
//				r--;

		// Handle the move operation
		std::cout << "Swapping items " << src << " and " << r << std::endl;
		indexList.swapItemsAt(src, r);
		items.swapItemsAt(src, r);
		setIndexList(indexList);
		setStringList(items);
		//emit orderChanged();

		return true;
	}
	return QStringListModel::dropMimeData(data, action, row, column, parent);
}

