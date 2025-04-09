#include "SessionListModel.h"

#include <QPainter>
#include <QPainterPath>

SessionDelegate::SessionDelegate(QObject *parent)
        : QStyledItemDelegate(parent),
          backgroundColor_(Qt::white),
          backgroundColorHover_(240, 240, 240),
          backgroundColorSelected_(204, 235, 255){}

void SessionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // TODO: 宽度改变时文字能自适应增缩

    QRect rect = option.rect;
    painter->save();

    // 绘制背景
    if (option.state & QStyle::State_MouseOver)
        painter->fillRect(rect, backgroundColorHover_);
    else if (option.state & QStyle::State_Selected)
        painter->fillRect(rect, backgroundColorSelected_);
    else
        painter->fillRect(rect, backgroundColor_);

    // 绘制头像
    QRect avatarRect(rect.left() + 10, rect.top() + 10, 48, 48);
    // TODO: read image data
    QPixmap avatar(":images/resource/pic/Cirno.png");
    painter->drawPixmap(avatarRect, avatar);

    // 绘制名称
    QRect groupNameRect(rect.left() + 70, rect.top() + 10, rect.width() - 100, 20);
    QFont font;
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(groupNameRect, Qt::AlignLeft, index.data(NameRole).toString());

    // 绘制消息内容
    QString message = index.data(MessageRole).toString();
    if(!message.isEmpty()){
        QRect contentRect(rect.left() + 70, rect.top() + 30, rect.width() - 100, 20);
        font.setBold(false);
        painter->setFont(font);
        painter->drawText(contentRect, Qt::AlignLeft, message);
    }

    // 绘制时间
    QString time = index.data(TimeRole).toString();
    if(!time.isEmpty()){
        QRect timeRect(rect.right() - 80, rect.top() + 10, 70, 20);
        painter->drawText(timeRect, Qt::AlignRight, time);
    }

    // 绘制未读消息数
    int unreadCount = index.data(UnreadCountRole).toInt();
    if (unreadCount > 0) {
        QRect unreadRect(rect.right() - 40, rect.top() + 30, 30, 20);
        QPainterPath path;
        path.addEllipse(unreadRect.adjusted(2, 2, -2, -2));
        painter->fillPath(path, Qt::gray);
        painter->setPen(Qt::white);
        painter->drawText(unreadRect, Qt::AlignCenter, QString::number(unreadCount));
    }

    painter->restore();
}

QSize SessionDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QSize(option.rect.width(), 70);
}


SessionListModel::SessionListModel(QObject *parent) : QStandardItemModel(parent) {}

QVariant SessionListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();

    QStandardItem *item = itemFromIndex(index);
    if (!item) {
        qWarning() << "[SessionListModel::data] item is null, index:" << index << " role:" << role;
        return QVariant();
    }

    switch (role) {
        case Qt::DisplayRole:
            return item->data(NameRole);
        case Qt::DecorationRole:
            return item->data(AvatarRole);
        case SessionIdRole:
            return item->data(SessionIdRole);
        case AvatarRole:
            return item->data(AvatarRole);
        case NameRole:
            return item->data(NameRole);
        case MessageRole:
            return item->data(MessageRole);
        case TimeRole:
            return item->data(TimeRole);
        case UnreadCountRole:
            return item->data(UnreadCountRole);
        case LastAckRole:
            return item->data(LastAckRole);
        case IsGroupRole:
            return item->data(IsGroupRole);
        default:
            return QVariant();
    }
}

Qt::ItemFlags SessionListModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void SessionListModel::addSession(int64_t sessionId, const QString &sessionName, const QString &sessionAvatar,
                                  const QString &message, const QString &time, int unreadCount) {
    QStandardItem *item = new QStandardItem;
    item->setEditable(false);
    item->setData(sessionId, SessionIdRole);
    item->setData(sessionAvatar, AvatarRole);
    item->setData(sessionName, NameRole);
    item->setData(message, MessageRole);
    item->setData(time, TimeRole);
    item->setData(unreadCount, UnreadCountRole);
    sessions_.insert(sessionId, item);
    appendRow(item);
}

void SessionListModel::addSession(int64_t sessionId, const QString &name, int64_t lastAck, bool isGroup) {
    QStandardItem *item = new QStandardItem;
    item->setData(sessionId, SessionIdRole);
    item->setData(name, NameRole);
    item->setData(lastAck, LastAckRole);
    item->setData(isGroup, IsGroupRole);
    sessions_.insert(sessionId, item);
    appendRow(item);
}

void SessionListModel::updateSession(int64_t sessionId, const QString &message, const QString &time, int unreadCount) {

}

QStandardItem* SessionListModel::getSessionItem(int64_t sessionId) {
    if(sessions_.contains(sessionId)) return sessions_.value(sessionId);
    return nullptr;
}