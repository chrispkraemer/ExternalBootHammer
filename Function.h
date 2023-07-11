#include "Instruction.h"
#include <vector>
#include <string>

using namespace std;

class Function {

public:
    string name;
    vector<Instruction*> body;
    vector<Function*> children;
    //vector<Function*> parents; 
    Function(string name);

};
