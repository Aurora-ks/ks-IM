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
    MEM_CREATE_D(QString, UserID)
    MEM_CREATE_D(QString, UserName)
    MEM_CREATE_D(Gender, Gender)
    MEM_CREATE_D(QPixmap, Avatar)
    MEM_CREATE_D(QString, Email)
    MEM_CREATE_D(QString, Phone)

public:
    explicit User() = default;
    ~User() = default;

    void setAvatarFromB64(const QString &base64);
};



#endif //USER_H
