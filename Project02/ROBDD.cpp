#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <utility>
#include <queue>
#include <string>

using namespace std;

struct plaForm{
    int inputAmount = 0;
    int outputAmount = 0;
    vector<char> inputVaribles;
    vector<char> outputVaribles;
    int booleanFunctionAmount = 0;
    //string function;
    //bool functionResult;
    vector<pair<string, bool>> function;
};

typedef struct node{
    string varibleName;
    int thenEdge = -1;
    int elseEdge = -1;
    bool isRedundant = false;
}node;

int main(int argc, char* argv[]){
    //command be like ROBDD   <pla file> <dot file>
    //                argv[0]   argv[1]   argv[2]
    /*
    if(argc != 3){
        cout << "Unknown command!\n";
        return 0;
    }*/
    fstream input, output;
    input.open("test1.pla", ios::in);
    output.open("output.dot", ios::out);
    //Input Process
    //Store pla file
    string prefix;
    struct plaForm plaInput;
    while(input >> prefix){
        //Input varibles
        if(prefix == ".i"){
            input >> plaInput.inputAmount;
        }
        if(prefix == ".o"){
            input >> plaInput.outputAmount;
        }   
        if(prefix == ".lib"){
            if(plaInput.inputAmount <= 0){
                cout << "Input error!\n";
                return 0;
            }
            for(int i = 0; i < plaInput.inputAmount; i++){
                char buffer;
                input >> buffer;
                plaInput.inputVaribles.push_back(buffer);
            }
        }
        if(prefix == ".ob"){
            if(plaInput.outputAmount <= 0){
                cout << "Input error!\n";
                return 0;
            }
            for(int i = 0; i < plaInput.outputAmount; i++){
                char buffer;
                input >> buffer;
                plaInput.outputVaribles.push_back(buffer);
            }
        }
        if(prefix == ".p"){
            input >> plaInput.booleanFunctionAmount;
            if(plaInput.booleanFunctionAmount <= 0){
                cout << "Input error!\n";
                return 0;
            }
            for(int i = 0; i < plaInput.booleanFunctionAmount; i++){
                string functionBuffer;
                bool resultBuffer;
                input >> functionBuffer >> resultBuffer;
                plaInput.function.push_back(make_pair(functionBuffer, resultBuffer));
            }
        }
        //end 
        if(prefix == ".e"){
            break;
        }
    }
    //Turn into truth table
    vector<bool> truthTable;
    //2^n 's possibility
    truthTable.resize(pow(2, plaInput.inputAmount));
    
    //preprocess don't care
    //buffer
    vector<pair<string, bool>> functionBuffer;
    //processer
    queue<pair<string, bool>> processer;
    for(int i = 0; i < plaInput.function.size(); i++){
        processer.push(plaInput.function[i]);
    }
    //process until there's no don't care
    while(!processer.empty()){
        bool isDontCareExist = false;
        string falseFunction, trueFunction;

        for(int i = 0; i < processer.front().first.size(); i++){
            //travel each function for don't care
            if(processer.front().first[i] == '-'){
                isDontCareExist = true;
                //turn don't care into 0/1 then process again
                falseFunction = processer.front().first;
                falseFunction[i] = '0';
                trueFunction = processer.front().first;
                trueFunction[i] = '1';
                break;
            }    
        }
        //Error for unknown reason
        if(isDontCareExist){
            processer.push(make_pair(falseFunction, processer.front().second));
            processer.push(make_pair(trueFunction, processer.front().second));
        }
        else{
            functionBuffer.push_back(processer.front());
        }
        processer.pop();
    }
    //Get value after process
    plaInput.function = functionBuffer;

    for(int i = 0; i < truthTable.size(); i++){
        //SOP or POS
        bool initBool = !plaInput.function[0].second;
        truthTable[i] = initBool;
    }
    //Turn binary number into truth table
    for(int i = 0; i < plaInput.function.size(); i++){
        //read each value in function
        int index = 0;
        for(int j = 0; j < plaInput.function[i].first.size(); j++){
            if(plaInput.function[i].first[j] == '1'){
                index += pow(2, plaInput.function[i].first.size() - j - 1);
            }
            else if(plaInput.function[i].first[j] == '0'){
                //pass
            }
        }
        truthTable[index] = plaInput.function[i].second;
    }
    //Create tree
    //Init
    vector<node> binaryTree;
    //Amount of input = depth of the tree
    //2 ^ depth + 1(express true boolean) -> tree
    binaryTree.resize(pow(2, plaInput.inputAmount) + 1);
    
    binaryTree[0].varibleName = "false";
    binaryTree[binaryTree.size() - 1].varibleName = "true";
    cout << "hi";
    //Build
    //+1 for easer read
    for(int i = 1; i < plaInput.inputAmount + 1; i++){
        //set last node (to bool) 
        if(i == plaInput.inputAmount){
            for(int j = pow(2, i - 1); j < pow(2, i); j++){
                binaryTree[j].varibleName = plaInput.inputVaribles[i - 1];
                //false to index 0, true to index pow(2,i) (last index)
                //j - pow(2, i - 1) : 0, 1, 2, ... , 2 ^ (inputAmount - 1)
                //2 * j - pow(2, i - 1) : 0, 2, 4, ... , 2 ^ inputAmount
                binaryTree[j].elseEdge = truthTable[2 * (j - pow(2, i - 1))] * pow(2, i);
                binaryTree[j].thenEdge = truthTable[2 * (j - pow(2, i - 1)) + 1] * pow(2, i);           
            }
        }
        else{
        //set node 
            for(int j = pow(2, i - 1); j < pow(2, i); j++){
                
                binaryTree[j].varibleName = plaInput.inputVaribles[i - 1];
                if(j * 2 + 1 < binaryTree.size()){
                    binaryTree[j].elseEdge = j * 2;
                    binaryTree[j].thenEdge = j * 2 + 1;
                }
            }
        }
    }
    //Redundant
    for(int i = plaInput.inputAmount ; i >= 1; i--){
        //reduce
        for(int j = pow(2, i - 1); j < pow(2, i); j++){
            //edge to same point
            if(binaryTree[j].elseEdge == binaryTree[j].thenEdge){
                binaryTree[j].isRedundant = true;
                continue;
            }
            for(int k = j + 1; k < pow(2, i); k++){
                //same edge
                if(binaryTree[j].thenEdge == binaryTree[k].thenEdge){
                    if(binaryTree[j].elseEdge == binaryTree[k].elseEdge){
                        binaryTree[j].isRedundant = true;
                        binaryTree[j].thenEdge = k;
                        binaryTree[j].elseEdge = k;
                    }
                }
            }
        }
        //reconnect
        for(int j = 0; j < binaryTree.size(); j++){
            //Using while to get real end(Not reduced)
            //then edge to deleted
            while(binaryTree[binaryTree[j].thenEdge].isRedundant){
                binaryTree[j].thenEdge = binaryTree[binaryTree[j].thenEdge].thenEdge;
            }
            //else edge to deleted
            while(binaryTree[binaryTree[j].elseEdge].isRedundant){
                binaryTree[j].elseEdge = binaryTree[binaryTree[j].elseEdge].elseEdge;
            }
        }
    }
    //ROBDD Output
    //title
    output << "digraph ROBDD {\n";
    //rank
    for(int i = 1; i < plaInput.inputAmount + 1; i++){
        output << "{rank=same";
        for(int j = pow(2, i - 1); j < pow(2, i); j++){
            if(!binaryTree[j].isRedundant){
                output << " " << j;
            }
        }
        output << "}\n";
    }
    output << '\n';
    //node
    for(int i = 0; i < binaryTree.size(); i++){
         if(!binaryTree[i].isRedundant){
            if(i == 0 || i == binaryTree.size() - 1){
                output << i << " [label=";
                if(binaryTree[i].varibleName == "false"){
                    output << "0";
                }
                else if(binaryTree[i].varibleName == "true"){
                    output << "1";
                }
                output << ", shape=box";
            }
            else{
                output << i << " [label=\"" << binaryTree[i].varibleName << "\"";
            }
            output << "]\n";
        }
    }
    output << '\n';
    //connection
    for(int i = 1; i < binaryTree.size() - 1; i++){
        if(!binaryTree[i].isRedundant){
            output << i << " -> " << binaryTree[i].elseEdge << " [label=\"0\", style=dotted]\n";
            output << i << " -> " << binaryTree[i].thenEdge << " [label=\"1\", style=solid]\n";
        }
    }
    output << "}\n";
}
