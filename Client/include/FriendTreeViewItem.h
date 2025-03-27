#ifndef FRIENDTREEVIEWITEM_H
#define FRIENDTREEVIEWITEM_H

#include <QObject>
#include <QMetaType>
#include "afx.h"
#include "user.h"

class QStandardItem;

class FriendTreeViewItem : public QObject{
    Q_OBJECT

    MEM_CREATE_D(int64_t, GroupId)
    MEM_CREATE_D(int64_t, RelationId)
    MEM_CREATE_D(QString, Alias)
    MEM_CREATE_D(User, User)
    MEM_CREATE_D(QStandardItem*, Item)
public:
    explicit FriendTreeViewItem(QObject *parent = nullptr);
    explicit FriendTreeViewItem(int64_t relationId, int64_t groupId, const QString &alias, const User &user, QObject *parent = nullptr);
};

Q_DECLARE_METATYPE(FriendTreeViewItem)

#endif //FRIENDTREEVIEWITEM_H
