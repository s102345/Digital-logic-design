#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <utility>
#include <queue>
#include <string>
#include <algorithm>
#include <set>
#include <map>

using namespace std;

string firstNode = "";

//Turn state into index
int stateToIndex(string state){
    //S_, S_
    if(state[0] == 'S' || state[0] == 's'){
        string beginNodeNum = "", stateNum = "";
        for(int i = 1; i < firstNode.length(); i++){
            beginNodeNum += firstNode[i];
        }
        for(int i = 1; i < state.length(); i++){
            stateNum += state[i];
        }
        return stoi(stateNum) - stoi(beginNodeNum);
    }
    //A, B, C
    else{
        return state[0] - firstNode[0];
    }
}

//Turn index into state
string indexToState(int index){
    //S_, S_
    if(firstNode[0] == 'S' || firstNode[0] == 's'){
        string beginNodeNum = "";
        for(int i = 1; i < firstNode.length(); i++){
            beginNodeNum += firstNode[i];
        }   
        return firstNode[0] + to_string(stoi(beginNodeNum) + index);
    }
    //A, B, C
    else{
        //Pretending to be str
        string str =  "";
        str += char(firstNode[0] + index);
        return str;
    }
}

bool cmp(pair<string, int> a, pair<string, int> b){
    return a.second < b.second;
}

//edge infomation
struct edge{
    string inputValue;
    string source;
    string destination;
    int outputValue;
};

struct kissForm{
    int inputAmount = 0;
    int outputAmount = 0;
    int edgeAmount = 0;
    int stateAmount = 0;
    vector<edge> allEdges;
    string beginNode;
};

int main(int argc, char* argv[]){
    //command be like mini_kiss <kiss file> <kiss file> <dot file>
    //                argv[0]    argv[1]    argv[2]  argv[3]
    
    fstream input, kiss_output, dot_output;
    
    if(argc != 4){
        cout << "Unknown command!\n";
        return 0;
    }
    
    input.open(argv[1], ios::in);
    kiss_output.open(argv[2], ios::out);
    dot_output.open(argv[3], ios::out);
    
    /*
    input.open("test2.kiss", ios::in);
    kiss_output.open("test2.out", ios::out);
    dot_output.open("test2_dot.dot", ios::out);
    */

    kissForm kissInput;

    //Read input
    string command;
    while(input >> command){
    
        if(command == ".start_kiss"){
            continue;
        }

        if(command == ".i"){
            input >> kissInput.inputAmount;
        }

        if(command == ".o"){
            input >> kissInput.outputAmount;
        }

        if(command == ".p"){
            input >> kissInput.edgeAmount;
        }

        if(command == ".s"){
            input >> kissInput.stateAmount;
        }
        if(command == ".r"){
            input >> kissInput.beginNode;
            kissInput.allEdges.resize(kissInput.edgeAmount);
            for(int i = 0; i < kissInput.edgeAmount; i++){
                edge input_buffer;        
                input >> input_buffer.inputValue >> input_buffer.source >> input_buffer.destination >> input_buffer.outputValue;
                if(i == 0){
                    firstNode = input_buffer.source;
                }
                kissInput.allEdges[i] = input_buffer;
            }
        }
        
        if(command == ".end_kiss"){
            break;
        }
    }
    //Build implication
    //Build statement table
    //next_state output
    typedef pair<string, bool> nextState;
    //vector of input_0(next_state, output) and input_1(next_state, output) => index[0] = A, index[1] = B ...  
    vector<vector<nextState>> statementTable;
    //Init
    statementTable.resize(kissInput.stateAmount);
    for(int i = 0; i < kissInput.stateAmount; i++){
        statementTable[i].resize(pow(2, kissInput.inputAmount));
        for(int j = 0; j < pow(2, kissInput.inputAmount); j++){
            statementTable[i][j].first = kissInput.allEdges[i * pow(2, kissInput.inputAmount) + j].destination;
            statementTable[i][j].second = kissInput.allEdges[i * pow(2, kissInput.inputAmount) + j].outputValue;
        }
    }

    //outputName outputTimes
    vector<pair<string, int>> outputCount;
    for(int i = 0; i < kissInput.stateAmount; i++){
        string outputBuf = "";
        for(int j = 0; j < pow(2, kissInput.inputAmount); j++){
            outputBuf += to_string(statementTable[i][j].second);
        }
        bool isExist = false;
        for(int j = 0; j < outputCount.size(); j++){
            if(outputCount[j].first == outputBuf){
                isExist = true;
                outputCount[j].second++;
            }
        }
        if(!isExist){
            outputCount.push_back(make_pair(outputBuf, 1));
        }
    }
    
    sort(outputCount.begin(), outputCount.end(), cmp);

    for(int cursor = 0; cursor < outputCount.size(); cursor++){
        if(outputCount[cursor].second == 1){
            continue;
        }

        //Implication table
        //next_state pair
        typedef pair<string, string> nextStateComparison;
        //Empty
        //2D array with input_0 input_1 comparison
        vector<vector<vector<nextStateComparison>>> implicationTable;
        //Init
        implicationTable.resize(statementTable.size());

        for(int i = 0; i < statementTable.size(); i++){
            implicationTable[i].resize(statementTable.size());
            //Init with empty
            for(int j = 0; j < statementTable.size(); j++){
                implicationTable[i][j].resize(pow(2, kissInput.inputAmount));
                for(int k = 0; k < pow(2, kissInput.inputAmount); k++){
                    implicationTable[i][j][k] = make_pair("0", "0");
                }
            }
        }
      
        //Fill with value
        for(int i = 1; i < statementTable.size(); i++){
            for(int j = 0; j < i; j++){
                //row next -> col next
                for(int k = 0; k < pow(2, kissInput.inputAmount); k++){
                    implicationTable[i][j][k].first = statementTable[i][k].first;
                    implicationTable[i][j][k].second = statementTable[j][k].first;
                }
            }
        }

        vector<vector<vector<nextStateComparison>>> tempImplicationTable = implicationTable;
        //Remove output incompatible pairs by finding the different output value
        
       
        for(int i = 0; i < statementTable.size(); i++){
            string outputBuf = "";
            for(int j = 0; j < pow(2, kissInput.inputAmount); j++){
                outputBuf += to_string(statementTable[i][j].second);
            }
            if(outputBuf != outputCount[cursor].first){
                //row del
                for(int j = 0; j < i; j++){
                    for(int k = 0; k < pow(2, kissInput.inputAmount); k++){
                        tempImplicationTable[i][j][k] = make_pair("-", "-");
                    }
                }
                //col del
                for(int j = 0; j < statementTable.size() - i - 1; j++){
                    for(int k = 0; k < pow(2, kissInput.inputAmount); k++){
                        tempImplicationTable[statementTable.size() - j - 1][i][k] = make_pair("-", "-");
                    }
                }
            }
        }
        implicationTable = tempImplicationTable;
       
        //Remove incompatible transition pairs
        bool isAnyIncompatible = false;

        do{
            tempImplicationTable = implicationTable;
            isAnyIncompatible = false;
            for(int i = 1; i < statementTable.size(); i++){
                for(int j = 0; j < i; j++){
                    if(implicationTable[i][j][0].first == "-" && implicationTable[i][j][0].second == "-"){
                        continue;
                    }
               
                    int index_0, index_1;
                    for(int k = 0; k < pow(2, kissInput.inputAmount); k++){
                        index_0 = stateToIndex(implicationTable[i][j][k].first);
                        index_1 = stateToIndex(implicationTable[i][j][k].second);
                        if(index_1 > index_0) swap(index_0, index_1);
                        if((implicationTable[index_0][index_1][0].first == "-" && implicationTable[index_0][index_1][0].second == "-")){
                            isAnyIncompatible = true;
                            for(int l = 0; l < pow(2, kissInput.inputAmount); l++){
                                tempImplicationTable[i][j][l] = make_pair("-", "-");
                            }     
                        }
                    }
                }
            }
            implicationTable = tempImplicationTable;  
        }while(isAnyIncompatible);
   
        //Merge
        //Remaining index -> can be merge 
        for(int i = 1; i < statementTable.size(); i++){
            for(int j = 0; j < i; j++){
                //Compatible
                if(implicationTable[i][j][0].first != "-" && implicationTable[i][j][0].second != "-"){
                    int merge_index = j, be_merged_index = i;
                    string merge_state, be_merged_state;
                    merge_state = indexToState(merge_index);
                    be_merged_state = indexToState(be_merged_index);
                    if(kissInput.beginNode == be_merged_state){
                        kissInput.beginNode = merge_state;
                    }
                    //repeated
                    if(statementTable[be_merged_index][0].first == "-"){
                        break;
                    }
                    //cout << merge_state << " " << be_merged_state << '\n';
                    for(int k = 0; k < statementTable.size(); k++){
                        if(k == be_merged_index){
                            for(int l = 0; l < pow(2, kissInput.inputAmount); l++){
                                statementTable[k][l].first = "-";    
                            }
                        }
                        else{
                            for(int l = 0; l < pow(2, kissInput.inputAmount); l++){
                                if(statementTable[k][l].first == be_merged_state){
                                    statementTable[k][l].first = merge_state;
                                }

                            }
                        }
                    }  
                }
            }
        }
        /*
        vector<vector<nextState>> tempStatementTable;
        for(int i = 0; i < statementTable.size(); i++){
            if(this_round_be_merged.find(i) != this_round_be_merged.end()){
                continue;
            }
            else{
                tempStatementTable.push_back(statementTable[i]);
            }
        }
        statementTable = tempStatementTable;
        */
        /*
        //Print
        for(int i = 0; i < statementTable.size(); i++){
            for(int j = 0; j < pow(2, kissInput.inputAmount); j++){
                cout << "(" << statementTable[i][j].first << ", " << statementTable[i][j].second << ")";
            }
            cout << '\n';
        }
        cout << '\n';  
        */
    }

    int realStatementSize = 0;
    for(int i = 0; i < statementTable.size(); i++){
        if(statementTable[i][0].first == "-"){
            continue;
        }
        realStatementSize++;
    }
    

    //Kiss output
    kiss_output << ".start_kiss\n";
    kiss_output << ".i " << kissInput.inputAmount << '\n';
    kiss_output << ".o " << kissInput.outputAmount << '\n';
    kiss_output << ".p " << realStatementSize * pow(2, kissInput.inputAmount) << '\n';
    kiss_output << ".s " << realStatementSize << '\n';
    kiss_output << ".r " << kissInput.beginNode << '\n';
    for(int i = 0; i < statementTable.size(); i++){
        for(int j = 0; j < pow(2, kissInput.inputAmount); j++){
            if(statementTable[i][0].first == "-"){
                continue;
            }
            //input
            if(kissInput.inputAmount == 1){
                kiss_output << j << " ";
            }
            else if(kissInput.inputAmount == 2){
                switch(j){
                    case 0:
                        kiss_output << "00 ";
                        break;
                    case 1:
                        kiss_output << "01 ";
                        break;
                    case 2:
                        kiss_output << "10 ";
                        break;
                    case 3:
                        kiss_output << "11 ";
                        break;
                }
            }
            kiss_output << indexToState(i) << " ";
            //destniation & output
            kiss_output << statementTable[i][j].first << " " << statementTable[i][j].second << '\n';        
        }
    }
    kiss_output << ".end_kiss\n"; 
    //output dot
    dot_output << "digraph STG {\n";
    dot_output << "rankdir=LR;\n";
    dot_output << "INIT [shape=point];\n";
    for(int i = 0; i < statementTable.size(); i++){
        if(statementTable[i][0].first == "-"){
            continue;
        }
        dot_output << indexToState(i) << " [label=\"" << indexToState(i) << "\"];\n";
    }
    dot_output << '\n';
    
    dot_output << "INIT -> " << kissInput.beginNode << ";\n";
    for(int i = 0; i < statementTable.size(); i++){
        for(int j = 0; j < pow(2, kissInput.inputAmount); j++){
            if(statementTable[i][0].first == "-"){
                continue;
            }
            dot_output << indexToState(i) << " -> " << statementTable[i][j].first;
            dot_output << " [label=\"";
            if(kissInput.inputAmount == 1){
                dot_output << j;
            }
            else if(kissInput.inputAmount == 2){
                switch(j){
                    case 0:
                        dot_output << "00";
                        break;
                    case 1:
                        dot_output << "01";
                        break;
                    case 2:
                        dot_output << "10";
                        break;
                    case 3:
                        dot_output << "11";
                        break;
                }
            }
            dot_output << "/";
            dot_output << statementTable[i][j].second;
            dot_output << "\"]\n";
        }
    }
    dot_output << "}";
}