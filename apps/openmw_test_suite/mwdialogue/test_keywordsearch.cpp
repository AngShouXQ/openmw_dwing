#include <gtest/gtest.h>
#include "apps/openmw/mwdialogue/keywordsearch.hpp"

struct KeywordSearchTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(KeywordSearchTest, keyword_test_conflict_resolution)
{
    // test to make sure the longest keyword in a chain of conflicting keywords gets chosen
    MWDialogue::KeywordSearch<std::string, int> search;
    search.seed("foo bar", 0);
    search.seed("bar lock", 0);
    search.seed("lock switch", 0);

    std::string text = "foo bar lock switch";

    std::vector<MWDialogue::KeywordSearch<std::string, int>::Match> matches;
    search.highlightKeywords(text.begin(), text.end(), matches);

    // Should contain: "foo bar", "lock switch"
    ASSERT_TRUE (matches.size() == 2);
    ASSERT_TRUE (std::string(matches.front().mBeg, matches.front().mEnd) == "foo bar");
    ASSERT_TRUE (std::string(matches.rbegin()->mBeg, matches.rbegin()->mEnd) == "lock switch");
}

TEST_F(KeywordSearchTest, keyword_test_conflict_resolution2)
{
    MWDialogue::KeywordSearch<std::string, int> search;
    search.seed("the dwemer", 0);
    search.seed("dwemer language", 0);

    std::string text = "the dwemer language";

    std::vector<MWDialogue::KeywordSearch<std::string, int>::Match> matches;
    search.highlightKeywords(text.begin(), text.end(), matches);

    ASSERT_TRUE (matches.size() == 1);
    ASSERT_TRUE (std::string(matches.front().mBeg, matches.front().mEnd) == "dwemer language");
}


TEST_F(KeywordSearchTest, keyword_test_conflict_resolution3)
{
    // testing that the longest keyword is chosen, rather than maximizing the
    // amount of highlighted characters by highlighting the first and last keyword
    MWDialogue::KeywordSearch<std::string, int> search;
    search.seed("foo bar", 0);
    search.seed("bar lock", 0);
    search.seed("lock so", 0);

    std::string text = "foo bar lock so";

    std::vector<MWDialogue::KeywordSearch<std::string, int>::Match> matches;
    search.highlightKeywords(text.begin(), text.end(), matches);

    ASSERT_TRUE (matches.size() == 1);
    ASSERT_TRUE (std::string(matches.front().mBeg, matches.front().mEnd) == "bar lock");
}


TEST_F(KeywordSearchTest, keyword_test_utf8_word_begin)
{
    // We make sure that the search works well even if the character is not ASCII
    MWDialogue::KeywordSearch<std::string, int> search;
    search.seed("�tats", 0);
    search.seed("�rradi�s", 0);
    search.seed("�a nous d��ois", 0);


    std::string text = "les nations unis ont r�unis le monde entier, �tats units inclus pour parler du probl�me des gens �rradi�s et �a nous d��ois";

    std::vector<MWDialogue::KeywordSearch<std::string, int>::Match> matches;
    search.highlightKeywords(text.begin(), text.end(), matches);

    ASSERT_TRUE (matches.size() == 3);
    ASSERT_TRUE(std::string( matches[0].mBeg, matches[0].mEnd) == "�tats");
    ASSERT_TRUE(std::string( matches[1].mBeg, matches[1].mEnd) == "�rradi�s");
    ASSERT_TRUE(std::string( matches[2].mBeg, matches[2].mEnd) == "�a nous d��ois");

    //ASSERT_TRUE (std::string(matches.front().mBeg, matches.front().mEnd) == "bar lock");
}
