#ifndef SETTING_H
#define SETTING_H

#include <logger.h>
#include <QSettings>
#include <QSqlDatabase>

enum ErrorType {
    NoError,
    ConnectionError,
    StatementError,
    TransactionError,
    UnknownError,
    InvalidFileType
};

enum SettingFileType {
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
    static setting* getDBInstance(const QString &filename);
    static setting* getIniInstance(const QString &filename);
    static void close();

    explicit setting(const QString &filename, SettingFileType fileType);

    ~setting();

    SettingFileType fileType() const { return fileType_; }

    SettingError remove(const QString &key, const QString &table = QString());

    // INI
    void setValue(const QString &key, const QVariant &value) {
        if (fileType_ != SettingFileType::INI) LOG_FATAL("setting::setValue(): invalid filetype");
        setting_->setValue(key, value);
    }

    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const {
        if (fileType_ != SettingFileType::INI) LOG_FATAL("setting::value(): invalid filetype");
        return setting_->value(key, defaultValue);
    }

    bool contanis(const QString &key) const {
        if (fileType_ != SettingFileType::INI) LOG_FATAL("setting::contanis(): invalid filetype");
        return setting_->contains(key);
    }

    void beginGroup(const QString &prefix) {
        if (fileType_ != SettingFileType::INI) LOG_FATAL("setting::beginGroup(): invalid filetype");
        setting_->beginGroup(prefix);
    }

    void endGroup() {
        if (fileType_ != SettingFileType::INI) LOG_FATAL("setting::endGroup(): invalid filetype");
        setting_->endGroup();
    }

    // SQLite

    SettingError setValueDB(const QString &key, const QString &value, const QString &table = QString());

    std::pair<QString, SettingError> valueDB(const QString &key, const QString &defaultValue = QString()) const;

    std::pair<QString, SettingError> valueDBWithTable(const QString &table, const QString &key,
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
    static QMap<QString, setting*> instance_;

    QSettings *setting_{nullptr};
    QSqlDatabase db_;
    QString filename_;
    SettingFileType fileType_{SettingFileType::NONE};

    std::pair<bool, SettingError> checkTableExists(const QString &table) const;
    SettingError createTable(const QString &table);
};


#endif //SETTING_H
