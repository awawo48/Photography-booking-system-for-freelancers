#include <iostream>
#include <string>
using namespace std;
int main() {
    try {
        int w = -1;
        string pad(w, ' ');
        cout << "OK" << endl;
    } catch(exception& e) {
        cout << "Error: " << e.what() << endl;
    }
}
