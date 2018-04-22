#include "SimJoiner.h"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <algorithm>
#include <unordered_map>

using namespace std;

SimJoiner::SimJoiner() {
}

SimJoiner::~SimJoiner() {
}

bool cmp(string a, string b) {
	return (a[0] == b[0] ? a[1] < b[1] : a[0] < b[0]); 
}

int get(string a, string b) {
	if (a[0] < b[0]) return 1;
	if (a[0] > b[0]) return -1;
	if (a[1] < b[1]) return 1;
	if (a[1] > b[1]) return -1;
	return 0;
}

unsigned getED(const char* st1, const char* ed1, const char* st2, const char* ed2, unsigned thres);

int loadData(vector <string> &res, const char *filename) {
	res.clear();
	ifstream fin(filename);
	char buf[2050];
	while (fin.getline(buf, 2048)) {
		res.emplace_back(buf);
	}
	return SUCCESS;
}

int SimJoiner::joinJaccard(const char *filename1, const char *filename2, double threshold, vector<JaccardJoinResult> &result) {
    result.clear();
    return SUCCESS;
}

int SimJoiner::joinED(const char *filename1, const char *filename2, unsigned threshold, vector<EDJoinResult> &result) {
    result.clear();
    vector <string> data1, data2;
    loadData(data1, filename1);
    loadData(data2, filename2);

	vector <vector <int>> filter;
	vector <vector <string>> grams;
	vector <unordered_map <string, vector <int>>> preInv;
	int maxlen = 0;
	for (const auto &str : data2) {
		if (str.length() > maxlen) maxlen = str.length();
	}
	filter.resize(maxlen + 1);
	preInv.resize(maxlen + 1);
	grams.resize(data2.size());

	for (int i = 0; i < data2.size(); i++) {
		auto &str = data2[i];
		auto &tmpfilter = filter[str.length()];
		tmpfilter.emplace_back(i);
		for (int j = 0; j + q <= str.length(); j++) {
			grams[i].emplace_back(str.substr(j, q));
		}
		sort(grams[i].begin(), grams[i].end(), cmp);
		for (int j = 0; j < threshold * q + 1 && j < grams[i].size(); j++) {
			preInv[str.length()][grams[i][j]].emplace_back(i);
		}
	}

	for (int i = 0; i <= maxlen; i++) {
		for (auto &tmp : preInv[i]) {
			unordered_map <int, int> unq;
			for (const auto &idx : tmp.second) {
				unq[idx] = 1;
			}
			tmp.second.clear();
			for (const auto &p : unq) {
				tmp.second.emplace_back(p.first);
			}
		}
	}

	vector <int> flag(data2.size(), 0);

	for (int i = 0; i < data1.size(); i++) {
		vector <string> tmpGrams;
		auto &str = data1[i];
		for (int j = 0; j + q <= str.length(); j++) {
			tmpGrams.emplace_back(str.substr(j, q));
		}
		sort(tmpGrams.begin(), tmpGrams.end(), cmp);

		int minl = str.length() - threshold;
		if (minl < 0) {
			minl = 0;
		}
		int maxl = str.length() + threshold;
		if (maxl > maxlen) {
			maxl = maxlen;
		}

		vector <int> candidates1;

		for (int l = minl; l <= maxl; l++) {
			for (const auto &subinv : preInv[l]) {
				for (const auto &idx : subinv.second) {
					if (!flag[idx]) {
						candidates1.emplace_back(idx);
						flag[idx] = 1;
					}
				}
			}
		}

		for (const auto &idx : candidates1) {
			flag[idx] = 0;
		}

		vector <int> candidates2;

		for (const auto &idx : candidates1) {
			int bound = max(grams[idx].size(), tmpGrams.size()) - q * threshold;
			int l = 0, r = 0, s = 0, flag = 0, cnt = 0;
			while (l < tmpGrams.size() && r < grams[idx].size()) {
				s = get(tmpGrams[l], grams[idx][r]);
				if (s == 1) l++;
				if (s == 0) {cnt += 2; l++; r++;}
				if (s == -1) r++;
				if (cnt >= bound) break;
			}
			if (cnt >= bound) candidates2.emplace_back(idx);
		}

/*		vector <pair <unsigned, unsigned>> tmpRes;
		for (const auto &idx : candidates2) {
			unsigned ed = getED(str.c_str(), str.c_str() + str.length(), data2[idx].c_str(), data2[idx].c_str() + data2[idx].length(), threshold);
			
			if (ed <= threshold) {
				tmpRes.emplace_back(idx, ed);
			}
		}
		sort(tmpRes.begin(), tmpRes.end());
		for (const auto &p : tmpRes) {
			result.emplace_back(i, p.first, p.second);
		}*/
	}

/*	for (int i = 0; i <= maxlen; i++) {
		printf("len: %d\n", i);
		for (const auto &tmp : filter[i]) {
			printf("%d ", tmp);
		}
		printf("\n");
		for (const auto &tmp : preInv[i]) {
			printf("%s ", tmp.first.c_str());
			for (const auto &idx : tmp.second) {
				printf("%d ", idx);
			}
			printf("\n");
		}
	}

	for (int i = 0; i < data2.size(); i++) {
		printf("id: %d\n", i);
		for (const auto &j : grams[i]) {
			printf("%s ", j.c_str());
		}
		printf("\n");
	}*/
	
    return SUCCESS;
}

inline void denew(unsigned &x, unsigned y) {
	if (x > y) x = y;
}

unsigned getED(const char* st1, const char* ed1, const char* st2, const char* ed2, unsigned thres) {
	if (ed1 - st1 > ed2 - st2) {
		swap(st1, st2);
		swap(ed1, ed2);
	}
	while (st1 < ed1 && *st1 == *st2) {
		st1++;
		st2++;
	}
	while (st1 < ed1 && *(ed1 - 1) == *(ed2 - 1)) {
		ed1--;
		ed2--;
	}
	if (st1 == ed1) {
		return ed2 - st2;
	}
	unsigned l1 = ed1 - st1, l2 = ed2 - st2;
	if (thres > l2) {
		thres = l2;
	}
	if (l1 + thres < l2) {
		return thres + 1;
	}

	unsigned range = thres << 1 | 1;
	vector <unsigned> d0(range), d1(range);

	for (unsigned j = 0; j <= thres; j++) {
		d0[j + thres] = j;
	}
	for (unsigned i = 1; i <= l1; i++) {
		unsigned lowb = i < thres ? 0 : i - thres;
		unsigned upb = min(l2, i + thres);
		bool f = 0;
		for (unsigned j = lowb, pos = j + thres - i; j <= upb; j++, pos++) {
			d1[pos] = thres + 1;
			if (j > lowb) {
				denew(d1[pos], d1[pos - 1] + 1);
			}
			if (j > 0) {
				denew(d1[pos], d0[pos] + (st1[i - 1] != st2[j - 1]));
			}
			if (j < i + thres) {
				denew(d1[pos], d0[pos + 1] + 1);
			}
			f |= (d1[pos] <= thres);
		}
		if (!f) {
			return thres + 1;
		}
		swap(d0, d1);
	}
	return d0[l2 + thres - l1];
}
