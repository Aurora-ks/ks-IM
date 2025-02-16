#ifndef SETTING_H
#define SETTING_H

#include <QSettings>
#include <QSqlDatabase>

enum class ErrorType {
    NoError,
    ConnectionError,
    StatementError,
    TransactionError,
    UnknownError,
    InvalidFileType
};

enum class SettingFileType {
    NONE,
    INI,
    SQLite
};

class SettingError {
public:
    explicit SettingError(ErrorType type = ErrorType::NoError, const QString &text = QString());

    explicit SettingError(const QSqlError &error);

    explicit operator bool() const;

    ErrorType type() const { return type_; }
    QString text() const { return text_; }

private:
    ErrorType type_;
    QString text_;
};

class setting {
public:
    explicit setting(const QString &filename, SettingFileType fileType);

    ~setting();

    SettingError remove(const QString &key, const QString &table = QString());

    // INI
    void setValue(const QString &key, const QVariant &value) {
        if (fileType_ != SettingFileType::INI) throw std::invalid_argument("setting::setting(): invalid filetype");
        setting_->setValue(key, value);
    }

    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const {
        if (fileType_ != SettingFileType::INI) throw std::invalid_argument("setting::value(): invalid filetype");
        return setting_->value(key, defaultValue);
    }

    bool contanis(const QString &key) const {
        if (fileType_ != SettingFileType::INI) throw std::invalid_argument("setting::contanis(): invalid filetype");
        return setting_->contains(key);
    }

    void beginGroup(const QString &prefix) {
        if (fileType_ != SettingFileType::INI) throw std::invalid_argument("setting::beginGroup(): invalid filetype");
        setting_->beginGroup(prefix);
    }

    void endGroup() {
        if (fileType_ != SettingFileType::INI) throw std::invalid_argument("setting::endGroup(): invalid filetype");
        setting_->endGroup();
    }

    // SQLite

    SettingError dbSetValue(const QString &key, const QString &value, const QString &table = QString());
    std::pair<QString, SettingError> dbValue(const QString &key, const QString &defaultValue = QString()) const;

    std::pair<QString, SettingError> dbValue(const QString &table, const QString &key,
                                           const QString &defaultValue = QString()) const;

    SettingError setEntries(const QMap<QString, QString> &entries, const QString &table = QString());

    std::pair<QMap<QString, QString>, SettingError> entries(const QString &table = QString()) const;

    std::pair<QList<QString>, SettingError> keys(const QString &table = QString()) const;

    bool beginTransaction() {
        return db_.transaction();
    }

    bool commit() {
        return db_.commit();
    }

    bool rollback() {
        return db_.rollback();
    }

private:
    QSettings *setting_{nullptr};
    QSqlDatabase db_;
    QString filename_;
    SettingFileType fileType_{SettingFileType::NONE};
};


#endif //SETTING_H
