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

vector<Node2> DayBest[7]; //�� ~ �Ͽ���
vector<Node2> twoBest[2]; //����, �ָ�
int UZ = 9;

// ������Ÿ�� �Ÿ� ��� �˰��� (���ڿ� ���絵 �˻�)
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

// ���� ���
int calcSimiler(const std::string& string1, const std::string& string2)
{
	if (string1.empty() && string2.empty()) return 100;
	if (string1.empty() || string2.empty()) return 1;

	int dist = levenshtein(string1, string2);
	int max_len = std::max(string1.length(), string2.length());
	// ���絵 ���� (1.0: ������ ����, 0.0: ���� �ٸ�)
	double similarity = 1.0 - (double)dist / max_len;

	int score = 1 + static_cast<int>(similarity * 99);

	return score;
}

// ���� ȯ��
bool similer(const std::string& string1, const std::string& string2) {
	int score = calcSimiler(string1, string2);
	return (score >= 80);
}

// ���� -> �ε��� ����
map<string, int> dayToIndex = {
	{"monday", 0}, {"tuesday", 1}, {"wednesday", 2},
	{"thursday", 3}, {"friday", 4}, {"saturday", 5}, {"sunday", 6}
};

void resetScore();
int increaseNodePoint(int point);

string updateKeywordBy(string keyword, string day) {
	UZ++;

	int day_index = dayToIndex[day];
	int weekend_index = (day_index >= dayToIndex["monday"] && day_index <= dayToIndex["friday"]) ? 0 : 1;

	int point = UZ;

	//���� ��Ͽ� �����ϴ��� Ȯ��
	//�����Ǵ� Ű�����̸� ������ ����

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

	//������ �۾�
	if (UZ >= 2100000000 || maxPoint_day >= 2100000000 || maxPoint_weekdayOrWeekend >= 2100000000) {
		resetScore();
	}

	//�Ϻ� HIT
	if (isPerfectHit) {
		return keyword;
	}

	//���� HIT
	for (Node2& node : DayBest[day_index]) {
		if (similer(node.name, keyword)) {
			return node.name;
		}
	}

	for (Node2& node : twoBest[day_index]) {
		if (similer(node.name, keyword)) {
			return node.name;
		}
	}

	//�Ϻ� HIT / ���� HIT �Ѵ� �ƴѰ��
	if (DayBest[day_index].size() < 10) {
		DayBest[day_index].push_back({ keyword, point });
		std::sort(DayBest[day_index].begin(), DayBest[day_index].end());
	}

	if (twoBest[day_index].size() < 10) {
		twoBest[day_index].push_back({ keyword, point });
		std::sort(twoBest[day_index].begin(), twoBest[day_index].end());
	}

	if (DayBest[day_index].size() == 10) {
		if (DayBest[day_index].back().point < point) {
			DayBest[day_index].pop_back();
			DayBest[day_index].push_back({ keyword, point });
			std::sort(DayBest[day_index].begin(), DayBest[day_index].end());
		}
	}

	if (twoBest[day_index].size() == 10) {
		if (twoBest[day_index].back().point < point) {
			twoBest[day_index].pop_back();
			twoBest[day_index].push_back({ keyword, point });
			std::sort(twoBest[day_index].begin(), twoBest[day_index].end());
		}
	}

	return keyword;
}

void findBetterKeywords() {
	ifstream fin{ "keyword_weekday_500.txt" }; //500�� ������ �Է�
	for (int i = 0; i < 500; i++) {
		string keyword, day;
		fin >> keyword >> day;
		string ret = updateKeywordBy(keyword, day);
		std::cout << ret << "\n";
	}
}

int main() {

	findBetterKeywords();

}

// ���ȭ
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


