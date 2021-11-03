#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <utility>
#include <queue>
#include <string>
#include <algorithm>

using namespace std;

struct plaForm{
    int inputAmount = 0;
    int outputAmount = 0;
    vector<char> inputVaribles;
    vector<char> outputVaribles;
    int booleanFunctionAmount = 0;
    vector<string> function;
};

vector<string> dont_care_processer(vector<string> functions){
    //preprocess don't care
    //buffer
    vector<string> functionBuffer;
    //processer
    queue<string> processer;
    for(int i = 0; i < functions.size(); i++){
        processer.push(functions[i]);
    }
    //process until there's no don't care
    while(!processer.empty()){
        bool isDontCareExist = false;
        string falseFunction, trueFunction;
        for(int i = 0; i < processer.front().size(); i++){
            //travel each function for don't care
            if(processer.front()[i] == '-'){
                isDontCareExist = true;
                //turn don't care into 0/1 then process again
                falseFunction = processer.front();
                falseFunction[i] = '0';
                trueFunction = processer.front();
                trueFunction[i] = '1';
                break;
            }    
        }
        //Error for unknown reason
        if(isDontCareExist){
            processer.push(falseFunction);
            processer.push(trueFunction);
        }
        else{
            //cout << processer.front().first << endl;
            if(find(functionBuffer.begin(), functionBuffer.end(), processer.front()) == functionBuffer.end())
                functionBuffer.push_back(processer.front());
        }
        processer.pop();
    }
    return functionBuffer;
}

vector<int> bestSolSet;

void petrickBestSol(vector<vector<bool>> primeTable, vector<int> targetTerms, vector<int> solSet, int ptr){
    if(ptr >= primeTable.size()){
        return;
    }
    //Fulfill check
    if(solSet.size() < bestSolSet.size() || bestSolSet.empty()){
        bool canBeFulfilled = true;
        for(int i = 0; i < targetTerms.size(); i++){
            bool flag = false;
            for(int j = 0; j < solSet.size(); j++){
                if(primeTable[solSet[j]][targetTerms[i]]){
                    flag = true;
                }
            }
            if(!flag){
                canBeFulfilled = false;
            }
        }
        if(canBeFulfilled){
            bestSolSet = solSet;
        }
    }
    //Choose
    solSet.push_back(ptr);
    //cout << ptr << " ";
    petrickBestSol(primeTable, targetTerms, solSet, ptr + 1);
    solSet.pop_back();
    //Not to choose
    petrickBestSol(primeTable, targetTerms, solSet, ptr + 1);
}

int main(int argc, char* argv[]){
    //command be like ROBDD   <pla file> <dot file>
    //                argv[0]   argv[1]   argv[2]
    
    if(argc != 3){
        cout << "Unknown command!\n";
        return 0;
    }
    fstream input, output;
    input.open(argv[1], ios::in);
    output.open(argv[2], ios::out);
    //Input Process
    //Store pla file
    string prefix;
    struct plaForm plaInput;
    //Input read
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
                char resultPreprocesser;
                bool resultBuffer;
                input >> functionBuffer >> resultPreprocesser;
                plaInput.function.push_back(functionBuffer);
            }
        }
        //end 
        if(prefix == ".e"){
            break;
        }
    }
    //Input process
    //Get value after process
    vector<string> inputFunctions = dont_care_processer(plaInput.function);
    
    //PI
    //Finding prime implicants
    //Buffer
    vector<string> primeBuffer;
    
    while(true){
        bool isReductionExist = false;
        for(int i = 0; i < inputFunctions.size(); i++){
            for(int j = i + 1; j < inputFunctions.size(); j++){
                //Check for the difference of 1 char
                int differenceCount = 0;
                int differencePos = 0;
                //cout << inputFunctions[i] << " " << inputFunctions[j] << '\n';
                for(int k = 0; k < plaInput.inputAmount; k++){
                    if(inputFunctions[i][k] != inputFunctions[j][k]){
                        differenceCount++;
                        differencePos = k;
                    }
                }
                if(differenceCount == 1){
                    isReductionExist = true;
                    string buffer = inputFunctions[i];
                    buffer[differencePos] = '-';
                    //If not exist
                    if(find(primeBuffer.begin(), primeBuffer.end(), buffer) == primeBuffer.end()){
                        primeBuffer.push_back(buffer);
                    }
                }
            }
        }
        if(!isReductionExist){
            break;
        }
        else{
            inputFunctions = primeBuffer;
            primeBuffer.clear();
        }
    }
    vector<string> reducedOutput;
    //Essential Prime chart
    vector<vector<bool>> primeTable (inputFunctions.size(), vector<bool>(pow(2, plaInput.inputAmount), false));
    //Fill in table
    for(int i = 0; i < inputFunctions.size(); i++){
        //Turn product term into numbers
        //Process don't care
        vector<string> minterms;
        queue<string> processer;
        processer.push(inputFunctions[i]);
        while(!processer.empty()){
            string falseFunction, trueFunction;
            bool isDontCareExist = false;
            for(int i = 0; i < processer.front().size(); i++){
                //travel each function for don't care
                if(processer.front()[i] == '-'){
                    isDontCareExist = true;
                    //turn don't care into 0/1 then process again
                    falseFunction = processer.front();
                    falseFunction[i] = '0';
                    trueFunction = processer.front();
                    trueFunction[i] = '1';
                    break;
                }    
            }
            if(isDontCareExist){
                processer.push(falseFunction);
                processer.push(trueFunction);
            }
            else{
                minterms.push_back(processer.front());
            }
            processer.pop();
        }
        //Turn minterms into number
        //Don't care amount indicate the amount of minterms
        for(int j = 0; j < minterms.size(); j++){
            //read each value in function
            int index = 0;
            for(int k = 0; k < minterms[j].size(); k++){
                if(minterms[j][k] == '1'){
                    index += pow(2, plaInput.inputAmount - k - 1);
                }
            }
            primeTable[i][index] = true;
        }
    }
    vector<vector<bool>> primeTableBuffer = primeTable;
    vector<string> inputFunctionBuf = inputFunctions;
    //Calculate essential prime
    for(int i = 0; i < pow(2, plaInput.inputAmount); i++){
        //Count times
        int count = 0;
        int onlyOneContained = 0;
        for(int j = 0; j < primeTable.size(); j++){
            if(primeTable[j][i]){
                count++;
                onlyOneContained = j;
            }
        }
        if(count == 1){
            reducedOutput.push_back(inputFunctions[onlyOneContained]);
            primeTableBuffer.erase(find(primeTableBuffer.begin(), primeTableBuffer.end(), primeTable[onlyOneContained]));
            inputFunctionBuf.erase(find(inputFunctionBuf.begin(), inputFunctionBuf.end(), inputFunctions[onlyOneContained]));
            vector<int> indexToBeClear;
            //Clear repeated 
            for(int j = 0; j < pow(2, plaInput.inputAmount); j++){
                if(primeTable[onlyOneContained][j]){
                    indexToBeClear.push_back(j);
                }
            }
            for(int j = 0; j < indexToBeClear.size(); j++){
                for(int k = 0; k < primeTableBuffer.size(); k++){
                    primeTableBuffer[k][indexToBeClear[j]] = false;
                }
            }
        }
    }
    inputFunctions = inputFunctionBuf;
    primeTable = primeTableBuffer;
    //Patrick's Method
    //List target term
    vector<int> target_Terms;
    for(int i = 0; i < pow(2, plaInput.inputAmount); i++){
        bool flag = false;
        for(int j = 0; j < primeTable.size(); j++){
            if(primeTable[j][i]){
                flag = true;
            }
        }
        if(flag){
            target_Terms.push_back(i);
        }
    }
    //Find the combination to fulfill target term
    vector<int> solSetBuffer;
    petrickBestSol(primeTable, target_Terms, solSetBuffer, 0);
    for(int i = 0; i < bestSolSet.size(); i++){
        reducedOutput.push_back(inputFunctions[bestSolSet[i]]);
    }
    
    //Output
    output << ".i " << plaInput.inputAmount << '\n';
    output << ".o " << plaInput.outputAmount << '\n';
    output << ".lib ";
    for(int i = 0; i < plaInput.inputVaribles.size(); i++){
        output << plaInput.inputVaribles[i] << ' ';
    }
    output << '\n';
    output << ".ob " << plaInput.outputVaribles[0] << '\n';
    output << ".p ";
    output << reducedOutput.size() << '\n';
    for(int i = 0; i < reducedOutput.size(); i++){
        output << reducedOutput[i] << " 1\n";
    }
    output << ".e";
}
