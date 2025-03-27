#include "user.h"

int64_t User::userId_;

User::User(int64_t userID, QString userName, User::Gender gender, QPixmap avatar,
           QString email, QString phone)
        : UserID_(userID), UserName_(userName), Gender_(gender), Avatar_(avatar), Email_(email), Phone_(phone) {
}
void User::setAvatarFromB64(const QString &base64) {
    QImage img;
    img.loadFromData(QByteArray::fromBase64(base64.toUtf8()));
    Avatar_ = QPixmap::fromImage(img);
}

QString User::UserIDToString() {
    return QString::number(UserID_);
}

User::User(int64_t userID, QString userName, int gender, QString avatar, QString email, QString phone)
:UserID_(userID), UserName_(userName), Avatar_(QPixmap(avatar)), Email_(email), Phone_(phone){
    switch (gender) {
        case 1:
            Gender_ = Gender::MALE;
            return;
        case 2:
            Gender_ = Gender::FEMALE;
            return;
        default:
            Gender_ = Gender::UNKNOWN;
            return;
    }
}
