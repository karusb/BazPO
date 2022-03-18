
#include "gtest/gtest.h"
#include "../ProgramOptions/BazPO.hpp"

TEST(ProgramOptionsTest, MultiTest) {
    int argc = 7;
    char* argv[] = { "programoptions","-d","-a","Aoption", "--bravo" , "Boption" , "-c" };
    BazPO po(argc, argv);
    po.Add("-a", "--alpha", "Option A");
    po.Add("-b", "--bravo", "Option B");
    po.Add("-c", "--charlie", "Option C");
    po.Add("-d", "--delta", "Option D");
    po.PrintOptions();
    po.ParseArguments();
    auto aoption = po.GetOption("-a");
    auto boption = po.GetOption("-b");
    auto coption = po.GetOption("-c");
    auto doption = po.GetOption("-d");

  EXPECT_EQ(std::string("Aoption"), std::string(aoption.Value));
  EXPECT_EQ(std::string("Aoption"), po.GetValue("-a"));
  EXPECT_EQ(std::string("Aoption"), po.GetValue("--alpha"));

  EXPECT_EQ(std::string("Boption"), std::string(boption.Value));
  EXPECT_EQ(std::string("Boption"), po.GetValue("-b"));
  EXPECT_EQ(std::string("Boption"), po.GetValue("--bravo"));


  EXPECT_EQ(std::string(""), std::string(coption.Value));
  EXPECT_EQ(std::string(""), po.GetValue("-c"));
  EXPECT_EQ(std::string(""), po.GetValue("--charlie"));

  EXPECT_EQ(std::string(""), std::string(doption.Value));
  EXPECT_EQ(std::string(""), po.GetValue("-d"));
  EXPECT_EQ(std::string(""), po.GetValue("--delta"));


  EXPECT_TRUE(aoption.Exists);
  EXPECT_TRUE(boption.Exists);
  EXPECT_TRUE(coption.Exists);
  EXPECT_TRUE(doption.Exists);

  EXPECT_TRUE(po.Exists("-a"));
  EXPECT_TRUE(po.Exists("-b"));
  EXPECT_TRUE(po.Exists("-c"));
  EXPECT_TRUE(po.Exists("-d"));

  EXPECT_TRUE(po.Exists("--alpha"));
  EXPECT_TRUE(po.Exists("--bravo"));
  EXPECT_TRUE(po.Exists("--charlie"));
  EXPECT_TRUE(po.Exists("--delta"));
}