/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "src/chatlog/textformatter.h"

#include <QString>
#include <QtTest/QtTest>

#include <functional>

#define PAIR_FORMAT(input, output) {QStringLiteral(input), QStringLiteral(output)}

using StringPair = QPair<QString, QString>;

namespace {
const StringPair TAGS[]{
    PAIR_FORMAT("<b>", "</b>"),
    PAIR_FORMAT("<i>", "</i>"),
    PAIR_FORMAT("<u>", "</u>"),
    PAIR_FORMAT("<s>", "</s>"),
    PAIR_FORMAT("<font color=#595959><code>", "</code></font>"),
};

enum StyleType
{
    BOLD,
    ITALIC,
    UNDERLINE,
    STRIKE,
    CODE,
};

/**
 * @brief The MarkdownToTags struct maps sequence of markdown symbols to HTML tags according this
 * sequence
 */
struct MarkdownToTags
{
    QString markdownSequence;
    StringPair htmlTags;
};

const QVector<MarkdownToTags> SINGLE_SIGN_MARKDOWN{
    {QStringLiteral("*"), TAGS[StyleType::BOLD]},
    {QStringLiteral("/"), TAGS[StyleType::ITALIC]},
    {QStringLiteral("_"), TAGS[StyleType::UNDERLINE]},
    {QStringLiteral("~"), TAGS[StyleType::STRIKE]},
    {QStringLiteral("`"), TAGS[StyleType::CODE]},
};

const QVector<MarkdownToTags> DOUBLE_SIGN_MARKDOWN{
    {QStringLiteral("**"), TAGS[StyleType::BOLD]},
    {QStringLiteral("//"), TAGS[StyleType::ITALIC]},
    {QStringLiteral("__"), TAGS[StyleType::UNDERLINE]},
    {QStringLiteral("~~"), TAGS[StyleType::STRIKE]},
};

const QVector<MarkdownToTags> MULTI_SIGN_MARKDOWN{
    {QStringLiteral("```"), TAGS[StyleType::CODE]},
};

/**
 * @brief Creates single container from two
 */
template <class Container>
Container concat(const Container& first, const Container& last)
{
    Container result;
    result.reserve(first.size() + last.size());
    result.append(first);
    result.append(last);
    return result;
}

const QVector<MarkdownToTags> ALL_MARKDOWN_TYPES =
    concat(concat(SINGLE_SIGN_MARKDOWN, DOUBLE_SIGN_MARKDOWN), MULTI_SIGN_MARKDOWN);

const QVector<MarkdownToTags> SINGLE_AND_DOUBLE_MARKDOWN =
    concat(SINGLE_SIGN_MARKDOWN, DOUBLE_SIGN_MARKDOWN);

// any markdown type must work for this data the same way
const QVector<StringPair> COMMON_WORK_CASES{
    PAIR_FORMAT("%1a%1", "%2%1a%1%3"),
    PAIR_FORMAT("%1aa%1", "%2%1aa%1%3"),
    PAIR_FORMAT("%1aaa%1", "%2%1aaa%1%3"),
    // Must allow same formatting more than one time
    PAIR_FORMAT("%1aaa%1 %1aaa%1", "%2%1aaa%1%3 %2%1aaa%1%3"),
};

const QVector<StringPair> SINGLE_SIGN_WORK_CASES{
    PAIR_FORMAT("a %1a%1", "a %2%1a%1%3"),
    PAIR_FORMAT("%1a%1 a", "%2%1a%1%3 a"),
    PAIR_FORMAT("a %1a%1 a", "a %2%1a%1%3 a"),
    // "Lazy" matching
    PAIR_FORMAT("%1aaa%1 aaa%1", "%2%1aaa%1%3 aaa%4"),
};

// only double-sign markdown must work for this data
const QVector<StringPair> DOUBLE_SIGN_WORK_CASES{
    // Must apply formatting to strings which contain reserved symbols
    PAIR_FORMAT("%1aaa%2%1", "%3%1aaa%2%1%4"),
    PAIR_FORMAT("%1%2aaa%1", "%3%1%2aaa%1%4"),
    PAIR_FORMAT("%1aaa%2aaa%1", "%3%1aaa%2aaa%1%4"),
    PAIR_FORMAT("%1%2%2aaa%1", "%3%1%2%2aaa%1%4"),
    PAIR_FORMAT("%1aaa%2%2%1", "%3%1aaa%2%2%1%4"),
    PAIR_FORMAT("%1aaa%2%2aaa%1", "%3%1aaa%2%2aaa%1%4"),
};

// only multi-sign markdown must work for this data
const QVector<StringPair> MULTI_SIGN_WORK_CASES{
    PAIR_FORMAT("%1int main()\n{    return 0;\n}%1", //
                "%2%1"
                "int main()\n{    return 0;\n}"
                "%1%3"),
    // allows new line and spaces to go right after ``` sequence
    PAIR_FORMAT("%1\nint main()\n{    return 0;\n}\n%1", //
                "%2%1"
                "\nint main()\n{    return 0;\n}\n"
                "%1%3"),
    PAIR_FORMAT("%1 int main()\n{    return 0;\n} %1", //
                "%2%1"
                " int main()\n{    return 0;\n} "
                "%1%3"),
};

// any type of markdown must fail for this data
const QVector<QString> COMMON_EXCEPTIONS{
    // No empty formatting string
    QStringLiteral("%1%1"),
    // Formatting string must be enclosed by whitespace symbols, newlines or message start/end
    QStringLiteral("a%1aa%1a"),
    QStringLiteral("%1aa%1a"),
    QStringLiteral("a%1aa%1"),
    QStringLiteral("a %1aa%1a"),
    QStringLiteral("a%1aa%1 a"),
    QStringLiteral("a\n%1aa%1a"),
    QStringLiteral("a%1aa%1\na"),
};

const QVector<QString> SINGLE_AND_DOUBLE_SIGN_EXCEPTIONS{
    // Formatting text must not start/end with whitespace symbols
    QStringLiteral("%1 %1"),
    QStringLiteral("%1 a%1"),
    QStringLiteral("%1a %1"),
    // No newlines
    QStringLiteral("%1\n%1"),
    QStringLiteral("%1aa\n%1"),
    QStringLiteral("%1\naa%1"),
    QStringLiteral("%1aa\naa%1"),
};

// only single-sign markdown must fail for this data
const QVector<QString> SINGLE_SIGN_EXCEPTIONS{
    // Reserved symbols within formatting string are disallowed
    QStringLiteral("%1aa%1a%1"),
    QStringLiteral("%1aa%1%1"),
    QStringLiteral("%1%1aa%1"),
    QStringLiteral("%1%1%1"),
};

const QVector<StringPair> MIXED_FORMATTING_SPECIAL_CASES{
    // Must allow mixed formatting if there is no tag overlap in result
    PAIR_FORMAT("aaa *aaa /aaa/ aaa*", "aaa <b>aaa <i>aaa</i> aaa</b>"),
    PAIR_FORMAT("aaa *aaa /aaa* aaa/", "aaa *aaa <i>aaa* aaa</i>"),
};

#define MAKE_LINK(url) "<a href=\"" url "\">" url "</a>"
#define MAKE_WWW_LINK(url) "<a href=\"http://" url "\">" url "</a>"

const QVector<QPair<QString, QString>> URL_CASES{
    PAIR_FORMAT(                                                //
        "https://github.com/qTox/qTox/issues/4233",             //
        MAKE_LINK("https://github.com/qTox/qTox/issues/4233")), //
    PAIR_FORMAT(                                                //
        "www.youtube.com",                                      //
        MAKE_WWW_LINK("www.youtube.com")),                      //
    PAIR_FORMAT(                                                //
        "https://url.com/some*url/some*more*url/",              //
        MAKE_LINK("https://url.com/some*url/some*more*url/")),  //
    PAIR_FORMAT(                                                //
        "https://url.com/some_url/some_more_url/",              //
        MAKE_LINK("https://url.com/some_url/some_more_url/")),  //
    PAIR_FORMAT(                                                //
        "https://url.com/some~url/some~more~url/",              //
        MAKE_LINK("https://url.com/some~url/some~more~url/")),  //
    // Test case from issue #4275
    PAIR_FORMAT(                                                                             //
        "http://www.metacritic.com/game/pc/mass-effect-andromeda\n"                          //
        "http://www.metacritic.com/game/playstation-4/mass-effect-andromeda\n"               //
        "http://www.metacritic.com/game/xbox-one/mass-effect-andromeda",                     //
        MAKE_LINK("http://www.metacritic.com/game/pc/mass-effect-andromeda") "\n"            //
        MAKE_LINK("http://www.metacritic.com/game/playstation-4/mass-effect-andromeda") "\n" //
        MAKE_LINK("http://www.metacritic.com/game/xbox-one/mass-effect-andromeda")),         //
    PAIR_FORMAT(                                                                             //
        "http://site.com/part1/part2 "                                                       //
        "http://site.com/part3 and one more time "                                           //
        "www.site.com/part1/part2",                                                          //
        MAKE_LINK("http://site.com/part1/part2") " "                                         //
        MAKE_LINK("http://site.com/part3") " and one more time "                             //
        MAKE_WWW_LINK("www.site.com/part1/part2")),                                          //
    PAIR_FORMAT(                                                                             //
        "https://127.0.0.1/asd\n"                                                            //
        "https://ABCD:EF01:2345:6789:ABCD:EF01:2345:6789/\n"                                 //
        "ftp://2001:DB8::8:800:200C:417A/\n"                                                 //
        "http://::1/\n"                                                                      //
        "http://::/\n"                                                                       //
        "https://127.0.0.1:8080/asd "                                                        //
        "https://[ABCD:EF01:2345:6789:ABCD:EF01:2345:6789]:8080/ "                           //
        "ftp://[2001:DB8::8:800:200C:417A]:21/ "                                             //
        "http://[::1]:22/ "                                                                  //
        "http://[::]:20/ ",                                                                  //
        MAKE_LINK("https://127.0.0.1/asd") "\n"                                              //
        MAKE_LINK("https://"                                                                 //
                  "ABCD:EF01:2345:6789:ABCD:EF01:2345:"                                      //
                  "6789/") "\n"                                                              //
        MAKE_LINK("ftp://"                                                                   //
                  "2001:DB8::8:800:"                                                         //
                  "200C:417A/") "\n"                                                         //
        MAKE_LINK("http://::1/") "\n"                                                        //
        MAKE_LINK("http://::/") "\n"                                                         //
        MAKE_LINK("https://127.0.0.1:8080/asd") " " MAKE_LINK(                               //
            "https://"                                                                       //
            "[ABCD:EF01:2345:6789:ABCD:EF01:2345:6789]:"                                     //
            "8080/") " "                                                                     //
        MAKE_LINK("ftp://"                                                                   //
                  "[2001:DB8::8:800:"                                                        //
                  "200C:417A]:21/")                                                          //
        " " MAKE_LINK("http://[::1]:22/") " " MAKE_LINK("http://[::]:20/") " "),             //
    // Test case from issue #4853 (include unicode, ending brackets that are part of URL)
    PAIR_FORMAT(                                                      //
        "https://ja.wikipedia.org/wiki/印章",                         //
        MAKE_LINK("https://ja.wikipedia.org/wiki/印章")),             //
    PAIR_FORMAT(                                                      //
        "https://en.wikipedia.org/wiki/Seal_(East_Asia)",             //
        MAKE_LINK("https://en.wikipedia.org/wiki/Seal_(East_Asia)")), //
    // Test cases from issue #4295 (exclude surrounding quotes, brackets, ending punctuation)
    PAIR_FORMAT(                                                         //
        "(http://www.google.com)",                                       //
        "(" MAKE_LINK("http://www.google.com") ")"),                     //
    PAIR_FORMAT(                                                         //
        "&quot;http://www.google.com&quot;",                             //
        "&quot;" MAKE_LINK("http://www.google.com") "&quot;"),           //
    PAIR_FORMAT(                                                         //
        "http://www.google.com.",                                        //
        MAKE_LINK("http://www.google.com") "."),                         //
    PAIR_FORMAT(                                                         //
        "http://www.google.com,",                                        //
        MAKE_LINK("http://www.google.com") ","),                         //
    PAIR_FORMAT(                                                         //
        "http://www.google.com?",                                        //
        MAKE_LINK("http://www.google.com") "?"),                         //
    PAIR_FORMAT(                                                         //
        "https://google.com?gfe_rd=cr",                                  //
        MAKE_LINK("https://google.com?gfe_rd=cr")),                      //
    PAIR_FORMAT(                                                         //
        "[&quot;https://en.wikipedia.org/wiki/Seal_(East_Asia)&quot;]?", //
        "[&quot;" MAKE_LINK("https://en.wikipedia.org/wiki/Seal_(East_Asia)") "&quot;]?"),
};

const QVector<StringPair> POST_NULL_HTML_CASE{
    // Test case for HTML output (in chat log).
    PAIR_FORMAT("Hello\x00\x00some garbage here", "Hello<font color=\"#228B22\">[...]</font>"),
};

const QVector<StringPair> POST_NULL_PLAIN_CASE{
    // Test case for plain text output (in notifications).
    PAIR_FORMAT("Hello\x00\x00some garbage here", "Hello[...]"),
};

#undef PAIR_FORMAT

using MarkdownFunction = QString (*)(const QString&, bool);
using InputProcessor = std::function<QString(const QString&, const MarkdownToTags&)>;
using OutputProcessor = std::function<QString(const QString&, const MarkdownToTags&, bool)>;

/**
 * @brief Testing cases where markdown must work
 * @param applyMarkdown Function which is used to apply markdown
 * @param markdownToTags Which markdown type to test
 * @param testData Test data - string pairs "Source message - Message after formatting"
 * @param showSymbols True if it is supposed to leave markdown symbols after formatting, false
 * otherwise
 * @param processInput Test data is a template, which must be expanded with concrete markdown
 * symbols, everytime in different way. This function determines how to expand source message
 * depending on user need
 * @param processOutput Same as previous parameter but is applied to markdown output
 */
void workCasesTest(MarkdownFunction applyMarkdown, const QVector<MarkdownToTags>& markdownToTags,
                   const QVector<StringPair>& testData, bool showSymbols,
                   InputProcessor processInput = nullptr, OutputProcessor processOutput = nullptr)
{
    for (const MarkdownToTags& mtt : markdownToTags) {
        for (const StringPair& data : testData) {
            const QString input = processInput != nullptr ? processInput(data.first, mtt) : data.first;
            QString output = processOutput != nullptr ? processOutput(data.second, mtt, showSymbols)
                                                      : data.second;
            QString result = applyMarkdown(input, showSymbols);
            QVERIFY(output == result);
        }
    }
}

/*
 * @brief Testing cases for TRIfA suffix.
 * @param testData Test data - string pairs "Source message - Message after formatting"
 * @param True if suffixes should be replaced by [...]
 */
void postNullCasesTest(const QVector<StringPair>& testData, bool html)
{
    for (const StringPair& data : testData) {
        QString result = TextFormatter::processPostNullSuffix(data.first, html);
        QCOMPARE(data.second, result);
    }
}

/**
 * @brief Testing cases where markdown must not to work
 * @param applyMarkdown Function which is used to apply markdown
 * @param markdownToTags Which markdown type to test
 * @param exceptions Collection of "source message - markdown result" pairs representing cases
 * where markdown must not to work
 * @param showSymbols True if it is supposed to leave markdown symbols after formatting, false
 * otherwise
 */
void exceptionsTest(MarkdownFunction applyMarkdown, const QVector<MarkdownToTags>& markdownToTags,
                    const QVector<QString>& exceptions, bool showSymbols)
{
    for (const MarkdownToTags& mtt : markdownToTags) {
        for (const QString& e : exceptions) {
            QString processedException = e.arg(mtt.markdownSequence);
            QVERIFY(processedException == applyMarkdown(processedException, showSymbols));
        }
    }
}

/**
 * @brief Testing some uncommon work cases
 * @param applyMarkdown Function which is used to apply markdown
 * @param pairs Collection of "source message - markdown result" pairs representing cases where
 * markdown must not to work
 */
void specialCasesTest(MarkdownFunction applyMarkdown, const QVector<StringPair>& pairs)
{
    for (const auto& p : pairs) {
        QString result = applyMarkdown(p.first, false);
        QVERIFY(p.second == result);
    }
}

using UrlHighlightFunction = QString (*)(const QString&);

/**
 * @brief Function for testing URL highlighting
 * @param data Test data - map of "URL - HTML-wrapped URL"
 */
void urlHighlightTest(UrlHighlightFunction function, const QVector<QPair<QString, QString>>& data)
{
    for (const QPair<QString, QString>& p : data) {
        QString result = function(p.first);
        QVERIFY(p.second == result);
    }
}

QString commonWorkCasesProcessInput(const QString& str, const MarkdownToTags& mtt)
{
    return str.arg(mtt.markdownSequence);
}

QString commonWorkCasesProcessOutput(const QString& str, const MarkdownToTags& mtt, bool showSymbols)
{
    const StringPair& tags = mtt.htmlTags;
    return str.arg(showSymbols ? mtt.markdownSequence : QString{}).arg(tags.first).arg(tags.second);
}

QString singleSignWorkCasesProcessInput(const QString& str, const MarkdownToTags& mtt)
{
    return str.arg(mtt.markdownSequence);
}

QString singleSignWorkCasesProcessOutput(const QString& str, const MarkdownToTags& mtt, bool showSymbols)
{
    const StringPair& tags = mtt.htmlTags;
    return str.arg(showSymbols ? mtt.markdownSequence : "").arg(tags.first).arg(tags.second).arg(mtt.markdownSequence);
}

QString doubleSignWorkCasesProcessInput(const QString& str, const MarkdownToTags& mtt)
{
    return str.arg(mtt.markdownSequence).arg(mtt.markdownSequence[0]);
}

QString doubleSignWorkCasesProcessOutput(const QString& str, const MarkdownToTags& mtt, bool showSymbols)
{
    const StringPair& tags = mtt.htmlTags;
    return str.arg(showSymbols ? mtt.markdownSequence : "")
        .arg(mtt.markdownSequence[0])
        .arg(tags.first)
        .arg(tags.second);
}

QString multiSignWorkCasesProcessInput(const QString& str, const MarkdownToTags& mtt)
{
    return str.arg(mtt.markdownSequence);
}

QString multiSignWorkCasesProcessOutput(const QString& str, const MarkdownToTags& mtt, bool showSymbols)
{
    const auto tags = mtt.htmlTags;
    return str.arg(showSymbols ? mtt.markdownSequence : "", tags.first, tags.second);
}

} // namespace

class TestTextFormatter : public QObject
{
    Q_OBJECT
private slots:
    void commonWorkCasesShowSymbols();
    void commonWorkCasesHideSymbols();
    void singleSignWorkCasesShowSymbols();
    void singleSignWorkCasesHideSymbols();
    void doubleSignWorkCasesShowSymbols();
    void doubleSignWorkCasesHideSymbols();
    void multiSignWorkCasesHideSymbols();
    void multiSignWorkCasesShowSymbols();
    void commonExceptionsShowSymbols();
    void commonExceptionsHideSymbols();
    void singleSignExceptionsShowSymbols();
    void singleSignExceptionsHideSymbols();
    void singleAndDoubleMarkdownExceptionsShowSymbols();
    void singleAndDoubleMarkdownExceptionsHideSymbols();
    void mixedFormattingSpecialCases();
    void postNullTagsHtml();
    void postNullTagsPlain();
    void urlTest();

private:
    const MarkdownFunction markdownFunction = TextFormatter::applyMarkdown;
    UrlHighlightFunction urlHighlightFunction = TextFormatter::highlightURI;
};

void TestTextFormatter::commonWorkCasesShowSymbols()
{
    workCasesTest(markdownFunction, ALL_MARKDOWN_TYPES, COMMON_WORK_CASES, true,
                  commonWorkCasesProcessInput, commonWorkCasesProcessOutput);
}

void TestTextFormatter::commonWorkCasesHideSymbols()
{
    workCasesTest(markdownFunction, ALL_MARKDOWN_TYPES, COMMON_WORK_CASES, false,
                  commonWorkCasesProcessInput, commonWorkCasesProcessOutput);
}

void TestTextFormatter::singleSignWorkCasesShowSymbols()
{
    workCasesTest(markdownFunction, SINGLE_SIGN_MARKDOWN, SINGLE_SIGN_WORK_CASES, true,
                  singleSignWorkCasesProcessInput, singleSignWorkCasesProcessOutput);
}

void TestTextFormatter::singleSignWorkCasesHideSymbols()
{
    workCasesTest(markdownFunction, SINGLE_SIGN_MARKDOWN, SINGLE_SIGN_WORK_CASES, false,
                  singleSignWorkCasesProcessInput, singleSignWorkCasesProcessOutput);
}

void TestTextFormatter::doubleSignWorkCasesShowSymbols()
{
    workCasesTest(markdownFunction, DOUBLE_SIGN_MARKDOWN, DOUBLE_SIGN_WORK_CASES, true,
                  doubleSignWorkCasesProcessInput, doubleSignWorkCasesProcessOutput);
}

void TestTextFormatter::doubleSignWorkCasesHideSymbols()
{
    workCasesTest(markdownFunction, DOUBLE_SIGN_MARKDOWN, DOUBLE_SIGN_WORK_CASES, false,
                  doubleSignWorkCasesProcessInput, doubleSignWorkCasesProcessOutput);
}

void TestTextFormatter::multiSignWorkCasesHideSymbols()
{
    workCasesTest(markdownFunction, MULTI_SIGN_MARKDOWN, MULTI_SIGN_WORK_CASES, false,
                  multiSignWorkCasesProcessInput, multiSignWorkCasesProcessOutput);
}

void TestTextFormatter::multiSignWorkCasesShowSymbols()
{
    workCasesTest(markdownFunction, MULTI_SIGN_MARKDOWN, MULTI_SIGN_WORK_CASES, true,
                  multiSignWorkCasesProcessInput, multiSignWorkCasesProcessOutput);
}

void TestTextFormatter::commonExceptionsShowSymbols()
{
    exceptionsTest(markdownFunction, ALL_MARKDOWN_TYPES, COMMON_EXCEPTIONS, true);
}

void TestTextFormatter::commonExceptionsHideSymbols()
{
    exceptionsTest(markdownFunction, ALL_MARKDOWN_TYPES, COMMON_EXCEPTIONS, false);
}

void TestTextFormatter::singleSignExceptionsShowSymbols()
{
    exceptionsTest(markdownFunction, SINGLE_SIGN_MARKDOWN, SINGLE_SIGN_EXCEPTIONS, true);
}

void TestTextFormatter::singleSignExceptionsHideSymbols()
{
    exceptionsTest(markdownFunction, SINGLE_SIGN_MARKDOWN, SINGLE_SIGN_EXCEPTIONS, false);
}

void TestTextFormatter::singleAndDoubleMarkdownExceptionsShowSymbols()
{
    exceptionsTest(markdownFunction, SINGLE_AND_DOUBLE_MARKDOWN, SINGLE_AND_DOUBLE_SIGN_EXCEPTIONS,
                   true);
}

void TestTextFormatter::singleAndDoubleMarkdownExceptionsHideSymbols()
{
    exceptionsTest(markdownFunction, SINGLE_AND_DOUBLE_MARKDOWN, SINGLE_AND_DOUBLE_SIGN_EXCEPTIONS,
                   false);
}

void TestTextFormatter::mixedFormattingSpecialCases()
{
    specialCasesTest(markdownFunction, MIXED_FORMATTING_SPECIAL_CASES);
}

void TestTextFormatter::urlTest()
{
    urlHighlightTest(urlHighlightFunction, URL_CASES);
}

void TestTextFormatter::postNullTagsHtml()
{
    postNullCasesTest(POST_NULL_HTML_CASE, true);
}

void TestTextFormatter::postNullTagsPlain()
{
    postNullCasesTest(POST_NULL_PLAIN_CASE, false);
}

QTEST_GUILESS_MAIN(TestTextFormatter)
#include "textformatter_test.moc"
