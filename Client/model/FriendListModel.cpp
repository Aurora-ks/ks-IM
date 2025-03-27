#include "FriendListModel.h"
#include <QRandomGenerator>
#include <QIcon>
#include <Ela/ElaMessageBar.h>
#include "net.h"
FriendListModel::FriendListModel(QObject *parent)
        : QStandardItemModel(parent) {
}

FriendListModel::~FriendListModel() {
}

Qt::ItemFlags FriendListModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    QVariant typeData = index.data(TypeRole);
    if (typeData.isValid()) {
        if (typeData.toInt() == Group) return Qt::ItemIsEnabled;
        if (typeData.toInt() == Friend) return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    return QStandardItemModel::flags(index);
}

QVariant FriendListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();
    QStandardItem *item = itemFromIndex(index);
    if (!item) return QVariant();
    switch (role) {
        case Qt::DisplayRole:
            if (item->data(TypeRole) == Group) {
                return item->text();
            } else if (item->data(TypeRole) == Friend) {
                FriendTreeViewItem *friendItem = item->data(DataRole).value<FriendTreeViewItem*>();
                return friendItem->getAlias().isEmpty()
                       ? QString(friendItem->getUser().getUserName())
                       : QString("%1 (%2)").arg(friendItem->getAlias()).arg(friendItem->getUser().getUserName());
            }
            break;
        case Qt::DecorationRole:
            // TODO: show icon
            if (item->data(TypeRole) == Friend) {
                FriendTreeViewItem *friendItem = item->data(DataRole).value<FriendTreeViewItem*>();
                if (friendItem->getUser().getUserID() % 2 == 0)
                    return QIcon(":/images/resource/pic/Cirno.png");
                return QIcon(":/images/resource/pic/Avatar.png");
            }
            break;
        case TypeRole:
            return item->data(TypeRole);
        case IDRole:
            return item->data(IDRole);
        default:
            break;
    }
    return QStandardItemModel::data(index, role);
}

void FriendListModel::addGroup(int64_t groupId, const QString &groupName) {
    QStandardItem *item = new QStandardItem(groupName);
    item->setData(Group, TypeRole);
    item->setData(groupId, IDRole);
    groups_.insert(groupId, item);
    appendRow(item);
}

void FriendListModel::addFriend(FriendTreeViewItem *friendItem) {
    QStandardItem *item = new QStandardItem();
    friendItem->setParent(this);
    friendItem->setItem(item);

    item->setData(Friend, TypeRole);
    item->setData(QVariant::fromValue(friendItem), DataRole);
    item->setData(friendItem->getRelationId(), IDRole);
    QStandardItem *grp = groups_[friendItem->getGroupId()];
    grp->appendRow(item);
}

void FriendListModel::updateFriendGrouping(int64_t oldGroupId, FriendTreeViewItem *item) {
    QStandardItem *oldGroupItem = groups_[oldGroupId];
    QStandardItem *newGroupItem = groups_[item->getGroupId()];

    oldGroupItem->takeRow(item->getItem()->row());
    newGroupItem->appendRow(item->getItem());
}
