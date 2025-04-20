#include "MessageListModel.h"

#include <QPainter>
#include <QPainterPath>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <Ela/ElaTheme.h>

using namespace MessageList;

MessageDelegate::MessageDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

void MessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QRect rect = option.rect;
    painter->save();

    // 获取消息文本
    QString message = index.data(Qt::DisplayRole).toString();
    // 获取发送者信息
    int sender = index.data(SenderRole).toInt();

    // 计算可用宽度
    int maxBubbleWidth = rect.width() * 0.8;
    int bubblePadding = 8;

    // 创建文本文档来处理多行文本
    QTextDocument doc;
    doc.setDefaultFont(painter->font());
    doc.setTextWidth(maxBubbleWidth - bubblePadding);
    doc.setPlainText(message);

//     计算实际文本尺寸
    int textWidth = qMin(int(doc.idealWidth() + bubblePadding), maxBubbleWidth);
    int textHeight = doc.size().height();

    // 计算气泡尺寸
    int bubbleWidth = textWidth + bubblePadding;
    int bubbleHeight = textHeight + bubblePadding;

    // 确定气泡位置
    QRect bubbleRect;
    if (sender == 0) { // 对方消息
        bubbleRect = QRect(rect.left() + 50, rect.top() + 10, bubbleWidth, bubbleHeight);
    } else { // 自己消息
        bubbleRect = QRect(rect.right() - bubbleWidth - 50, rect.top() + 10, bubbleWidth, bubbleHeight);
    }

    // 绘制背景
    QColor bgColor;
    if (sender == 0) {  // 对方消息
        bgColor = bubbleColor_;
    } else {           // 自己消息
        bgColor = QColor("#6699FF");
    }

    // 绘制气泡
    QPainterPath path;
    path.addRoundedRect(bubbleRect, 10, 10);
    painter->fillPath(path, bgColor);

    // 绘制头像
    QRect avatarRect;
    if (sender == 0) {
        avatarRect = QRect(rect.left() + 15, rect.top() + 10, 30, 30);
    } else {
        avatarRect = QRect(rect.right() - 45, rect.top() + 10, 30, 30);
    }
    QPixmap avatar(":images/resource/pic/Cirno.png");
    painter->drawPixmap(avatarRect, avatar);

    // 绘制消息文本
    painter->translate(bubbleRect.left() + bubblePadding / 2, bubbleRect.top() + bubblePadding / 2);
    QAbstractTextDocumentLayout::PaintContext ctx;
    if (sender != 0) {  // 自己消息使用白色文本
        ctx.palette.setColor(QPalette::Text, Qt::white);
    }

    // 正确定位文本文档
    doc.documentLayout()->draw(painter, ctx);
//    painter->translate(-(bubbleRect.left() + bubblePadding), -(bubbleRect.top() + bubblePadding));
    painter->restore();
}

QSize MessageDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // fix bug: 需要切换主题高度才正确，每一项的高度在改变大小时不太正确

    // 获取消息文本
    QString message = index.data(Qt::DisplayRole).toString();
    // 计算可用宽度
    int maxBubbleWidth = option.rect.width() * 0.8;
    int bubblePadding = 8;

    // 使用文本文档计算多行文本高度
    QTextDocument doc;
    doc.setDefaultFont(option.font);
    doc.setTextWidth(maxBubbleWidth - bubblePadding);
    doc.setPlainText(message);

    // 计算所需高度
    int textHeight = doc.size().height();
    int bubbleHeight = textHeight + bubblePadding;

    return QSize(option.rect.width(), qMax(bubbleHeight + 20, 60));
}

MessageListModel::MessageListModel(QObject *parent) : QStandardItemModel(parent) {}

Qt::ItemFlags MessageListModel::flags(const QModelIndex &index) const {
    if(!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant MessageListModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid()) return QVariant();
    QStandardItem *item = itemFromIndex(index);
    switch (role) {
        case Qt::DisplayRole:
            return item->data(Qt::DisplayRole);
        case SenderRole:
            return item->data(SenderRole);
        default:
            return QVariant();
    }
}

// sender: 0-对方，1-自己
void MessageListModel::addMessage(const QString &message, int sender, uint64_t seq) {
    QStandardItem *item = new QStandardItem(message);
    item->setData(-1, IdRole);  // 初始ID设为-1表示未确认
    item->setData(sender, SenderRole);
    item->setData(seq, SeqRole);  // 保存消息关联的ws数据包的序列号
    appendRow(item);
}

void MessageListModel::updateMessageId(uint64_t seq, int64_t messageId) {
    // 根据seq找到对应的消息
    for(int i = rowCount() - 1; i >= 0; --i) {
        QStandardItem* item = this->item(i);
        if(item->data(SeqRole).toULongLong() == seq) {
            item->setData(messageId, IdRole);
            break;
        }
    }
}
