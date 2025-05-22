#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "gmock/gmock.h"

using namespace std;

struct Node {
	string w;
	string wk;
};

struct Node2 {
	string name;
	int point;

	bool operator<(const Node2& other) const {
		return point < other.point;
	}
};

vector<Node2> DayBest[7]; //월 ~ 일요일
vector<Node2> twoBest[2]; //평일, 주말
int UZ = 9;

// 유사도 점수 계산을 위한 추상 클래스
class ISimilarityScorer {
public:
	virtual ~ISimilarityScorer() {}
	virtual int score(const std::string& a, const std::string& b) const = 0;
};

// 레벤슈타인 거리 계산 알고리즘 (문자열 유사도 검사)
int levenshtein(const std::string& a, const std::string& b) {
	const size_t len_a = a.size();
	const size_t len_b = b.size();

	std::vector<std::vector<int>> d(len_a + 1, std::vector<int>(len_b + 1));

	for (size_t i = 0; i <= len_a; ++i) d[i][0] = i;
	for (size_t j = 0; j <= len_b; ++j) d[0][j] = j;

	for (size_t i = 1; i <= len_a; ++i) {
		for (size_t j = 1; j <= len_b; ++j) {
			if (a[i - 1] == b[j - 1])
				d[i][j] = d[i - 1][j - 1];
			else
				d[i][j] = 1 + std::min({ d[i - 1][j], d[i][j - 1], d[i - 1][j - 1] });
		}
	}
	return d[len_a][len_b];
}

// 레벤슈타인 기반 점수 계산 구현
class LevenshteinScorer : public ISimilarityScorer {
public:
	int score(const std::string& string1, const std::string& string2) const override {
		if (string1.empty() && string2.empty()) return 100;
		if (string1.empty() || string2.empty()) return 1;

		int dist = levenshtein(string1, string2);
		int max_len = std::max(string1.length(), string2.length());
		double similarity = 1.0 - (double)dist / max_len;
		int score = 1 + static_cast<int>(similarity * 99);
		return score;
	}
};

// scorer 객체를 생성
LevenshteinScorer g_scorer;

// 문자열 유사도 계산 함수 포인터 타입 정의
using SimilarityFunc = int(*)(const std::string&, const std::string&);

// 점수 계산
int calcSimiler(const std::string& string1, const std::string& string2)
{
	if (string1.empty() && string2.empty()) return 100;
	if (string1.empty() || string2.empty()) return 1;

	int dist = levenshtein(string1, string2);
	int max_len = std::max(string1.length(), string2.length());
	// 유사도 비율 (1.0: 완전히 같음, 0.0: 전혀 다름)
	double similarity = 1.0 - (double)dist / max_len;

	int score = 1 + static_cast<int>(similarity * 99);

	return score;
}

// 추상화된 함수 포인터 선언 및 초기화
SimilarityFunc similarityFunc = calcSimiler;

//// 점수 환산
//bool similer(const std::string& string1, const std::string& string2) {
//	int score = similarityFunc(string1, string2);
//	return (score >= 80);
//}

// 점수 환산
bool similer(const std::string& string1, const std::string& string2) {
	int score = g_scorer.score(string1, string2);
	return (score >= 80);
}

//bool similer(const std::string& string1, const std::string& string2) {
//	int score = calcSimiler(string1, string2);
//	return (score >= 80);
//}

// 요일 -> 인덱스 맵핑
map<string, int> dayToIndex = {
	{"monday", 0}, {"tuesday", 1}, {"wednesday", 2},
	{"thursday", 3}, {"friday", 4}, {"saturday", 5}, {"sunday", 6}
};

void resetScore();
int increaseNodePoint(int point);

void addKeyword(std::vector<Node2>& node, std::string& keyword, int point);

string updateKeywordBy(string keyword, string day) {
	UZ++;

	int day_index = dayToIndex[day];
	int weekend_index = (day_index >= dayToIndex["monday"] && day_index <= dayToIndex["friday"]) ? 0 : 1;

	int point = UZ;

	//관리 목록에 존재하는지 확인
	//관리되는 키워드이면 점수가 증가

	long long int maxPoint_day = 0;
	long long int maxPoint_weekdayOrWeekend = 0;

	bool isPerfectHit = false;
	for (Node2& node : DayBest[day_index]) {
		if (node.name == keyword) {
			maxPoint_day = node.point = increaseNodePoint(node.point);
			isPerfectHit = true;
			break;
		}
	}

	for (Node2& node : twoBest[weekend_index]) {
		if (node.name == keyword) {
			maxPoint_weekdayOrWeekend = node.point = increaseNodePoint(node.point);
			break;
		}
	}

	//재정렬 작업
	if (UZ >= 2100000000 || maxPoint_day >= 2100000000 || maxPoint_weekdayOrWeekend >= 2100000000) {
		resetScore();
	}

	//완벽 HIT
	if (isPerfectHit) {
		return keyword;
	}

	//찰떡 HIT
	for (Node2& node : DayBest[day_index]) {
		if (similer(node.name, keyword)) {
			return node.name;
		}
	}

	for (Node2& node : twoBest[weekend_index]) {
		if (similer(node.name, keyword)) {
			return node.name;
		}
	}

	//완벽 HIT / 찰떡 HIT 둘다 아닌경우
	auto& dayNode = DayBest[day_index];
	addKeyword(dayNode, keyword, point);

	auto& twoNode = twoBest[weekend_index];
	addKeyword(twoNode, keyword, point);

	return keyword;
}

void addKeyword(std::vector<Node2>& node, std::string& keyword, int point)
{
	if (node.size() < 10) {
		node.push_back({ keyword, point });
		std::sort(node.begin(), node.end());
	}

	if (node.size() == 10) {
		if (node.back().point < point) {
			node.pop_back();
			node.push_back({ keyword, point });
			std::sort(node.begin(), node.end());
		}
	}
}


void findBetterKeywords() {
	ifstream fin{ "keyword_weekday_500.txt" }; //500개 데이터 입력
	for (int i = 0; i < 500; i++) {
		string keyword, day;
		fin >> keyword >> day;
		string ret = updateKeywordBy(keyword, day);
		std::cout << ret << "\n";
	}
}


// test ///////////////

TEST(KeywordTest, PerfectHit) {
	resetScore();
	string keyword = "banana";
	string day = "tuesday";
	updateKeywordBy(keyword, day); // 최초 추가
	int prevPoint = DayBest[1][0].point;
	string result = updateKeywordBy(keyword, day); // 완벽 HIT
	EXPECT_EQ(result, keyword);
	EXPECT_GT(DayBest[1][0].point, prevPoint); // 점수 증가 확인
}


TEST(KeywordTest, BasicAssertions) {
	// 초기화
	resetScore();
	// 테스트 데이터
	string keyword = "test_keyword";
	string day = "monday";
	// 키워드 업데이트
	string result = updateKeywordBy(keyword, day);
	// 결과 확인
	EXPECT_EQ(result, keyword); // 처음 추가된 키워드가 반환되어야 함
	EXPECT_EQ(DayBest[0][0].name, keyword); // 월요일에 추가된 키워드 확인
	EXPECT_EQ(DayBest[0][0].point, UZ); // 점수 확인
}


TEST(KeywordTest, ScoreReset) {
	resetScore();
	// UZ와 point를 임계값 이상으로 강제 설정
	UZ = 2100000000;
	DayBest[0].push_back({ "reset_test", 2100000000 });
	twoBest[0].push_back({ "reset_test2", 2100000000 });
	// 점수 리셋 트리거
	updateKeywordBy("zzz", "monday");
	// 점수 리셋 후 UZ가 9로 초기화되어야 함
	EXPECT_EQ(UZ, 9);
	// DayBest, twoBest의 point가 1부터 재할당되어야 함
	EXPECT_EQ(DayBest[0][0].point, 1);
	EXPECT_EQ(twoBest[0][0].point, 1);
}

TEST(KeywordTest, EmptyKeyword) {
	resetScore();
	string keyword = "";
	string day = "sunday";
	string result = updateKeywordBy(keyword, day);
	EXPECT_EQ(result, keyword);
	EXPECT_EQ(DayBest[6][0].name, keyword);
}

int main() {

	//findBetterKeywords();
	::testing::InitGoogleMock();
	return RUN_ALL_TESTS();
}

// 모듈화
int increaseNodePoint(int point)
{
	point += static_cast<int>(point * 0.1);
	if (point >= 2100000000) point = 2100000000;
	return point;
}

void resetScore()
{
	UZ = 9;
	for (int i = 0; i < 7; i++) {
		int num = 1;
		for (Node2& node : DayBest[i]) {
			node.point = num++;
		}
	}
	for (int i = 0; i < 2; i++) {
		int num = 1;
		for (Node2& node : twoBest[i]) {
			node.point = num++;
		}
	}
}


