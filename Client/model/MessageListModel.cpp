#include "MessageListModel.h"

#include <QPainter>
#include <QPainterPath>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QPixmap>
#include <QBoxLayout>
#include <QLabel>
#include <QApplication>
#include <Ela/ElaTheme.h>
#include <Ela/ElaScrollArea.h>
#include <Ela/ElaIcon.h>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>

using namespace MessageList;
const int maxImageWidth = 200;  // 图片固定最大宽度
const int maxImageHeight = 200; // 图片固定最大高度

MessageDelegate::MessageDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

ImagePreviewDialog::ImagePreviewDialog(const QPixmap &pixmap, QWidget *parent): QDialog(parent) {
    setWindowTitle("图片预览");
    setModal(true);
    QVBoxLayout * layout = new QVBoxLayout(this);
    // 创建滚动区域
    ElaScrollArea *scrollArea = new ElaScrollArea(this);
//    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    // 创建图片标签
    QLabel *imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    // 获取屏幕尺寸
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int maxWidth = screenGeometry.width() * 0.8;  // 屏幕宽度的80%
    int maxHeight = screenGeometry.height() * 0.8; // 屏幕高度的80%
    // 计算缩放后的尺寸
    QSize originalSize = pixmap.size();
    QSize scaledSize = originalSize;

    if (originalSize.width() > maxWidth || originalSize.height() > maxHeight) {
        scaledSize = originalSize.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio);
    }
    // 设置图片
    imageLabel->setPixmap(pixmap.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    // 设置滚动区域的内容
    scrollArea->setWidget(imageLabel);
    // 设置对话框大小
    resize(qMin(scaledSize.width() + 40, maxWidth + 40),
           qMin(scaledSize.height() + 40, maxHeight + 40));

    layout->addWidget(scrollArea);

}

void MessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QRect rect = option.rect;
    painter->save();

    // 获取消息类型
    int type = index.data(TypeRole).toInt();
    // 获取发送者信息
    int sender = index.data(SenderRole).toInt();

    // 绘制头像
    QRect avatarRect;
    if (sender == 0) {
        avatarRect = QRect(rect.left() + 15, rect.top() + 10, 30, 30);
    } else {
        avatarRect = QRect(rect.right() - 45, rect.top() + 10, 30, 30);
    }
    QPixmap avatar(":images/resource/pic/Cirno.png");
    painter->drawPixmap(avatarRect, avatar);

    if (type == MessageList::Text) {
        // 获取消息文本
        QString message = index.data(Qt::DisplayRole).toString();

        // 计算可用宽度
        int maxBubbleWidth = rect.width() * 0.8;
        int bubblePadding = 8;

        // 创建文本文档来处理多行文本
        QTextDocument doc;
        doc.setDefaultFont(painter->font());
        doc.setTextWidth(maxBubbleWidth - bubblePadding);
        doc.setPlainText(message);

        // 计算实际文本尺寸
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

        // 绘制消息文本
        painter->save();  // 保存当前状态
        painter->translate(bubbleRect.left() + bubblePadding / 2, bubbleRect.top() + bubblePadding / 2);
        QAbstractTextDocumentLayout::PaintContext ctx;
        if (sender != 0) {  // 自己消息使用白色文本
            ctx.palette.setColor(QPalette::Text, Qt::white);
        }
        doc.documentLayout()->draw(painter, ctx);
        painter->restore();  // 恢复状态
    } else if (type == MessageList::Image) {
        // 获取图片数据
        QPixmap pixmap = index.data(Qt::DisplayRole).value<QPixmap>();
        if (pixmap.isNull()) {
            painter->restore();
            return;
        }

        // 获取原始尺寸
        QSize originalSize = pixmap.size();

        // 计算缩放后的尺寸
        QSize scaledSize = originalSize;
        if (originalSize.width() > maxImageWidth && originalSize.height() > maxImageHeight) {
            scaledSize = originalSize.scaled(maxImageWidth, maxImageHeight, Qt::KeepAspectRatio);
        } else if (originalSize.width() > maxImageWidth) {
            scaledSize = originalSize.scaled(maxImageWidth, originalSize.height(), Qt::KeepAspectRatio);
        } else if (originalSize.height() > maxImageHeight) {
            scaledSize = originalSize.scaled(originalSize.width(), maxImageHeight, Qt::KeepAspectRatio);
        }

        // 计算图片位置
        QRect imageRect;
        if (sender == 0) { // 对方消息
            imageRect = QRect(rect.left() + 50, rect.top() + 10, scaledSize.width(), scaledSize.height());
        } else { // 自己消息
            imageRect = QRect(rect.right() - scaledSize.width() - 50, rect.top() + 10, scaledSize.width(),
                              scaledSize.height());
        }
        // 绘制图片
        if (scaledSize == originalSize)
            painter->drawPixmap(imageRect, pixmap);
        else
            painter->drawPixmap(imageRect, pixmap.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else if (type == MessageList::File) {
        // 获取文件信息
        QString fileName = index.data(MessageList::FileNameRole).toString();
        qint64 fileSize = index.data(MessageList::FileSizeRole).toLongLong();
        // 计算文件大小显示
        QString sizeStr;
        if (fileSize < 1024) {
            sizeStr = QString::number(fileSize) + " B";
        } else if (fileSize < 1024 * 1024) {
            sizeStr = QString::number(fileSize / 1024.0, 'f', 1) + " KB";
        } else if (fileSize < 1024 * 1024 * 1024) {
            sizeStr = QString::number(fileSize / (1024.0 * 1024.0), 'f', 1) + " MB";
        } else {
            sizeStr = QString::number(fileSize / (1024.0 * 1024.0 * 1024.0), 'f', 1) + " GB";
        }

        // 计算气泡尺寸
        int maxBubbleWidth = rect.width() * 0.6;
        int bubblePadding = 8;
        int iconSize = 32;
        int textWidth = maxBubbleWidth - iconSize - bubblePadding * 3;

        // 创建文本文档来处理文件名
        QTextDocument doc;
        doc.setDefaultFont(painter->font());
        doc.setTextWidth(textWidth);
        doc.setPlainText(fileName);

        // 计算实际文本尺寸
        int textHeight = doc.size().height();
        int bubbleHeight = qMax(textHeight + 20, iconSize) + bubblePadding * 2;

        // 确定气泡位置
        QRect bubbleRect;
        if (sender == 0) { // 对方消息
            bubbleRect = QRect(rect.left() + 50, rect.top() + 10, maxBubbleWidth, bubbleHeight);
        } else { // 自己消息
            bubbleRect = QRect(rect.right() - maxBubbleWidth - 50, rect.top() + 10, maxBubbleWidth, bubbleHeight);
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

        // 绘制文件图标
        QRect iconRect;
        if (sender == 0) {
            iconRect = QRect(bubbleRect.left() + bubblePadding,
                           bubbleRect.top() + (bubbleHeight - iconSize) / 2,
                           iconSize, iconSize);
        } else {
            iconRect = QRect(bubbleRect.left() + bubblePadding,
                           bubbleRect.top() + (bubbleHeight - iconSize) / 2,
                           iconSize, iconSize);
        }
        QIcon fileIcon = ElaIcon::getInstance()->getElaIcon(ElaIconType::IconName::File, iconSize);
        painter->drawPixmap(iconRect, fileIcon.pixmap(iconSize, iconSize));

        // 绘制文件名和大小
        painter->save();
        painter->translate(bubbleRect.left() + iconSize + bubblePadding * 2,
                         bubbleRect.top() + bubblePadding);
        QAbstractTextDocumentLayout::PaintContext ctx;
        if (sender != 0) {  // 自己消息使用白色文本
            ctx.palette.setColor(QPalette::Text, Qt::white);
        }
        doc.documentLayout()->draw(painter, ctx);

        // 绘制文件大小
        painter->setPen(sender == 0 ? Qt::gray : Qt::white);
        painter->drawText(0, textHeight + 5, textWidth, 20, Qt::AlignLeft | Qt::AlignVCenter, sizeStr);
        painter->restore();
    }

    painter->restore();
}

QSize MessageDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    int type = index.data(TypeRole).toInt();

    if (type == Text) {
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
    } else if (type == Image) {
        // 获取图片数据
        QPixmap pixmap = index.data(Qt::DisplayRole).value<QPixmap>();
        if (pixmap.isNull()) return QSize(option.rect.width(), 60);

        // 获取原始尺寸
        QSize originalSize = pixmap.size();

        // 计算缩放后的尺寸
        QSize scaledSize = originalSize;
        if (originalSize.width() > maxImageWidth && originalSize.height() > maxImageHeight) {
            scaledSize = originalSize.scaled(maxImageWidth, maxImageHeight, Qt::KeepAspectRatio);
        } else if (originalSize.width() > maxImageWidth) {
            scaledSize = originalSize.scaled(maxImageWidth, originalSize.height(), Qt::KeepAspectRatio);
        } else if (originalSize.height() > maxImageHeight) {
            scaledSize = originalSize.scaled(originalSize.width(), maxImageHeight, Qt::KeepAspectRatio);
        }
        if (scaledSize == originalSize)
            return QSize(option.rect.width(), qMax(originalSize.height() + 20, 60));
        else
            return QSize(option.rect.width(), qMax(scaledSize.height() + 20, 60));
    } else if (type == File) {
        QString fileName = index.data(MessageList::FileNameRole).toString();
        int maxBubbleWidth = option.rect.width() * 0.6;
        int bubblePadding = 8;
        int iconSize = 32;
        int textWidth = maxBubbleWidth - iconSize - bubblePadding * 3;

        QTextDocument doc;
        doc.setDefaultFont(option.font);
        doc.setTextWidth(textWidth);
        doc.setPlainText(fileName);

        int textHeight = doc.size().height();
        int bubbleHeight = qMax(textHeight + 20, iconSize) + bubblePadding * 2;

        return QSize(option.rect.width(), bubbleHeight + 20);
    }

    return QSize(option.rect.width(), 60);
}

bool MessageDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) {
    if (event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            int type = index.data(MessageList::TypeRole).toInt();
            if (type == MessageList::Image) {
                // 获取图片数据
                QPixmap pixmap = index.data(Qt::DisplayRole).value<QPixmap>();
                if (!pixmap.isNull()) {
                    // 创建并显示图片预览对话框
                    ImagePreviewDialog *dialog = new ImagePreviewDialog(pixmap);
                    dialog->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动删除
                    dialog->show();
                    return true;
                }
            } else if (type == MessageList::File) {
                QString filePath = index.data(MessageList::FilePathRole).toString();
                if (!filePath.isEmpty()) {
                    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
                    return true;
                }
            }
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

MessageListModel::MessageListModel(QObject *parent) : QStandardItemModel(parent) {}

Qt::ItemFlags MessageListModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant MessageListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();
    QStandardItem *item = itemFromIndex(index);
    switch (role) {
        case Qt::DisplayRole:
            return item->data(Qt::DisplayRole);
        case Qt::DecorationRole:
            return item->data(Qt::DecorationRole);
        case SenderRole:
            return item->data(SenderRole);
        case TypeRole:
            return item->data(TypeRole);
        case FileNameRole:
            return item->data(FileNameRole);
        case FileSizeRole:
            return item->data(FileSizeRole);
        case FilePathRole:
            return item->data(FilePathRole);
        default:
            return QVariant();
    }
}

// sender: 0-对方，1-自己
void MessageListModel::addMessage(const QString &message, int sender, uint64_t seq) {
    QStandardItem *item = new QStandardItem(message);
    item->setData(-1, IdRole);  // 初始ID设为-1表示未确认
    item->setData(sender, SenderRole);
    item->setData(Text, TypeRole);
    item->setData(seq, SeqRole);  // 保存消息关联的ws数据包的序列号
    appendRow(item);
}

void MessageListModel::addMessage(const QImage &image, int sender, uint64_t seq) {
    QStandardItem *item = new QStandardItem();
    item->setData(-1, IdRole);  // 初始ID设为-1表示未确认
    item->setData(sender, SenderRole);
    item->setData(seq, SeqRole);  // 保存消息关联的ws数据包的序列号
    item->setData(Image, TypeRole);  // 设置消息类型为图片
    item->setData(QPixmap::fromImage(image), Qt::DisplayRole);  // 存储图片数据
    appendRow(item);
}

void MessageListModel::addMessage(const MessageList::FileInfo &fileInfo, int sender, uint64_t seq) {
    QStandardItem *item = new QStandardItem();
    item->setData(-1, IdRole);  // 初始ID设为-1表示未确认
    item->setData(sender, SenderRole);
    item->setData(seq, SeqRole);  // 保存消息关联的ws数据包的序列号
    item->setData(File, TypeRole);  // 设置消息类型为文件
    item->setData(fileInfo.fileName, FileNameRole);
    item->setData(fileInfo.fileSize, FileSizeRole);
    item->setData(fileInfo.filePath, FilePathRole);
    appendRow(item);
}

void MessageListModel::updateMessageId(uint64_t seq, int64_t messageId) {
    // 根据seq找到对应的消息
    for (int i = rowCount() - 1; i >= 0; --i) {
        QStandardItem *item = this->item(i);
        if (item->data(SeqRole).toULongLong() == seq) {
            item->setData(messageId, IdRole);
            break;
        }
    }
}
