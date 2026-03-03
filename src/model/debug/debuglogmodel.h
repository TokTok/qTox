/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QLoggingCategory>

class DebugLogModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Filter
    {
        All,
        Debug,
        Info,
        Warning,
        Critical,
        Fatal,
    };

    Q_ENUM(Filter)

    // [12:35:16.634 UTC] src/core/core.cpp:370 : Debug: Connected to a TCP relay
    struct LogEntry
    {
        /// Index in the original log list.
        int index;

        QDateTime time;
        QString category;
        QString file;
        int line;
        QtMsgType type;
        QString message;
    };

    explicit DebugLogModel(QObject* parent);
    ~DebugLogModel() override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void reload(const QStringList& newLogs);

    void setFilter(Filter filter);

    /**
     * @brief Given the index in the filtered list, return the index in the original list.
     */
    int originalIndex(const QModelIndex& index) const;

    /**
     * @brief Parse a list of log lines into LogEntry objects.
     */
    static QList<LogEntry> parse(const QStringList& logs);

    /**
     * @brief Render a LogEntry object into a string.
     */
    static QString render(const LogEntry& entry, bool includeDate = false);

    static QString renderWithDate(const LogEntry& entry)
    {
        return render(entry, true);
    }

private:
    void recomputeFilter();

    Filter filter_ = All;
    QList<LogEntry> logs_;
    QList<LogEntry> filteredLogs_;
};
