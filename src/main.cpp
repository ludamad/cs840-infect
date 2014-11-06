#include <cstdio>
#include <string>
#include <vector>

using namespace std;

typedef int entity_id;

struct Entity {
    vector<entity_id> entities;
};

struct Network {
    vector<Entity> entities;
    Network(int size) : entities(size) {
    }
};

int main(int argn, char** args) {
    string graph_type = "uniform";
    string method = "bad";
    int size = 10;
    return 0;
}
