/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2022 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "src/model/friendlist/friendlistmanager.h"

#include <QSignalSpy>
#include <QTest>

class MockFriend : public IFriendListItem
{
public:
    MockFriend()
        : name("No Name")
        , lastActivity(QDateTime::currentDateTime())
        , online(false)
    {
    }

    MockFriend(const QString& nameStr, bool onlineRes, const QDateTime& lastAct)
        : name(nameStr)
        , lastActivity(lastAct)
        , online(onlineRes)
    {
    }

    ~MockFriend();

    bool isFriend() const override
    {
        return true;
    }
    bool isConference() const override
    {
        return false;
    }
    bool isOnline() const override
    {
        return online;
    }
    bool widgetIsVisible() const override
    {
        return visible;
    }

    QString getNameItem() const override
    {
        return name;
    }
    QDateTime getLastActivity() const override
    {
        return lastActivity;
    }
    QWidget* getWidget() override
    {
        return nullptr;
    }

    void setWidgetVisible(bool v) override
    {
        visible = v;
    }

private:
    QString name;
    QDateTime lastActivity;
    bool online = false;
    bool visible = true;
};

MockFriend::~MockFriend() = default;

class MockConference : public IFriendListItem
{
public:
    MockConference()
        : name("conference")
    {
    }

    MockConference(const QString& nameStr)
        : name(nameStr)
    {
    }

    ~MockConference();

    bool isFriend() const override
    {
        return false;
    }
    bool isConference() const override
    {
        return true;
    }
    bool isOnline() const override
    {
        return true;
    }
    bool widgetIsVisible() const override
    {
        return visible;
    }

    QString getNameItem() const override
    {
        return name;
    }
    QDateTime getLastActivity() const override
    {
        return QDateTime::currentDateTime();
    }
    QWidget* getWidget() override
    {
        return nullptr;
    }

    void setWidgetVisible(bool v) override
    {
        visible = v;
    }

private:
    QString name;
    bool visible = true;
};

MockConference::~MockConference() = default;

class FriendItemsBuilder
{
public:
    FriendItemsBuilder* addOfflineFriends()
    {
        QStringList testNames{".test",
                              "123",
                              "A test user",
                              "Aatest user",
                              "atest",
                              "btest",
                              "ctest",
                              "Test user",
                              "user with long nickname one",
                              "user with long nickname two"};

        for (int i = 0; i < testNames.size(); ++i) {
            int unsortedIndex = i % 2 ? i - 1 : testNames.size() - i - 1; // Mixes positions
            int sortedByActivityIndex = testNames.size() - i - 1;
            unsortedAllFriends.append(testNames[unsortedIndex]);
            sortedByNameOfflineFriends.append(testNames[i]);
            sortedByActivityFriends.append(testNames[sortedByActivityIndex]);
        }

        return this;
    }

    FriendItemsBuilder* addOnlineFriends()
    {
        QStringList testNames{".test online",
                              "123 online",
                              "A test user online",
                              "Aatest user online",
                              "atest online",
                              "btest online",
                              "ctest online",
                              "Test user online",
                              "user with long nickname one online",
                              "user with long nickname two online"};

        for (int i = 0; i < testNames.size(); ++i) {
            int unsortedIndex = i % 2 ? i - 1 : testNames.size() - i - 1;
            int sortedByActivityIndex = testNames.size() - i - 1;
            unsortedAllFriends.append(testNames[unsortedIndex]);
            sortedByNameOnlineFriends.append(testNames[i]);
            sortedByActivityFriends.append(testNames[sortedByActivityIndex]);
        }

        return this;
    }

    FriendItemsBuilder* addConferences()
    {
        unsortedConferences.append("Test Conference");
        unsortedConferences.append("A Conference");
        unsortedConferences.append("Test Conference long name");
        unsortedConferences.append("Test Conference long aname");
        unsortedConferences.append("123");

        sortedByNameConferences.push_back("123");
        sortedByNameConferences.push_back("A Conference");
        sortedByNameConferences.push_back("Test Conference");
        sortedByNameConferences.push_back("Test Conference long aname");
        sortedByNameConferences.push_back("Test Conference long name");

        return this;
    }

    FriendItemsBuilder* setConferencesOnTop(bool val)
    {
        conferencesOnTop = val;
        return this;
    }

    bool getConferencesOnTop()
    {
        return conferencesOnTop;
    }

    /**
     * @brief buildUnsorted Creates items to init the FriendListManager.
     * FriendListManager will own and manage these items
     * @return Unsorted vector of items
     */
    QVector<IFriendListItem*> buildUnsorted()
    {
        checkDifferentNames();

        QVector<IFriendListItem*> vec;
        for (auto name : unsortedAllFriends) {
            vec.push_back(new MockFriend(name, isOnline(name), getDateTime(name)));
        }
        for (auto name : unsortedConferences) {
            vec.push_back(new MockConference(name));
        }
        clear();
        return vec;
    }

    /**
     * @brief buildSortedByName Create items to compare with items
     * from the FriendListManager. FriendItemsBuilder owns these items
     * @return Sorted by name vector of items
     */
    QVector<std::shared_ptr<IFriendListItem>> buildSortedByName()
    {
        QVector<std::shared_ptr<IFriendListItem>> vec;
        if (!conferencesOnTop) {
            for (auto name : sortedByNameOnlineFriends) {
                vec.push_back(std::shared_ptr<IFriendListItem>(
                    new MockFriend(name, true, QDateTime::currentDateTime())));
            }

            for (auto name : sortedByNameConferences) {
                vec.push_back(std::shared_ptr<IFriendListItem>(new MockConference(name)));
            }
        } else {
            for (auto name : sortedByNameConferences) {
                vec.push_back(std::shared_ptr<IFriendListItem>(new MockConference(name)));
            }

            for (auto name : sortedByNameOnlineFriends) {
                vec.push_back(std::shared_ptr<IFriendListItem>(
                    new MockFriend(name, true, QDateTime::currentDateTime())));
            }
        }

        for (auto name : sortedByNameOfflineFriends) {
            vec.push_back(
                std::shared_ptr<IFriendListItem>(new MockFriend(name, false, getDateTime(name))));
        }
        clear();
        return vec;
    }

    /**
     * @brief buildSortedByActivity Creates items to compare with items
     * from FriendListManager. FriendItemsBuilder owns these items
     * @return Sorted by activity vector of items
     */
    QVector<std::shared_ptr<IFriendListItem>> buildSortedByActivity()
    {
        QVector<std::shared_ptr<IFriendListItem>> vec;

        // Add conferences on top
        for (auto name : sortedByNameConferences) {
            vec.push_back(std::shared_ptr<IFriendListItem>(new MockConference(name)));
        }

        // Add friends and set the date of the last activity by index
        QDateTime dateTime = QDateTime::currentDateTime();
        for (int i = 0; i < sortedByActivityFriends.size(); ++i) {
            QString name = sortedByActivityFriends.at(i);
            vec.push_back(std::shared_ptr<IFriendListItem>(
                new MockFriend(name, isOnline(name), getDateTime(name))));
        }
        clear();
        return vec;
    }

private:
    void clear()
    {
        sortedByNameOfflineFriends.clear();
        sortedByNameOnlineFriends.clear();
        sortedByNameConferences.clear();
        sortedByActivityFriends.clear();
        sortedByActivityConferences.clear();
        unsortedAllFriends.clear();
        unsortedConferences.clear();
        conferencesOnTop = true;
    }

    bool isOnline(const QString& name)
    {
        return sortedByNameOnlineFriends.indexOf(name) != -1;
    }

    /**
     * @brief checkDifferentNames The check is necessary for
     * the correct setting of the online status
     */
    void checkDifferentNames()
    {
        for (auto name : sortedByNameOnlineFriends) {
            if (sortedByNameOfflineFriends.contains(name, Qt::CaseInsensitive)) {
                QFAIL("Names in sortedByNameOnlineFriends and sortedByNameOfflineFriends "
                      "should be different");
                break;
            }
        }
    }

    QDateTime getDateTime(const QString& name)
    {
        QDateTime dateTime = QDateTime::currentDateTime();
        int pos = sortedByActivityFriends.indexOf(name);
        if (pos == -1) {
            return dateTime;
        }
        const int dayRatio = -1;
        return dateTime.addDays(dayRatio * pos * pos);
    }

    QStringList sortedByNameOfflineFriends;
    QStringList sortedByNameOnlineFriends;
    QStringList sortedByNameConferences;
    QStringList sortedByActivityFriends;
    QStringList sortedByActivityConferences;
    QStringList unsortedAllFriends;
    QStringList unsortedConferences;
    bool conferencesOnTop = true;
};

class TestFriendListManager : public QObject
{
    Q_OBJECT
private slots:
    void testAddFriendListItem();
    void testSortByName();
    void testSortByActivity();
    void testSetFilter();
    void testApplyFilterSearchString();
    void testApplyFilterByStatus();
    void testSetConferencesOnTop();

private:
    std::unique_ptr<FriendListManager> createManagerWithItems(const QVector<IFriendListItem*> itemsVec);
};

void TestFriendListManager::testAddFriendListItem()
{
    auto manager = std::unique_ptr<FriendListManager>(new FriendListManager(0, this));
    QSignalSpy spy(manager.get(), &FriendListManager::itemsChanged);
    FriendItemsBuilder listBuilder;

    auto checkFunc = [&](const QVector<IFriendListItem*> itemsVec) {
        for (auto item : itemsVec) {
            manager->addFriendListItem(item);
        }
        QCOMPARE(manager->getItems().size(), itemsVec.size());
        QCOMPARE(spy.count(), itemsVec.size());
        spy.clear();
        for (auto item : itemsVec) {
            manager->removeFriendListItem(item);
        }
        QCOMPARE(manager->getItems().size(), 0);
        QCOMPARE(spy.count(), itemsVec.size());
        spy.clear();
    };

    // Only friends
    checkFunc(listBuilder.addOfflineFriends()->buildUnsorted());
    checkFunc(listBuilder.addOfflineFriends()->addOnlineFriends()->buildUnsorted());
    // Friends and conferences
    checkFunc(listBuilder.addOfflineFriends()->addConferences()->buildUnsorted());
    checkFunc(listBuilder.addOfflineFriends()->addOnlineFriends()->addConferences()->buildUnsorted());
    // Only conferences
    checkFunc(listBuilder.addConferences()->buildUnsorted());
}

void TestFriendListManager::testSortByName()
{
    FriendItemsBuilder listBuilder;
    auto unsortedVec =
        listBuilder.addOfflineFriends()->addOnlineFriends()->addConferences()->buildUnsorted();
    auto sortedVec =
        listBuilder.addOfflineFriends()->addOnlineFriends()->addConferences()->buildSortedByName();
    auto manager = createManagerWithItems(unsortedVec);

    manager->sortByName();
    bool success = manager->getPositionsChanged();
    manager->sortByName();

    QCOMPARE(success, true);
    QCOMPARE(manager->getPositionsChanged(), false);
    QCOMPARE(manager->getItems().size(), sortedVec.size());
    QCOMPARE(manager->getConferencesOnTop(), listBuilder.getConferencesOnTop());

    for (int i = 0; i < sortedVec.size(); ++i) {
        IFriendListItem* fromManager = manager->getItems().at(i).get();
        std::shared_ptr<IFriendListItem> fromSortedVec = sortedVec.at(i);
        QCOMPARE(fromManager->getNameItem(), fromSortedVec->getNameItem());
    }
}

void TestFriendListManager::testSortByActivity()
{
    FriendItemsBuilder listBuilder;
    auto unsortedVec =
        listBuilder.addOfflineFriends()->addOnlineFriends()->addConferences()->buildUnsorted();
    auto sortedVec =
        listBuilder.addOfflineFriends()->addOnlineFriends()->addConferences()->buildSortedByActivity();

    std::unique_ptr<FriendListManager> manager = createManagerWithItems(unsortedVec);
    manager->sortByActivity();
    bool success = manager->getPositionsChanged();
    manager->sortByActivity();

    QCOMPARE(success, true);
    QCOMPARE(manager->getPositionsChanged(), false);
    QCOMPARE(manager->getItems().size(), sortedVec.size());
    for (int i = 0; i < sortedVec.size(); ++i) {
        auto fromManager = manager->getItems().at(i).get();
        auto fromSortedVec = sortedVec.at(i);
        QCOMPARE(fromManager->getNameItem(), fromSortedVec->getNameItem());
    }
}

void TestFriendListManager::testSetFilter()
{
    FriendItemsBuilder listBuilder;
    auto manager = createManagerWithItems(
        listBuilder.addOfflineFriends()->addOnlineFriends()->addConferences()->buildUnsorted());
    QSignalSpy spy(manager.get(), &FriendListManager::itemsChanged);

    manager->setFilter("", false, false, false);

    QCOMPARE(spy.count(), 0);

    manager->setFilter("Test", true, false, false);
    manager->setFilter("Test", true, false, false);

    QCOMPARE(spy.count(), 1);
}

void TestFriendListManager::testApplyFilterSearchString()
{
    FriendItemsBuilder listBuilder;
    auto manager = createManagerWithItems(
        listBuilder.addOfflineFriends()->addOnlineFriends()->addConferences()->buildUnsorted());
    QVector<std::shared_ptr<IFriendListItem>> resultVec;
    QString testNameA = "NOITEMSWITHTHISNAME";
    QString testNameB = "Test Name B";
    manager->sortByName();
    manager->setFilter(testNameA, false, false, false);
    manager->applyFilter();

    resultVec = manager->getItems();
    for (auto item : resultVec) {
        QCOMPARE(item->widgetIsVisible(), false);
    }

    manager->sortByActivity();
    manager->addFriendListItem(new MockFriend(testNameB, true, QDateTime::currentDateTime()));
    manager->applyFilter();

    resultVec = manager->getItems();
    for (auto item : resultVec) {
        QCOMPARE(item->widgetIsVisible(), false);
    }

    manager->addFriendListItem(new MockFriend(testNameA, true, QDateTime::currentDateTime()));
    manager->applyFilter();

    resultVec = manager->getItems();
    for (auto item : resultVec) {
        if (item->getNameItem() == testNameA) {
            QCOMPARE(item->widgetIsVisible(), true);
        } else {
            QCOMPARE(item->widgetIsVisible(), false);
        }
    }

    manager->setFilter("", false, false, false);
    manager->applyFilter();

    resultVec = manager->getItems();
    for (auto item : resultVec) {
        QCOMPARE(item->widgetIsVisible(), true);
    }
}

void TestFriendListManager::testApplyFilterByStatus()
{
    FriendItemsBuilder listBuilder;
    auto manager = createManagerWithItems(
        listBuilder.addOfflineFriends()->addOnlineFriends()->addConferences()->buildUnsorted());
    auto onlineItems = listBuilder.addOnlineFriends()->buildSortedByName();
    auto offlineItems = listBuilder.addOfflineFriends()->buildSortedByName();
    auto conferenceItems = listBuilder.addConferences()->buildSortedByName();
    manager->sortByName();

    manager->setFilter("", true /*hideOnline*/, false /*hideOffline*/, false /*hideConferences*/);
    manager->applyFilter();

    for (auto item : manager->getItems()) {
        if (item->isOnline() && item->isFriend()) {
            QCOMPARE(item->widgetIsVisible(), false);
        } else {
            QCOMPARE(item->widgetIsVisible(), true);
        }
    }

    manager->setFilter("", false /*hideOnline*/, true /*hideOffline*/, false /*hideConferences*/);
    manager->applyFilter();

    for (auto item : manager->getItems()) {
        if (item->isOnline()) {
            QCOMPARE(item->widgetIsVisible(), true);
        } else {
            QCOMPARE(item->widgetIsVisible(), false);
        }
    }

    manager->setFilter("", false /*hideOnline*/, false /*hideOffline*/, true /*hideConferences*/);
    manager->applyFilter();

    for (auto item : manager->getItems()) {
        if (item->isConference()) {
            QCOMPARE(item->widgetIsVisible(), false);
        } else {
            QCOMPARE(item->widgetIsVisible(), true);
        }
    }

    manager->setFilter("", true /*hideOnline*/, true /*hideOffline*/, true /*hideConferences*/);
    manager->applyFilter();

    for (auto item : manager->getItems()) {
        QCOMPARE(item->widgetIsVisible(), false);
    }

    manager->setFilter("", false /*hideOnline*/, false /*hideOffline*/, false /*hideConferences*/);
    manager->applyFilter();

    for (auto item : manager->getItems()) {
        QCOMPARE(item->widgetIsVisible(), true);
    }
}

void TestFriendListManager::testSetConferencesOnTop()
{
    FriendItemsBuilder listBuilder;
    auto manager = createManagerWithItems(
        listBuilder.addOfflineFriends()->addOnlineFriends()->addConferences()->buildUnsorted());
    auto sortedVecOnlineOnTop =
        listBuilder.addOfflineFriends()->addOnlineFriends()->addConferences()->setConferencesOnTop(false)->buildSortedByName();
    auto sortedVecConferencesOnTop =
        listBuilder.addOfflineFriends()->addOnlineFriends()->addConferences()->setConferencesOnTop(true)->buildSortedByName();

    manager->setConferencesOnTop(false);
    manager->sortByName();

    for (int i = 0; i < manager->getItems().size(); ++i) {
        auto fromManager = manager->getItems().at(i);
        auto fromSortedVec = sortedVecOnlineOnTop.at(i);
        QCOMPARE(fromManager->getNameItem(), fromSortedVec->getNameItem());
    }

    manager->setConferencesOnTop(true);
    manager->sortByName();

    for (int i = 0; i < manager->getItems().size(); ++i) {
        auto fromManager = manager->getItems().at(i);
        auto fromSortedVec = sortedVecConferencesOnTop.at(i);
        QCOMPARE(fromManager->getNameItem(), fromSortedVec->getNameItem());
    }
}

std::unique_ptr<FriendListManager>
TestFriendListManager::createManagerWithItems(const QVector<IFriendListItem*> itemsVec)
{
    std::unique_ptr<FriendListManager> manager =
        std::unique_ptr<FriendListManager>(new FriendListManager(0, this));

    for (auto item : itemsVec) {
        manager->addFriendListItem(item);
    }

    return manager;
}

QTEST_GUILESS_MAIN(TestFriendListManager)
#include "friendlistmanager_test.moc"
