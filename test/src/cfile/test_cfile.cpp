
#include <gtest/gtest.h>

#include <cfile/cfile.h>

#include "util/FSTestFixture.h"

class CFileTest : public test::FSTestFixture {
 public:
	CFileTest() : test::FSTestFixture(0) {
		pushModDir("cfile");
	}
};

TEST_F(CFileTest, temp_subdir) {
	ASSERT_FALSE(cf_exists_full("test_001.dds", CF_TYPE_ANY));

	ASSERT_TRUE(cf_set_temp_subdir_pathtype("test.eff"));

	ASSERT_TRUE(cf_exists_full("test_001.dds", CF_TYPE_ANY));
}

TEST_F(CFileTest, temp_subdir_vps) {
	ASSERT_FALSE(cf_exists_full("test_001.dds", CF_TYPE_ANY));

	ASSERT_TRUE(cf_set_temp_subdir_pathtype("test.eff"));

	ASSERT_TRUE(cf_exists_full("test_001.dds", CF_TYPE_ANY));
}
