#include<iostream>
#include<fstream>
#include<string>
#include<map>
#include<set>
#include<vector>
#include<algorithm>
#include<limits>
#include<random>
#include<chrono>
#include<iterator>
#include<cmath>
#include<ctime>
using namespace std;

struct Macro{
    Macro() {};
    Macro(int w, int h): width(w), height(h) {};
    int width,height;
    int x,y;
};

struct Terminal{
    Terminal() {};
    Terminal(int xx, int yy): x(xx), y(yy) {};
    int x,y;
};

struct Position {
    Position() {};
    int x,y;
};

map<string, Macro> macroMap; // from Macro name to find Macro object
vector<string> macroHash; //from sequence to macroName
map<string, Terminal> terminalMap; // from terminalName to find Terminal obj
map<string, bool> terminalOrMacro; //terminal for 0 , macro for 1
vector<set<string>> netlist; // vector of nets
vector<int> positiveLoci,negativeLoci; // sequence, from 1 to n
int outlineWidth,outlineHeight,numBlocks,numTerminals,numNets;
random_device rd;
default_random_engine gen(rd());
double a,Rstar;
vector<Position> match;
int chipWidth,chipHeight;
time_t start,ending;
bool WH;
int countSA = 0;

void read_block(string fileName) {
    ifstream fin(fileName);
    string trash;
    fin >> trash >> outlineWidth >> outlineHeight >> trash >> numBlocks >> trash >> numTerminals;
    if(outlineHeight >= outlineWidth) {
        Rstar = (double)outlineHeight/outlineWidth;
        WH = false;  
    }
    else {
        Rstar = (double)outlineWidth/outlineHeight;
        WH = true;
    }
    string macroName,terminalName;
    int macroWidth,macroHeight,terminalX,terminalY;
    positiveLoci.resize(numBlocks+1);
    negativeLoci.resize(numBlocks+1);
    macroHash.resize(numBlocks+1);
    match.resize(numBlocks+1);
    for(int i = 1; i <= numBlocks; ++i) {
        fin >> macroName >> macroWidth >> macroHeight;
        macroMap[macroName] = Macro(macroWidth,macroHeight);
        macroHash[i] = macroName;
        positiveLoci[i] = i;
        negativeLoci[i] = i;
        terminalOrMacro[macroName] = true;
    }
    for(int i = 0; i < numTerminals; ++i) {
        fin >> terminalName >> trash >> terminalX >> terminalY;
        terminalMap[terminalName] = Terminal(terminalX,terminalY);
        terminalOrMacro[terminalName] = false;
    }
    fin.close();
}

void read_net(string fileName) {
    ifstream fin(fileName);
    string trash;
    fin >> trash >> numNets;
    int degrees;
    string pinName;
    for(int i = 0;i < numNets; ++i) {
        fin >> trash >> degrees;
        set<string> nets;
        for(int j = 0; j < degrees; ++j) {
            fin >> pinName;
            nets.insert(pinName);
        }
        netlist.push_back(nets);
    }
    fin.close();
}

int HPWL() {
    int totalLength = 0;
    for(auto it = netlist.begin(); it != netlist.end(); ++it) {
        set<string>& nets = *it;
        int leftx = numeric_limits<int>::max(), rightx = 0;
        int downy = numeric_limits<int>::max(), upy = 0;
        for(auto _it = nets.begin(); _it != nets.end(); ++_it) {
            if(terminalOrMacro[*_it] == true) { //macro
                Macro& curMacro = macroMap[*_it];
                int macroX = curMacro.x + curMacro.width/2;
                int macroY = curMacro.y + curMacro.height/2;
                leftx = min(leftx,macroX);
                rightx = max(rightx,macroX);
                downy = min(downy,macroY);
                upy = max(upy,macroY); 
            }
            else {
                Terminal& curTerminal = terminalMap[*_it];
                int terminalX = curTerminal.x;
                int terminalY = curTerminal.y;
                leftx = min(leftx,terminalX);
                rightx = max(rightx,terminalX);
                downy = min(downy,terminalY);
                upy = max(upy,terminalY); 
            }
        }
        totalLength += (rightx-leftx) + (upy-downy);
    }
    return totalLength;
}

int find_BST(map<int,int>& indexLength, int p) {
    auto lb = indexLength.lower_bound(p);
    if(lb != indexLength.begin()) {
        lb--;
    }
    return lb->second;
}

void initialMatch() {
    for(int i = 1; i <= numBlocks; ++i) {
        match[positiveLoci[i]].x = i;
        match[negativeLoci[i]].y = i;
    }
}

int Area() {
    map<int,int> indexLength;
    vector<int> P_x(numBlocks+1);
    vector<int> P_y(numBlocks+1);
    indexLength[0] = 0;
    for(int i = 1; i <= numBlocks; ++i) {
        int b = positiveLoci[i];
        int p = match[b].y;
        P_x[b] = find_BST(indexLength,p);
        int t = P_x[b] + macroMap[macroHash[b]].width;
        indexLength[p] = t;
        auto ub = indexLength.upper_bound(p);
        while(ub != indexLength.end()) {
            if(ub->second < t) {
                ub = indexLength.erase(ub);
            }
            else ub++;
        }
        macroMap[macroHash[b]].x = P_x[b];
    }
    chipWidth = (prev(indexLength.end()))->second;
    indexLength.clear();
    indexLength[0] = 0;
    for(int i = numBlocks; i >= 1; --i) {
        int b = positiveLoci[i];
        int p = match[b].y;
        P_y[b] = find_BST(indexLength,p);
        int t = P_y[b] + macroMap[macroHash[b]].height;
        indexLength[p] = t;
        auto ub = indexLength.upper_bound(p);
        while(ub != indexLength.end()) {
            if(ub->second < t) {
                ub = indexLength.erase(ub);
            }
            else ub++;
        }
        macroMap[macroHash[b]].y = P_y[b];
    }
    chipHeight = (prev(indexLength.end()))->second;
    return chipHeight*chipWidth;
}

void initSequence() {
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(positiveLoci.begin()+1, positiveLoci.end(), default_random_engine(seed));
    seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(negativeLoci.begin()+1, negativeLoci.end(), default_random_engine(seed));
}

double getCost() {
    int area = Area();
    // cout << area << endl;
    int wireLength = HPWL();
    
    double R;
    if(!WH)
        R = (double) chipHeight/chipWidth;
    else
        R = (double) chipWidth/chipHeight;
    if(countSA > 300) 
        return a*area + 5*abs(Rstar-R);
    else 
        return a*area + (1-a)*wireLength + 5*abs(Rstar-R);
}

void swapX(int disturbedBlock1,int disturbedBlock2) {
    int pos1 = match[disturbedBlock1].x;
    int pos2 = match[disturbedBlock2].x;
    int temp = positiveLoci[pos1];
    positiveLoci[pos1] = positiveLoci[pos2];
    positiveLoci[pos2] = temp;
    temp = match[disturbedBlock1].x;
    match[disturbedBlock1].x = match[disturbedBlock2].x;
    match[disturbedBlock2].x = temp;
}

void swapY(int disturbedBlock1, int disturbedBlock2) {
    int pos1 = match[disturbedBlock1].y;
    int pos2 = match[disturbedBlock2].y;
    int temp = negativeLoci[pos1];
    negativeLoci[pos1] = negativeLoci[pos2];
    negativeLoci[pos2] = temp;
    temp = match[disturbedBlock1].y;
    match[disturbedBlock1].y = match[disturbedBlock2].y;
    match[disturbedBlock2].y = temp;
}

void move(int choice, int sequence, int disturbedBlock1,int disturbedBlock2) {
    if(choice == 1) {
        string macroName = macroHash[disturbedBlock1];
        Macro& macro = macroMap[macroName];
        int temp = macro.height;
        macro.height = macro.width;
        macro.width = temp;
    }
    else if(choice == 2) {
        if(sequence == 1) 
            swapX(disturbedBlock1,disturbedBlock2);           
        else 
            swapY(disturbedBlock1,disturbedBlock2);
    }
    else {
        swapX(disturbedBlock1,disturbedBlock2); 
        swapY(disturbedBlock1,disturbedBlock2); 
    }
}

void SA() {
    double temp = 1e3;
    uniform_int_distribution<> dis(1,3);
    uniform_int_distribution<> selectBlock(1,numBlocks);
    uniform_int_distribution<> selectSequence(1,2);
    uniform_real_distribution<> disexp(0.0,1.0);
    double cost = getCost();
    int reject = 0;
    int iteration_cnt = 1200;
    double current;
    while(1) {
        // cout.precision(numeric_limits<double>::max_digits10+1); 
        // cout << cost << "\n"; 
        reject = 0;
        for(int i = 0; i < iteration_cnt; ++i) {
            // cout << "Cost " << cost << "\n";
            int choice = dis(gen);
            int disturbedBlock1 = selectBlock(gen);
            int disturbedBlock2 = selectBlock(gen);
            int sequence = selectSequence(gen);
            move(choice,sequence,disturbedBlock1,disturbedBlock2);
            double newCost = getCost();
            if(newCost > cost) {
                // cout << "Temp " << temp << "\n";
                double exponent = -(newCost-cost)/temp;
                exponent = exp(exponent);
                if(disexp(gen) > exponent) {
                    reject++;
                    move(choice,sequence,disturbedBlock1,disturbedBlock2);
                    continue;
                }
            }
            cost = newCost;
        }
        temp *= 0.7;
        ending = time(NULL);
        current = difftime(ending,start);
        double rejectRate = (double)reject/iteration_cnt;
        if(temp < 1e-3 || rejectRate > 0.95 || current > 295) {
            return;
        }
    }
}

int main(int argc, char** argv) 
{
    if(argc != 5) {
        printf("There need to be 4 arguments\n");
        return 0;
    }
    a = stod(argv[1]);
    string inputBlockName = argv[2];
    string inputNetName = argv[3];
    string output_name = argv[4];
    
    double diff;
    start = time(NULL);
    read_block(inputBlockName);
    read_net(inputNetName);
    int count = 0;
    double oneSA_time;
    while(1) {
        initSequence();
        initialMatch();
        SA();
        countSA++;
        ending = time(NULL);
        time_t cuurent = difftime(ending,start);
        if(count++ == 0) {
            oneSA_time = cuurent;
        }
        if((chipHeight > outlineHeight || chipWidth > outlineWidth) && (295-cuurent) > oneSA_time) {
            continue;
        }
        break;
    }
    ending = time(NULL);
	diff = difftime(ending,start);
    int area = Area();
    int wireLength = HPWL();
    double op1 = (double)a*area;
    double op2 = (double)(1-a)*wireLength;
    double cost = op1+op2;
    ofstream fout(output_name);
    fout.precision(numeric_limits<double>::max_digits10+1);
    fout << cost << "\n" << wireLength << "\n" << area << "\n" << chipWidth << " " << chipHeight << "\n" << diff << "\n";
    for(auto it = macroMap.begin(); it != macroMap.end(); ++it) {
        fout << it->first << " " << it->second.x << " " << it->second.y << " " << (it->second.x + it->second.width) << " " << (it->second.y + it->second.height) << "\n";
    }
    return 0;
}
