/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024 The TokTok team.
 */

#include "debuglogmodel.h"

#include <QColor>
#include <QRegularExpression>

namespace {
QtMsgType parseMsgType(const QString& type)
{
    if (type == "Debug") {
        return QtDebugMsg;
    }
    if (type == "Info") {
        return QtInfoMsg;
    }
    if (type == "Warning") {
        return QtWarningMsg;
    }
    if (type == "Critical") {
        return QtCriticalMsg;
    }
    if (type == "Fatal") {
        return QtFatalMsg;
    }
    qWarning() << "Unknown message type:" << type;
    return QtInfoMsg;
}

QString renderMsgType(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return QStringLiteral("Debug");
    case QtInfoMsg:
        return QStringLiteral("Info");
    case QtWarningMsg:
        return QStringLiteral("Warning");
    case QtCriticalMsg:
        return QStringLiteral("Critical");
    case QtFatalMsg:
        return QStringLiteral("Fatal");
    }
    qWarning() << "Unknown message type:" << type;
    return QStringLiteral("Unknown");
}

QList<DebugLogModel::LogEntry> parse(const QStringList& logs)
{
    QList<DebugLogModel::LogEntry> result;
    for (const QString& log : logs) {
        // Regex extraction of log entry
        // [12:35:16.634 UTC] src/core/core.cpp:370 : Debug: Connected to a TCP relay
        //  ^                 ^                 ^     ^      ^
        //  time              file              line  type   message
        const QRegularExpression re(R"(\[(.*) UTC\] (.*):(\d+) : ([^:]+): (.*))");
        const auto match = re.match(log);
        if (!match.hasMatch()) {
            qWarning() << "Failed to parse log entry:" << log;
            continue;
        }

        DebugLogModel::LogEntry entry;
        entry.index = result.size();
        entry.time = match.captured(1);
        entry.file = match.captured(2);
        entry.line = match.captured(3).toInt();
        entry.type = parseMsgType(match.captured(4));
        entry.message = match.captured(5);
        result.append(entry);
    }
    return result;
}

QString render(const DebugLogModel::LogEntry& entry)
{
    return QStringLiteral("[%1] %2:%3 : %4: %5")
        .arg(entry.time)
        .arg(entry.file)
        .arg(entry.line)
        .arg(renderMsgType(entry.type))
        .arg(entry.message);
}

bool filterAccepts(DebugLogModel::Filter filter, QtMsgType type)
{
    switch (filter) {
    case DebugLogModel::All:
        return true;
    case DebugLogModel::Debug:
        return type == QtDebugMsg;
    case DebugLogModel::Info:
        return type == QtInfoMsg;
    case DebugLogModel::Warning:
        return type == QtWarningMsg;
    case DebugLogModel::Critical:
        return type == QtCriticalMsg;
    case DebugLogModel::Fatal:
        return type == QtFatalMsg;
    }
    return false;
}
} // namespace

DebugLogModel::DebugLogModel(const QStringList& logs, QObject* parent)
    : QAbstractListModel(parent)
    , logs_(parse(logs))
{
    recomputeFilter();
}

DebugLogModel::~DebugLogModel() = default;

int DebugLogModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return filteredLogs_.size();
}

QVariant DebugLogModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return render(filteredLogs_.at(index.row()));
    }

    if (role == Qt::ForegroundRole) {
        switch (filteredLogs_.at(index.row()).type) {
        case QtDebugMsg:
            return QColor(Qt::white);
        case QtInfoMsg:
            return QColor(Qt::cyan);
        case QtWarningMsg:
            return QColor(255, 165, 0); // orange
        case QtCriticalMsg:
            return QColor(Qt::red);
        case QtFatalMsg:
            return QColor(255, 64, 64);
        }
    }

    return QVariant();
}

void DebugLogModel::reload(const QStringList& newLogs)
{
    logs_ = parse(newLogs);
    recomputeFilter();
}

void DebugLogModel::setFilter(Filter filter)
{
    filter_ = filter;
    recomputeFilter();
}

int DebugLogModel::originalIndex(const QModelIndex& index) const
{
    return filteredLogs_.at(index.row()).index;
}

void DebugLogModel::recomputeFilter()
{
    beginResetModel();
    filteredLogs_.clear();
    for (const LogEntry& entry : logs_) {
        if (filterAccepts(filter_, entry.type)) {
            filteredLogs_.append(entry);
        }
    }
    endResetModel();
}
