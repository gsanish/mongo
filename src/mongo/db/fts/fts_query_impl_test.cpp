// fts_query_impl_test.cpp

/**
*    Copyright (C) 2012 10gen Inc.
*
*    This program is free software: you can redistribute it and/or  modify
*    it under the terms of the GNU Affero General Public License, version 3,
*    as published by the Free Software Foundation.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU Affero General Public License for more details.
*
*    You should have received a copy of the GNU Affero General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*    As a special exception, the copyright holders give permission to link the
*    code of portions of this program with the OpenSSL library under certain
*    conditions as described in each individual source file and distribute
*    linked combinations including the program with the OpenSSL library. You
*    must comply with the GNU Affero General Public License in all respects for
*    all of the code used other than as permitted herein. If you modify file(s)
*    with this exception, you may extend this exception to your version of the
*    file(s), but you are not obligated to do so. If you do not wish to do so,
*    delete this exception statement from your version. If you delete this
*    exception statement from all source files in the program, then also delete
*    it in the license file.
*/


#include "mongo/db/fts/fts_query_impl.h"
#include "mongo/unittest/unittest.h"

namespace mongo {
namespace fts {

TEST(FTSQueryImpl, Basic1) {
    FTSQueryImpl q;
    ASSERT(q.parse("this is fun", "english", false, false, TEXT_INDEX_VERSION_3).isOK());

    ASSERT_EQUALS(false, q.getCaseSensitive());
    ASSERT_EQUALS(1U, q.getPositiveTerms().size());
    ASSERT_EQUALS("fun", *q.getPositiveTerms().begin());
    ASSERT_EQUALS(0U, q.getNegatedTerms().size());
    ASSERT_EQUALS(0U, q.getPositivePhr().size());
    ASSERT_EQUALS(0U, q.getNegatedPhr().size());
    ASSERT_TRUE(q.getTermsForBounds() == q.getPositiveTerms());
}

TEST(FTSQueryImpl, ParsePunctuation) {
    FTSQueryImpl q;
    ASSERT(q.parse("hello.world", "english", false, false, TEXT_INDEX_VERSION_3).isOK());

    ASSERT_EQUALS(false, q.getCaseSensitive());
    ASSERT_EQUALS(2U, q.getPositiveTerms().size());
    ASSERT_EQUALS("hello", *q.getPositiveTerms().begin());
    ASSERT_EQUALS("world", *(--q.getPositiveTerms().end()));
    ASSERT_EQUALS(0U, q.getNegatedTerms().size());
    ASSERT_EQUALS(0U, q.getPositivePhr().size());
    ASSERT_EQUALS(0U, q.getNegatedPhr().size());
    ASSERT_TRUE(q.getTermsForBounds() == q.getPositiveTerms());
}

TEST(FTSQueryImpl, Neg1) {
    FTSQueryImpl q;
    ASSERT(q.parse("this is -really fun", "english", false, false, TEXT_INDEX_VERSION_3).isOK());

    ASSERT_EQUALS(1U, q.getPositiveTerms().size());
    ASSERT_EQUALS("fun", *q.getPositiveTerms().begin());
    ASSERT_EQUALS(1U, q.getNegatedTerms().size());
    ASSERT_EQUALS("realli", *q.getNegatedTerms().begin());
    ASSERT_TRUE(q.getTermsForBounds() == q.getPositiveTerms());
}

TEST(FTSQueryImpl, Phrase1) {
    FTSQueryImpl q;
    ASSERT(q.parse("doing a \"phrase test\" for fun", "english", false, false, TEXT_INDEX_VERSION_3)
               .isOK());

    ASSERT_EQUALS(3U, q.getPositiveTerms().size());
    ASSERT_EQUALS(0U, q.getNegatedTerms().size());
    ASSERT_EQUALS(1U, q.getPositivePhr().size());
    ASSERT_EQUALS(0U, q.getNegatedPhr().size());
    ASSERT_TRUE(q.getTermsForBounds() == q.getPositiveTerms());

    ASSERT_EQUALS("phrase test", q.getPositivePhr()[0]);
    ASSERT_EQUALS("fun|phrase|test||||phrase test||", q.debugString());
}

TEST(FTSQueryImpl, Phrase2) {
    FTSQueryImpl q;
    ASSERT(q.parse("doing a \"phrase-test\" for fun", "english", false, false, TEXT_INDEX_VERSION_3)
               .isOK());
    ASSERT_EQUALS(1U, q.getPositivePhr().size());
    ASSERT_EQUALS("phrase-test", q.getPositivePhr()[0]);
}

TEST(FTSQueryImpl, NegPhrase1) {
    FTSQueryImpl q;
    ASSERT(
        q.parse("doing a -\"phrase test\" for fun", "english", false, false, TEXT_INDEX_VERSION_3)
            .isOK());
    ASSERT_EQUALS("fun||||||phrase test", q.debugString());
}

TEST(FTSQueryImpl, CaseSensitiveOption) {
    FTSQueryImpl q;
    ASSERT(q.parse("this is fun", "english", true, false, TEXT_INDEX_VERSION_3).isOK());
    ASSERT_EQUALS(true, q.getCaseSensitive());
}

TEST(FTSQueryImpl, CaseSensitivePositiveTerms) {
    FTSQueryImpl q;
    ASSERT(q.parse("This is Positively fun", "english", true, false, TEXT_INDEX_VERSION_3).isOK());

    ASSERT_EQUALS(2U, q.getTermsForBounds().size());
    ASSERT_EQUALS(1,
                  std::count(q.getTermsForBounds().begin(), q.getTermsForBounds().end(), "posit"));
    ASSERT_EQUALS(1, std::count(q.getTermsForBounds().begin(), q.getTermsForBounds().end(), "fun"));
    ASSERT_EQUALS(2U, q.getPositiveTerms().size());
    ASSERT_EQUALS(1, std::count(q.getPositiveTerms().begin(), q.getPositiveTerms().end(), "Posit"));
    ASSERT_EQUALS(1, std::count(q.getPositiveTerms().begin(), q.getPositiveTerms().end(), "fun"));
    ASSERT_EQUALS(0U, q.getNegatedTerms().size());
    ASSERT_EQUALS(0U, q.getPositivePhr().size());
    ASSERT_EQUALS(0U, q.getNegatedPhr().size());
}

TEST(FTSQueryImpl, CaseSensitiveNegativeTerms) {
    FTSQueryImpl q;
    ASSERT(q.parse("-This -is -Negatively -miserable", "english", true, false, TEXT_INDEX_VERSION_3)
               .isOK());

    ASSERT_EQUALS(0U, q.getPositiveTerms().size());
    ASSERT_EQUALS(0U, q.getTermsForBounds().size());
    ASSERT_EQUALS(2U, q.getNegatedTerms().size());
    ASSERT_EQUALS(1, std::count(q.getNegatedTerms().begin(), q.getNegatedTerms().end(), "Negat"));
    ASSERT_EQUALS(1, std::count(q.getNegatedTerms().begin(), q.getNegatedTerms().end(), "miser"));
    ASSERT_EQUALS(0U, q.getPositivePhr().size());
    ASSERT_EQUALS(0U, q.getNegatedPhr().size());
}

TEST(FTSQueryImpl, CaseSensitivePositivePhrases) {
    FTSQueryImpl q;
    ASSERT(q.parse("doing a \"Phrase Test\" for fun", "english", true, false, TEXT_INDEX_VERSION_3)
               .isOK());

    ASSERT_EQUALS(1U, q.getPositivePhr().size());
    ASSERT_EQUALS(0U, q.getNegatedPhr().size());
    ASSERT_EQUALS("Phrase Test", q.getPositivePhr()[0]);
}

TEST(FTSQueryImpl, CaseSensitiveNegativePhrases) {
    FTSQueryImpl q;
    ASSERT(q.parse("doing a -\"Phrase Test\" for fun", "english", true, false, TEXT_INDEX_VERSION_3)
               .isOK());

    ASSERT_EQUALS(0U, q.getPositivePhr().size());
    ASSERT_EQUALS(1U, q.getNegatedPhr().size());
    ASSERT_EQUALS("Phrase Test", q.getNegatedPhr()[0]);
}

TEST(FTSQueryImpl, Mix1) {
    FTSQueryImpl q;
    ASSERT(
        q.parse("\"industry\" -Melbourne -Physics", "english", false, false, TEXT_INDEX_VERSION_3)
            .isOK());
    ASSERT_EQUALS("industri||melbourn|physic||industry||", q.debugString());
}

TEST(FTSQueryImpl, NegPhrase2) {
    FTSQueryImpl q1, q2, q3;
    ASSERT(q1.parse("foo \"bar\"", "english", false, false, TEXT_INDEX_VERSION_3).isOK());
    ASSERT(q2.parse("foo \"-bar\"", "english", false, false, TEXT_INDEX_VERSION_3).isOK());
    ASSERT(q3.parse("foo \" -bar\"", "english", false, false, TEXT_INDEX_VERSION_3).isOK());

    ASSERT_EQUALS(2U, q1.getPositiveTerms().size());
    ASSERT_EQUALS(2U, q2.getPositiveTerms().size());
    ASSERT_EQUALS(2U, q3.getPositiveTerms().size());

    ASSERT_EQUALS(0U, q1.getNegatedTerms().size());
    ASSERT_EQUALS(0U, q2.getNegatedTerms().size());
    ASSERT_EQUALS(0U, q3.getNegatedTerms().size());

    ASSERT_EQUALS(1U, q1.getPositivePhr().size());
    ASSERT_EQUALS(1U, q2.getPositivePhr().size());
    ASSERT_EQUALS(1U, q3.getPositivePhr().size());

    ASSERT_EQUALS(0U, q1.getNegatedPhr().size());
    ASSERT_EQUALS(0U, q2.getNegatedPhr().size());
    ASSERT_EQUALS(0U, q3.getNegatedPhr().size());
}

TEST(FTSQueryImpl, NegPhrase3) {
    FTSQueryImpl q1, q2, q3;
    ASSERT(q1.parse("foo -\"bar\"", "english", false, false, TEXT_INDEX_VERSION_3).isOK());
    ASSERT(q2.parse("foo -\"-bar\"", "english", false, false, TEXT_INDEX_VERSION_3).isOK());
    ASSERT(q3.parse("foo -\" -bar\"", "english", false, false, TEXT_INDEX_VERSION_3).isOK());

    ASSERT_EQUALS(1U, q1.getPositiveTerms().size());
    ASSERT_EQUALS(1U, q2.getPositiveTerms().size());
    ASSERT_EQUALS(1U, q3.getPositiveTerms().size());

    ASSERT_EQUALS(0U, q1.getNegatedTerms().size());
    ASSERT_EQUALS(0U, q2.getNegatedTerms().size());
    ASSERT_EQUALS(0U, q3.getNegatedTerms().size());

    ASSERT_EQUALS(0U, q1.getPositivePhr().size());
    ASSERT_EQUALS(0U, q2.getPositivePhr().size());
    ASSERT_EQUALS(0U, q3.getPositivePhr().size());

    ASSERT_EQUALS(1U, q1.getNegatedPhr().size());
    ASSERT_EQUALS(1U, q2.getNegatedPhr().size());
    ASSERT_EQUALS(1U, q3.getNegatedPhr().size());
}

// Test textIndexVersion:1 query with language "english".  This invokes the standard English
// stemmer and stopword list.
TEST(FTSQueryImpl, TextIndexVersion1LanguageEnglish) {
    FTSQueryImpl q;
    ASSERT(q.parse("the running", "english", false, false, TEXT_INDEX_VERSION_1).isOK());
    ASSERT_EQUALS(1U, q.getPositiveTerms().size());
    ASSERT_EQUALS("run", *q.getPositiveTerms().begin());
    ASSERT_EQUALS(0U, q.getNegatedTerms().size());
    ASSERT_EQUALS(0U, q.getPositivePhr().size());
    ASSERT_EQUALS(0U, q.getNegatedPhr().size());
}

// Test textIndexVersion:1 query with language "eng".  "eng" uses the English stemmer, and
// no stopword list.
TEST(FTSQueryImpl, TextIndexVersion1LanguageEng) {
    FTSQueryImpl q;
    ASSERT(q.parse("the running", "eng", false, false, TEXT_INDEX_VERSION_1).isOK());
    ASSERT_EQUALS(2U, q.getPositiveTerms().size());
    ASSERT_EQUALS(1, std::count(q.getPositiveTerms().begin(), q.getPositiveTerms().end(), "the"));
    ASSERT_EQUALS(1, std::count(q.getPositiveTerms().begin(), q.getPositiveTerms().end(), "run"));
    ASSERT_EQUALS(0U, q.getNegatedTerms().size());
    ASSERT_EQUALS(0U, q.getPositivePhr().size());
    ASSERT_EQUALS(0U, q.getNegatedPhr().size());
}

// Test textIndexVersion:1 query with language "invalid".  No stemming will be performed,
// and no stopword list will be used.
TEST(FTSQueryImpl, TextIndexVersion1LanguageInvalid) {
    FTSQueryImpl q;
    ASSERT(q.parse("the running", "invalid", false, false, TEXT_INDEX_VERSION_1).isOK());
    ASSERT_EQUALS(2U, q.getPositiveTerms().size());
    ASSERT_EQUALS(1, std::count(q.getPositiveTerms().begin(), q.getPositiveTerms().end(), "the"));
    ASSERT_EQUALS(1,
                  std::count(q.getPositiveTerms().begin(), q.getPositiveTerms().end(), "running"));
    ASSERT_EQUALS(0U, q.getNegatedTerms().size());
    ASSERT_EQUALS(0U, q.getPositivePhr().size());
    ASSERT_EQUALS(0U, q.getNegatedPhr().size());
}
}
}