/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/core/toxfile.h"
#include "src/core/corefile.h"

#include <QLabel>
#include <QListWidgetItem>
#include <QHash>
#include <QString>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QTableView>

class ContentLayout;
class QTableView;
class Settings;
class Style;
class QFileInfo;
class IMessageBoxManager;
class FriendList;

namespace FileTransferList
{

    enum class Column : int {
        // NOTE: Order defines order in UI
        fileName,
        contact,
        progress,
        size,
        speed,
        status,
        control,
        invalid
    };

    Column toFileTransferListColumn(int in);
    QString toQString(Column column);

    enum class EditorAction : int {
        pause,
        cancel,
        invalid,
    };

    EditorAction toEditorAction(int in);

    class Model : public QAbstractTableModel
    {
        Q_OBJECT
    public:
        Model(FriendList& friendList, QObject* parent = nullptr);
        ~Model() = default;

        void onFileUpdated(const ToxFile& file);

        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    signals:
        void togglePause(ToxFile file);
        void cancel(ToxFile file);

    private:
        QHash<QByteArray /*file id*/, int /*row index*/> idToRow;
        std::vector<ToxFile> files;
        FriendList& friendList;
    };

    class Delegate : public QStyledItemDelegate
    {
    public:
        Delegate(Settings& settings, Style& style, QWidget* parent = nullptr);
        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

        bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
    private:
        Settings& settings;
        Style& style;
    };

    class View : public QTableView
    {
    public:
        View(QAbstractItemModel* model, Settings& settings, Style& style,
            QWidget* parent = nullptr);
        ~View();

    };
} // namespace FileTransferList

class FilesForm : public QObject
{
    Q_OBJECT

public:
    FilesForm(CoreFile& coreFile, Settings& settings, Style& style,
        IMessageBoxManager& messageBoxManager, FriendList& friendList);
    ~FilesForm();

    bool isShown() const;
    void show(ContentLayout* contentLayout);

public slots:
    void onFileUpdated(const ToxFile& file);

private slots:
    void onSentFileActivated(const QModelIndex& index);
    void onReceivedFileActivated(const QModelIndex& index);

private:
    struct FileInfo
    {
        QListWidgetItem* item = nullptr;
        ToxFile file;
    };

    void retranslateUi();

    QWidget* head;
    QLabel headLabel;
    QVBoxLayout headLayout;
    QTabWidget main;
    QTableView *sent, *recvd;
    FileTransferList::Model *sentModel, *recvdModel;
    IMessageBoxManager& messageBoxManager;
};
