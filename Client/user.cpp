#include "user.h"

void User::setAvatarFromB64(const QString &base64) {
    QImage img;
    img.loadFromData(QByteArray::fromBase64(base64.toUtf8()));
    Avatar_ = img;
}
