#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <iomanip>
#include <algorithm>

using namespace std;

struct Participant {
    string nickname;
    double spent = 0.0;
    double share_of_total = 0.0;
};

string clean(const string& s) {
    size_t start = s.find_first_not_of(" \r\n\t");
    if (start == string::npos) return "";
    size_t finish = s.find_last_not_of(" \r\n\t");
    return s.substr(start, finish - start + 1);
}

int main() {
    ifstream data("траты.txt");
    if (!data.is_open()) {
        cerr << "Ошибка. Не удалось открыть файл" << endl;
        return 1;
    }

    string header;
    getline(data, header);
    stringstream headerStream(header);
    int peopleCount;
    headerStream >> peopleCount;

    vector<Participant> participants(peopleCount);
    map<string, int> idToIndex;

    for (int i = 0; i < peopleCount; ++i) {
        headerStream >> participants[i].nickname;
        idToIndex[participants[i].nickname] = i;
    }

    string raw;
    while (getline(data, raw)) {
        raw = clean(raw);
        if (raw.empty()) continue;

        size_t slashPos = raw.find('/');
        string leftSide = raw.substr(0, slashPos);
        stringstream leftParser(leftSide);
        string buyer;
        double cost;
        if (!(leftParser >> buyer >> cost)) continue;

        if (idToIndex.count(buyer)) {
            participants[idToIndex[buyer]].spent += cost;
        }

        set<string> excludedSet;
        if (slashPos != string::npos) {
            string rightSide = raw.substr(slashPos + 1);
            stringstream rightParser(rightSide);
            string excludedName;
            while (getline(rightParser, excludedName, ',')) {
                excludedName = clean(excludedName);
                if (!excludedName.empty()) excludedSet.insert(excludedName);
            }
        }

        vector<int> includedIndices;
        for (int i = 0; i < peopleCount; ++i) {
            if (!excludedSet.count(participants[i].nickname)) {
                includedIndices.push_back(i);
            }
        }

        if (!includedIndices.empty()) {
            double portion = cost / includedIndices.size();
            for (int idx : includedIndices) {
                participants[idx].share_of_total += portion;
            }
        }
    }
    data.close();

    cout << fixed << setprecision(1);
    for (const auto& p : participants) {
        cout << p.nickname << " " << p.spent << " " << p.share_of_total << endl;
    }

    vector<pair<string, double>> debtors, creditors;
    for (const auto& p : participants) {
        double diff = p.spent - p.share_of_total;
        if (diff < -0.001)
            debtors.push_back({ p.nickname, -diff });
        else if (diff > 0.001)
            creditors.push_back({ p.nickname, diff });
    }

    sort(debtors.begin(), debtors.end());
    sort(creditors.begin(), creditors.end());

    size_t d = 0, c = 0;
    while (d < debtors.size() && c < creditors.size()) {
        double amount = min(debtors[d].second, creditors[c].second);
        if (amount > 0.001) {
            cout << debtors[d].first << " "
                << amount << " "
                << creditors[c].first << endl;
        }
        debtors[d].second -= amount;
        creditors[c].second -= amount;
        if (debtors[d].second < 0.001) d++;
        if (creditors[c].second < 0.001) c++;
    }

    return 0;
}