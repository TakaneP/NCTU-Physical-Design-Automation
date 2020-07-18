#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<map>
#include<sstream>
#include<vector>
#include<algorithm>
#include<limits>
#include<cmath>
#include<set>

using namespace std;

struct Node{
    Node(string n, int x,int y): name(n), pos_x(x), pos_y(y) {};
    Node() = default;
    string name;
    int pos_x,pos_y,width,height,legal_x,legal_y;
};

struct Terminal{
    Terminal(int x,int y): pos_x(x), pos_y(y) {};
    Terminal() = default;
    int pos_x,pos_y,width,height;
};

struct Cluster{
    Cluster() {totalWeight = 0; totalWidth = 0; quadratic = 0;};
    int optimal_x,totalWeight,totalWidth,quadratic,firstCell,lastCell;
};

//Cells on row are clustered in PlaceRow, cluster should be cleared everytime PlaceRow are called
struct Row
{
    Row() = default;
    Row(int coor,int hei,int siteSp, int subRow,int numSite): coordinate(coor), height(hei), siteSpacing(siteSp), subRowOrigin(subRow), numSites(numSite) {rightBotx = subRowOrigin + numSites*siteSpacing;totalWidth=0;};
    int coordinate,height,siteSpacing,subRowOrigin,numSites;
    int rightBotx;
    int totalWidth;
    vector<Cluster> clusters;
    vector<int> cells;
};

bool cmp_row(Row& x, Row& y) { return x.coordinate < y.coordinate;}
bool cmp_node(Node& x, Node& y) { return x.pos_x < y.pos_x;}

vector<Node> nodes;
map<string,int> nodeHash;
map<string,Terminal> terminals;
vector<Row> rows;
int numRows,numNodes,numTerminals;
string dir, nodeFile, plFile, sclFile;
map<int,set<int>> rowHeight; //coordinate to set of rows

void file_parser(string auxFile)
{
    ifstream fin(auxFile);
    string trash,node,nets,wts,pl,scl,shapes;
    fin >> trash >> trash >> node >> nets >> wts >> pl >> scl >> shapes;
    fin.close();
    vector<string> dirPos;
    string delimiter = "/";
    size_t pos = 0;
    string token;
    while((pos = auxFile.find(delimiter)) != string::npos) {
        token = auxFile.substr(0,pos);
        dirPos.push_back(token);
        auxFile.erase(0, pos + delimiter.length());
    }
    
    for(auto it = dirPos.begin(); it != dirPos.end(); ++it) {
        dir += *it;
        dir += "/";
    }
    nodeFile = dir + node;
    plFile = dir + pl;
    sclFile = dir + scl;

    ifstream finPL(plFile);
    string line;
    getline(finPL,line);
    while(getline(finPL,line)) {
        // cout << line << "\n";
        size_t found = line.find("#");
        if(found == 0 || line.empty()) continue;
        istringstream iss(line);
        string name;
        int pos_x,pos_y;
        iss >> name >> pos_x >> pos_y;
        size_t find_terminal = line.find("FIXED");
        size_t find_terminal_NI = line.find("FIXED_NI");
        if(find_terminal_NI != string::npos) continue;
        else if(find_terminal != string::npos)  {
            terminals[name] = Terminal(pos_x,pos_y);
        }
        else {
            nodes.push_back(Node(name,pos_x,pos_y));
            nodeHash[name] = nodes.size() - 1;
        }
    }
    finPL.close();
    ifstream finScl(sclFile);
    getline(finScl,line);
    while(line.find("NumRows") == string::npos)
        getline(finScl,line);

    {
        istringstream iss(line);
        iss >> trash >> trash >> numRows;
    }
    while(line.find("CoreRow") == string::npos)
        getline(finScl,line);
    
    for(int i = 0; i < numRows; ++i) {
        int coordinate,height,siteSpacing,subRowOrigin,numSites;
        int j = (i == 0)? 1:0;
        for(; j < 9; ++j) {
            getline(finScl,line);
            istringstream iss(line);
            if(j == 1)  
                iss >> trash >> trash >> coordinate;
            else if(j == 2) 
                iss >> trash >> trash >> height;
            else if(j == 4) 
                iss >> trash >> trash >> siteSpacing;
            else if(j == 7) 
                iss >> trash >> trash >> subRowOrigin >> trash >> trash >> numSites;
        }
        rows.push_back(Row(coordinate,height,siteSpacing,subRowOrigin,numSites));
    }
    finScl.close();

    // for(auto it = rows.begin(); it != rows.end(); ++it) 
    //     cout << it->first << " " << it->second.coordinate << " " << it->second.height << " " << it->second.siteSpacing << " " << it->second.subRowOrigin << " " << it->second.numSites  << " " << it->second.rightBotx << "\n"; 
    

    ifstream finNodes(nodeFile);
    getline(finNodes,line);
    while(line.find("NumNodes") == string::npos)
        getline(finNodes,line);
    {
        istringstream iss(line);
        iss >> trash >> trash >> numNodes;
    }
    getline(finNodes,line);
    {
        istringstream iss(line);
        iss >> trash >> trash >> numTerminals;
    }
    while(getline(finNodes,line)) {
        size_t found = line.find("#");
        if(found == 0 || line.empty()) continue;
        istringstream iss(line);
        string name;
        int width,height;
        iss >> name >> width >> height;
        
        size_t find_terminal = line.find("terminal");
        size_t find_terminal_NI = line.find("terminal_NI");
        if(find_terminal_NI != string::npos) continue;
        else if(find_terminal != string::npos) {
            terminals[name].width = width;
            terminals[name].height = height;
        }
        else {
            int nodeIndex = nodeHash[name];
            nodes[nodeIndex].width = width;
            nodes[nodeIndex].height = height;
        }     
    }
    finNodes.close();
    
    // cout << nodes["o1224935"].pos_x << " " << nodes["o1224935"].pos_y << " " << nodes["o1224935"].width << " " << nodes["o1224935"].height << "\n";
    // cout << terminals["o1233057"].pos_x << " " << terminals["o1233057"].pos_y << " " << terminals["o1233057"].width << " " << terminals["o1233057"].height << "\n";
    // for(auto it = nodes.begin(); it != nodes.end(); ++it) {
    //     cout << it->first << " " << it->second.width << " " << it->second.height << " " << it->second.terminal << "\n";
    // }
    // cout << nodes["o0"].width << " " << nodes["o0"].height << " " << nodes["o0"].terminal << "\n";
    // cout << nodes["o1228973"].width << " " << nodes["o1228973"].height << " " << nodes["o1228973"].terminal << "\n";  
}

void checkMacro() {
    for(auto it = terminals.begin(); it != terminals.end(); ++it) {
        Terminal& terminal = it->second;
        int row_num = rows.size();
        for(int i = 0; i < row_num; ++i) {
            Row& row = rows[i];
            if(row.coordinate >= (terminal.pos_y + terminal.height) || (row.coordinate + row.height) <= terminal.pos_y) continue;
            if(row.subRowOrigin < terminal.pos_x && row.rightBotx > (terminal.pos_x + terminal.width)) {
                int originalRight = row.rightBotx;
                row.rightBotx = terminal.pos_x;
                row.numSites = (row.rightBotx - row.subRowOrigin) / row.siteSpacing;
                rows.push_back(Row(row.coordinate,row.height,row.siteSpacing,terminal.pos_x + terminal.width,(originalRight - terminal.pos_x - terminal.width)/row.siteSpacing));
            }
            else if(terminal.pos_x <= row.subRowOrigin && (terminal.pos_x + terminal.width) > row.subRowOrigin) {
                row.subRowOrigin = terminal.pos_x + terminal.width;
                row.numSites = (row.rightBotx - row.subRowOrigin) / row.siteSpacing;
            }
            else if((terminal.pos_x + terminal.width) >= row.rightBotx && terminal.pos_x < row.rightBotx) {
                row.rightBotx = terminal.pos_x;
                row.numSites = (row.rightBotx - row.subRowOrigin) / row.siteSpacing;
            }
        }
    }
}

void AddCell(Cluster& cluster, int i, Node& cell) {
    cluster.lastCell = i;
    cluster.totalWeight += 1; //assume all the weights of cells are 1
    cluster.quadratic += (cell.pos_x - cluster.totalWidth);
    cluster.totalWidth += cell.width; 
}

void AddCluster(Cluster& cluster1, Cluster& cluster2) {
    cluster1.lastCell = cluster2.lastCell;
    cluster1.totalWeight += cluster2.totalWeight;
    cluster1.quadratic += cluster2.quadratic - cluster2.totalWeight * cluster1.totalWidth;
    cluster1.totalWidth += cluster2.totalWidth;
}

void Collapse(Cluster& cluster,int xMin, int xMax, Cluster* predecessor,vector<Cluster>& clusters) {
    cluster.optimal_x = cluster.quadratic / cluster.totalWeight;
    if(cluster.optimal_x < xMin) cluster.optimal_x = xMin;
    if((cluster.optimal_x + cluster.totalWidth) > xMax) cluster.optimal_x = xMax - cluster.totalWidth;
    if(predecessor != NULL && (predecessor->optimal_x + predecessor->totalWidth) > cluster.optimal_x) {
        AddCluster(*predecessor,cluster);
        clusters.pop_back();
        Cluster* newPredecessor = NULL;
        if(clusters.size() > 1) newPredecessor = &clusters[clusters.size()-2];
        Collapse(clusters.back(), xMin, xMax, newPredecessor, clusters);
    }
}

void PlaceRow(Row& row) {
    vector<int>& cells = row.cells;
    vector<Cluster>& clusters = row.clusters;
    for(int i = 0; i < cells.size(); ++i) {
        int& cellIndex = cells[i];
        Node& cell = nodes[cellIndex]; //cells[i] Cell index
  
        Cluster* lastCluster = NULL;
        if(!clusters.empty()) lastCluster = &clusters.back();
        if(i == 0 || (lastCluster->optimal_x + lastCluster->totalWidth) <= cell.pos_x) {
  
            clusters.push_back(Cluster());
            Cluster& newCluster = clusters.back();
            newCluster.optimal_x = cell.pos_x;
            newCluster.firstCell = i;
            AddCell(newCluster,i,cell);
        }
        else {
  
            Cluster* predecessor = NULL;
            if(clusters.size() > 1) predecessor = &clusters[clusters.size()-2];
            AddCell(*lastCluster,i,cell);
            Collapse(*lastCluster,row.subRowOrigin,row.rightBotx,predecessor,clusters);
        }
    }
    
    int i = 0;
    for(auto it = clusters.begin(); it != clusters.end(); ++it) {
        int x = it->optimal_x;
        for(;  i <= it->lastCell; ++i) {
            Node& cell = nodes[cells[i]];
            cell.legal_x = x;
   
            x = x + cell.width;
        }
    }
    clusters.clear();
}

void Abacus() {
    for(int i = 0; i < nodes.size(); ++i) {
        // if(i%100 == 0) cout << i << "\n";
        double cBest = numeric_limits<double>::max();
        Node& cell = nodes[i];
        int rowBest;

        auto lb = rowHeight.lower_bound(cell.pos_y);
        if(lb != rowHeight.begin() || (lb == rowHeight.begin() && lb->first == cell.pos_y)) {
            if(lb == rowHeight.end() || lb->first != cell.pos_y) {
                lb--;
            }
            while(1) {
                if(abs(lb->first - cell.pos_y) > cBest) break;
                for(auto it = lb->second.begin(); it != lb->second.end(); ++it) {
                    Row& row = rows[*it];
                    if((row.rightBotx - row.subRowOrigin - row.totalWidth) < cell.width) continue;
                    row.cells.push_back(i);
                    cell.legal_y = row.coordinate;
                    int originalX = cell.pos_x;
                    if(cell.pos_x < row.subRowOrigin) cell.pos_x = row.subRowOrigin;
                    if((cell.pos_x + cell.width) > row.rightBotx) cell.pos_x = row.rightBotx - cell.width;              
                    PlaceRow(row);
                    double cost = sqrt(pow((cell.legal_x - cell.pos_x),2) + pow((cell.legal_y - cell.pos_y),2));
                    if(cost < cBest) {
                        cBest = cost;
                        rowBest = *it;
                    }
                    row.cells.pop_back();
                    cell.pos_x = originalX;
                }
                if(lb->first == rowHeight.begin()->first) break;
                lb--;
            }
        }
        auto ub = rowHeight.upper_bound(cell.pos_y);
        for(; ub != rowHeight.end(); ub++) {
            if(abs(ub->first - cell.pos_y) > cBest) break;
            for(auto it = ub->second.begin(); it != ub->second.end(); ++it) {
                Row& row = rows[*it];
                if((row.rightBotx - row.subRowOrigin - row.totalWidth) < cell.width) continue;
                row.cells.push_back(i);
                int originalX = cell.pos_x;
                if(cell.pos_x < row.subRowOrigin) cell.pos_x = row.subRowOrigin;
                if((cell.pos_x + cell.width) > row.rightBotx) cell.pos_x = row.rightBotx - cell.width;
                cell.legal_y = row.coordinate;         
                PlaceRow(row);
                double cost = sqrt(pow((cell.legal_x - cell.pos_x),2) + pow((cell.legal_y - cell.pos_y),2));
                if(cost < cBest) {
                    cBest = cost;
                    rowBest = *it;
                }
                row.cells.pop_back();
                cell.pos_x = originalX;
            }
        }
        

        // for(auto it = rows.begin(); it != rows.end(); ++it) {
        //     // cout << "Row " << rowInd << "\n";
        //     it->cells.push_back(i);
        //     cell.legal_y = it->coordinate;
        //     int originalX = cell.pos_x;
        //     // if(it->cells.size() != 0 && (it->rightBotx - nodes[it->cells.back()].legal_x - nodes[it->cells.back()].width) < cell.width) continue;
        //     // if(originalX < it->subRowOrigin) {
        //     //     if(it->cells.size() == 0) {
        //     //         cell.pos_x = it->subRowOrigin;
        //     //     }z
        //     //     else {
        //     //         cell.pos_x = nodes[it->cells.back()].legal_x;
        //     //     }
        //     // }
        //     // else if((originalX + cell.width) > it->rightBotx) {
        //     //     cell.pos_x = it->rightBotx - cell.width;
        //     // }
        //     if((it->rightBotx - it->totalWidth) < cell.width) continue;
        //     // if(cell.pos_x < nodes[it->cells.back()].legal_x) cell.pos_x = nodes[it->cells.back()].legal_x + nodes[it->cells.back()].width;
        //     PlaceRow(*it);
        //     double cost = sqrt(pow((cell.legal_x - cell.pos_x),2) + pow((cell.legal_y - cell.pos_y),2));
        //     if(cost < cBest) {
        //         cBest = cost;
        //         rowBest = rowInd;
        //     }
        //     it->cells.pop_back();
        //     rowInd++;
        //     it->clusters.clear();
        // }
        Row& bestRow = rows[rowBest];
        bestRow.cells.push_back(i);
        cell.legal_y = bestRow.coordinate;
        if(cell.pos_x < bestRow.subRowOrigin) cell.pos_x = bestRow.subRowOrigin;
        if((cell.pos_x + cell.width) > bestRow.rightBotx) cell.pos_x = bestRow.rightBotx - cell.width;
        PlaceRow(bestRow);
        bestRow.totalWidth += cell.width;
    }
}


int main(int argc, char** argv) 
{
    if(argc != 2) {
        printf("Please provide aux file\n");
        return 1;
    }
    string auxFile = argv[1];
    file_parser(auxFile);
    checkMacro();
    sort(rows.begin(),rows.end(),cmp_row);
    for(int i = 0; i < rows.size(); ++i) {
        rowHeight[rows[i].coordinate].insert(i);
    }
    sort(nodes.begin(), nodes.end(), cmp_node);
    // for(auto it = rows.begin(); it != rows.end(); ++it) 
    //     cout  << " " << it->coordinate << " " << it->height << " " << it->siteSpacing << " " << it->subRowOrigin << " " << it->numSites  << " " << it->rightBotx << "\n"; 
    // for(auto it = nodes.begin(); it != nodes.end(); ++it) {
    //     cout  << it->width << " " << it->height << " " << it->pos_x << " " << it->pos_y << "\n";
    // }
    Abacus();

    

    for(int i = 0; i < nodes.size(); ++i) {
        nodeHash[nodes[i].name] = i;
    }

    ifstream finPL(plFile);
    ofstream fout("output.pl");
    string line;
    getline(finPL,line);
    fout << line << "\n";
    while(getline(finPL,line)) {
        size_t found = line.find("#");
        if(found == 0 || line.empty())  {
            fout << line << "\n";
            continue;
        }
        istringstream iss(line);
        string name;

        iss >> name;
        size_t find_terminal = line.find("FIXED");
        size_t find_terminal_NI = line.find("FIXED_NI");
        if(find_terminal_NI != string::npos) fout << line << "\n";
        else if(find_terminal != string::npos) fout << line << "\n";
        else {
            int nodeIdx = nodeHash[name];
            fout << name << " " << nodes[nodeIdx].legal_x << " " << nodes[nodeIdx].legal_y << " : N\n"; 
        }
    }
    finPL.close();
    fout.close();

    return 0;
}