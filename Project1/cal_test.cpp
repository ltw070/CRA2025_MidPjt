#include "gmock/gmock.h"
#include "cal.cpp"

using namespace testing;

class MockCal : public Cal {
	public:
	MOCK_METHOD(int, getSum, (int a, int b), ());
};

TEST(Group, TC1) {
	Cal cal;

	int ret = cal.getSum(10, 20);

	EXPECT_EQ(30, ret);
}

TEST(Group, TC2) {
	MockCal mock;

	EXPECT_CALL(mock, getSum(1, 2))
		.WillRepeatedly(Return(999));

	int ret = mock.getSum(1, 2);
	EXPECT_EQ(999, ret);
}