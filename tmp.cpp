#include <iostream>
#include <cstdio>
#include <string>
using namespace std;

int main(void)
{
    freopen("tmp.txt", "r", stdin);
    string str;
    cin >> str;
    cout << str << endl;
    getline(cin, str);
    getline(cin, str);
    cout << str << endl;
    cin >> str;
    cout << str << endl;
    getline(cin, str);
    getline(cin, str);
    cout << str << endl;
    getline(cin, str);
    cout << str << endl;
    cin >> str;
    cout << str << endl;

    getline(cin, str);
    getline(cin, str);
    cout << str << endl;
    return 0;
}