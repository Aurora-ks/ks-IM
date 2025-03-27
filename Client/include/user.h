#ifndef USER_H
#define USER_H

#include <QPixmap>
#include "afx.h"

class User {
public:
    enum Gender {
        UNKNOWN,
        MALE,
        FEMALE
    };
    MEM_CREATE_D(int64_t , UserID)
    MEM_CREATE_D(QString, UserName)
    MEM_CREATE_D(Gender, Gender)
    MEM_CREATE_D(QPixmap, Avatar)
    MEM_CREATE_D(QString, Email)
    MEM_CREATE_D(QString, Phone)

public:
    explicit User() = default;
    explicit User(int64_t userID, QString userName, Gender gender, QPixmap avatar, QString email, QString phone);
    explicit User(int64_t userID, QString userName, int gender, QString avatar, QString email, QString phone);
    ~User() = default;

    void setAvatarFromB64(const QString &base64);
    QString UserIDToString();
    static void SetUid(int64_t uid) { userId_ = uid; }
    static int64_t GetUid() { return userId_; }

private:
    static int64_t userId_;
};



#endif //USER_H
