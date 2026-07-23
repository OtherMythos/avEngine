#ifdef SCRIPT_PROFILER

#include "gtest/gtest.h"

#include "Scripting/Profiler/ProfileReport.h"

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

using namespace AV;

namespace{
    ProfileReport::Function makeFunction(uint32_t id, const char* name, double exclusiveMs){
        ProfileReport::Function func;
        func.id = id;
        func.name = name;
        func.source = "/project/scripts/gameplay.nut";
        func.declLine = 42;
        func.callCount = 7;
        func.inclusiveMs = exclusiveMs * 2.0;
        func.exclusiveMs = exclusiveMs;
        func.avgUs = 12.5;
        func.maxUs = 30.0;
        return func;
    }

    ProfileReport makeReport(){
        ProfileReport report;
        report.totals.enabled = true;
        report.totals.running = true;
        report.totals.elapsedMs = 5000.0;
        report.totals.scriptMs = 250.0;
        report.totals.totalCalls = 1234;
        report.totals.framesElapsed = 300;
        report.totals.lineExecutions = 98765;
        report.totals.functionsSeen = 12;
        report.functions.push_back(makeFunction(0, "updateWorld", 40.0));
        report.functions.push_back(makeFunction(1, "drawHud", 10.0));
        return report;
    }

    std::string toJsonString(const ProfileReport& report){
        rapidjson::Document doc;
        ProfileReportWriter::toJson(report, doc);
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        return buffer.GetString();
    }
}

TEST(ProfileReportTests, parseSortAcceptsTheDocumentedNames){
    ProfileQuery::Sort sort = ProfileQuery::Sort::EXCLUSIVE;

    ASSERT_TRUE(ProfileQuery::parseSort("inclusive", sort));
    EXPECT_EQ(sort, ProfileQuery::Sort::INCLUSIVE);
    ASSERT_TRUE(ProfileQuery::parseSort("calls", sort));
    EXPECT_EQ(sort, ProfileQuery::Sort::CALLS);
    ASSERT_TRUE(ProfileQuery::parseSort("avg", sort));
    EXPECT_EQ(sort, ProfileQuery::Sort::AVERAGE);
    ASSERT_TRUE(ProfileQuery::parseSort("exclusive", sort));
    EXPECT_EQ(sort, ProfileQuery::Sort::EXCLUSIVE);
}

TEST(ProfileReportTests, parseSortRejectsAnythingElseAndLeavesTheValueAlone){
    ProfileQuery::Sort sort = ProfileQuery::Sort::CALLS;

    EXPECT_FALSE(ProfileQuery::parseSort("wallclock", sort));
    EXPECT_FALSE(ProfileQuery::parseSort("", sort));
    EXPECT_EQ(sort, ProfileQuery::Sort::CALLS);
}

TEST(ProfileReportTests, sortNameRoundTripsThroughParseSort){
    const ProfileQuery::Sort all[] = {
        ProfileQuery::Sort::EXCLUSIVE,
        ProfileQuery::Sort::INCLUSIVE,
        ProfileQuery::Sort::CALLS,
        ProfileQuery::Sort::AVERAGE
    };

    for(ProfileQuery::Sort expected : all){
        ProfileQuery::Sort parsed = ProfileQuery::Sort::EXCLUSIVE;
        ASSERT_TRUE(ProfileQuery::parseSort(ProfileQuery::sortName(expected), parsed));
        EXPECT_EQ(parsed, expected);
    }
}

TEST(ProfileReportTests, findFunctionResolvesByIdAndMissesCleanly){
    const ProfileReport report = makeReport();

    const ProfileReport::Function* found = report.findFunction(1);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->name, "drawHud");
    //An id capped out of the response must not resolve to the wrong entry.
    EXPECT_EQ(report.findFunction(99), nullptr);
}

TEST(ProfileReportTests, textReportNamesTheFlagWhenDisabled){
    ProfileReport report;
    report.totals.enabled = false;

    const std::string text = ProfileReportWriter::toText(report);
    EXPECT_NE(text.find("--profileScripts"), std::string::npos);
}

TEST(ProfileReportTests, textReportListsFunctionsAndTotals){
    const std::string text = ProfileReportWriter::toText(makeReport());

    EXPECT_NE(text.find("updateWorld"), std::string::npos);
    EXPECT_NE(text.find("drawHud"), std::string::npos);
    //The path is reduced to a file name so the column stays readable.
    EXPECT_NE(text.find("gameplay.nut:42"), std::string::npos);
    EXPECT_EQ(text.find("/project/scripts/"), std::string::npos);
    EXPECT_NE(text.find("1234 calls"), std::string::npos);
}

TEST(ProfileReportTests, textReportSaysHowManyFunctionsWereOmitted){
    ProfileReport report = makeReport();
    report.totals.functionsOmitted = 37;

    const std::string text = ProfileReportWriter::toText(report);
    EXPECT_NE(text.find("37 more functions"), std::string::npos);
}

TEST(ProfileReportTests, textReportHandlesAnEmptyRun){
    ProfileReport report;
    report.totals.enabled = true;

    const std::string text = ProfileReportWriter::toText(report);
    EXPECT_NE(text.find("No squirrel functions"), std::string::npos);
}

TEST(ProfileReportTests, jsonCarriesTheFunctionFields){
    rapidjson::Document doc;
    ProfileReportWriter::toJson(makeReport(), doc);

    ASSERT_TRUE(doc.IsObject());
    EXPECT_TRUE(doc["enabled"].GetBool());
    EXPECT_EQ(doc["totalCalls"].GetUint64(), 1234u);
    EXPECT_EQ(doc["lineExecutions"].GetUint64(), 98765u);

    ASSERT_TRUE(doc["functions"].IsArray());
    ASSERT_EQ(doc["functions"].Size(), 2u);
    const rapidjson::Value& first = doc["functions"][0];
    EXPECT_STREQ(first["name"].GetString(), "updateWorld");
    //The full path is kept in json, unlike the text table.
    EXPECT_STREQ(first["source"].GetString(), "/project/scripts/gameplay.nut");
    EXPECT_EQ(first["line"].GetInt(), 42);
    EXPECT_EQ(first["calls"].GetUint64(), 7u);
    EXPECT_DOUBLE_EQ(first["exclusiveMs"].GetDouble(), 40.0);
}

TEST(ProfileReportTests, jsonOmitsSectionsThatWereNotCollected){
    const std::string json = toJsonString(makeReport());

    //Bulk sections are opt-in, and an empty array would still cost a caller context.
    EXPECT_EQ(json.find("\"edges\""), std::string::npos);
    EXPECT_EQ(json.find("\"hotLines\""), std::string::npos);
    EXPECT_EQ(json.find("\"frames\""), std::string::npos);
}

TEST(ProfileReportTests, jsonReportsEngineEnteredCallsAsSuch){
    ProfileReport report = makeReport();
    ProfileReport::Edge fromEngine;
    fromEngine.callerId = ProfileReport::ROOT_ID;
    fromEngine.calleeId = 0;
    fromEngine.callCount = 300;
    fromEngine.inclusiveMs = 80.0;
    report.edges.push_back(fromEngine);

    rapidjson::Document doc;
    ProfileReportWriter::toJson(report, doc);

    ASSERT_TRUE(doc["edges"].IsArray());
    //A caller id of ROOT_ID is not a real function, so it is named rather than numbered.
    EXPECT_STREQ(doc["edges"][0]["caller"].GetString(), "engine");
    EXPECT_EQ(doc["edges"][0]["callee"].GetUint(), 0u);
}

TEST(ProfileReportTests, hotLinesCarryTheirOwnFunctionName){
    ProfileReport report = makeReport();
    ProfileReport::Line line;
    line.functionId = 0;
    line.functionName = "updateWorld";
    line.source = "/project/scripts/gameplay.nut";
    line.line = 88;
    line.executions = 5000;
    line.selfMs = 12.5;
    line.pctSelf = 60.0;
    report.hotLines.push_back(line);

    rapidjson::Document doc;
    ProfileReportWriter::toJson(report, doc);

    ASSERT_TRUE(doc["hotLines"].IsArray());
    const rapidjson::Value& entry = doc["hotLines"][0];
    //Readable without cross referencing the function table, which the endpoints cap.
    EXPECT_STREQ(entry["name"].GetString(), "updateWorld");
    EXPECT_EQ(entry["line"].GetInt(), 88);
    EXPECT_EQ(entry["executions"].GetUint64(), 5000u);

    const std::string text = ProfileReportWriter::toText(report);
    EXPECT_NE(text.find("gameplay.nut:88"), std::string::npos);
}

#endif
