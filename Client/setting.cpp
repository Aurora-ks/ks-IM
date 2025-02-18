#include "setting.h"

#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>

using enum ErrorType;
using enum SettingFileType;

SettingError::SettingError(ErrorType type, const QString &text): type_(type), text_(text) {
}

SettingError::SettingError(const QSqlError &error) {
    switch (error.type()) {
        case QSqlError::NoError:
            type_ = NoError;
            break;
        case QSqlError::ConnectionError:
            type_ = ConnectionError;
            break;
        case QSqlError::StatementError:
            type_ = StatementError;
            break;
        case QSqlError::TransactionError:
            type_ = TransactionError;
            break;
        case QSqlError::UnknownError:
            type_ = UnknownError;
            break;
    }
    text_ = error.text();
}

SettingError::operator bool() const {
    return type_ != NoError;
}

setting::setting(const QString &filename, const SettingFileType fileType) {
    if (fileType != INI && fileType != SQLite)
        LOG_FATAL(
            "setting::setting(): invalid filetype");
    fileType_ = fileType;
    switch (fileType) {
        case INI:
            setting_ = new QSettings(filename, QSettings::IniFormat);
            break;
        case SQLite:
            db_ = QSqlDatabase::addDatabase("QSQLITE", filename);
        // 检查目录是否存在。不存在则创建
            QFileInfo f(filename);
            QString path = f.path();
            if (!QDir(path).exists())
                QDir().mkpath(path);
            db_.setDatabaseName(filename);
            filename_ = f.baseName();
            if (!db_.open()) {
                throw std::runtime_error("setting::setting(): failed to open database");
            }
            QSqlQuery query(db_);
            if (!query.exec(QString("CREATE TABLE IF NOT EXISTS '%1' (key TEXT PRIMARY KEY, value TEXT)").arg(filename_)))
                throw std::runtime_error("setting::setting(): failed to create default table");
    }
}

setting::~setting() {
    switch (fileType_) {
        case INI:
            delete setting_;
            break;
        case SQLite:
            db_.close();
            break;
        default:
            break;
    }
}

SettingError setting::remove(const QString &key, const QString &table) {
    if (fileType_ != INI && fileType_ != SQLite) LOG_FATAL("setting::remove(): invalid filetype");
    if (fileType_ == INI) {
        setting_->remove(key);
        return SettingError();
    }
    QSqlQuery query(db_);
    query.prepare(QString("DELETE FROM '%1' WHERE key = :k").arg(table.isEmpty() ? filename_ : table));
    query.bindValue(":k", key);
    query.exec();
    return SettingError(query.lastError());
}

SettingError setting::dbSetValue(const QString &key, const QString &value, const QString &table) {
    if (fileType_ != INI && fileType_ !=  SQLite) LOG_FATAL("setting::setValue(): invalid filetype");
    if (fileType_ == INI) {
        setting_->setValue(key, value);
        return SettingError();
    }
    QSqlQuery query(db_);
    query.prepare(QString("INSERT OR REPLACE INTO '%1' (key, value) VALUES (:k, :v)").arg(table.isEmpty() ? filename_ : table));
    query.bindValue(":k", key);
    query.bindValue(":v", value);
    query.exec();
    return SettingError(query.lastError());
}

std::pair<QString, SettingError> setting::dbValue(const QString &key, const QString &defaultValue) const {
    if (fileType_ != SQLite) LOG_FATAL("setting::value(): invalid filetype");
    QSqlQuery query(db_);
    query.prepare(QString("SELECT value FROM '%1' WHERE key = :k").arg(filename_));
    query.bindValue(":k", key);
    if (!query.exec())
        return std::make_pair(defaultValue, SettingError(query.lastError()));
    if (query.next())
        return std::make_pair(query.value(0).toString(), SettingError());
    return std::make_pair(defaultValue, SettingError());
}

std::pair<QString, SettingError> setting::dbValue(const QString &table, const QString &key,
                                                const QString &defaultValue) const {
    if (fileType_ != SQLite) LOG_FATAL("setting::value(): invalid filetype");
    QSqlQuery query(db_);
    query.prepare(QString("SELECT value FROM '%1' WHERE key = :k").arg(table));
    query.bindValue(":k", key);
    if (!query.exec())
        return std::make_pair(defaultValue, SettingError(query.lastError()));
    if (query.next())
        return std::make_pair(query.value(0).toString(), SettingError());
    return std::make_pair(defaultValue, SettingError());
}

SettingError setting::setEntries(const QMap<QString, QString> &entries, const QString &table) {
    if (fileType_ != SQLite) LOG_FATAL("setting::setEntries(): invalid filetype");
    QSqlQuery query(db_);
    query.prepare(QString("INSERT OR REPLACE INTO '%1' (key, value) VALUES (:k, :v)").arg(table.isEmpty() ? filename_ : table));
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        query.bindValue(":k", it.key());
        query.bindValue(":v", it.value());
        if (!query.exec())
            return SettingError(query.lastError());
    }
    return SettingError();
}

std::pair<QMap<QString, QString>, SettingError> setting::entries(const QString &table) const {
    if (fileType_ != SQLite) LOG_FATAL("setting::entries(): invalid filetype");
    QSqlQuery query(db_);
    query.prepare(QString("SELECT key, value FROM '%1'").arg(table.isEmpty() ? filename_ : table));
    if (!query.exec())
        return std::make_pair(QMap<QString, QString>(), SettingError(query.lastError()));
    QMap<QString, QString> entries;
    while (query.next())
        entries.insert(query.value(0).toString(), query.value(1).toString());
    return std::make_pair(entries, SettingError());
}

std::pair<QList<QString>, SettingError> setting::keys(const QString &table) const {
    if (fileType_ != SQLite) LOG_FATAL("setting::keys(): invalid filetype");
    QSqlQuery query(db_);
    query.prepare(QString("SELECT key FROM '%1'").arg(table.isEmpty() ? filename_ : table));
    if (!query.exec())
        return std::make_pair(QList<QString>(), SettingError(query.lastError()));
    QList<QString> keys;
    while (query.next())
        keys.append(query.value(0).toString());
    return std::make_pair(keys, SettingError());
}
