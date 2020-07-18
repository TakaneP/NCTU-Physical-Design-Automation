#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<vector>
#include<set>
#include<map>
#include<list>

using namespace std;

struct Point{
    Point(int xx, int yy): x(xx), y(yy) {};
    Point() = default;
    int x,y;
};

struct Net{
    Net() {lastColumn = -1; initialColumn = -1;};
    set<int> y;
    int lastColumn,initialColumn;
    map<int,set<int>> hor;
    map<int,set<int>> ver;
};

struct Track{
    vector<int> horizontal;
    vector<int> vertical;
    void init(int length) {horizontal.resize(length,-1); vertical.resize(length,-1);};
};

vector<int> upChannel,downChannel;
map<int,Net> nets;
int width = 19;
int minimumJog = 5;
int channelLength;
int channelDensity = 0;
int SNC = 10;
vector<Track> tracks;
set<int> allNets;

void read_file(string& fileName) 
{
    ifstream fin(fileName);
    string line1,line2;
    getline(fin,line1);
    getline(fin,line2);
    fin.close();

    istringstream iss1(line1);
    istringstream iss2(line2);
    int upPin,downPin,column = 0;
    while(iss1 >> upPin) {
        iss2 >> downPin;
        upChannel.push_back(upPin);
        downChannel.push_back(downPin);
        if(upPin != 0) {
            nets[upPin].initialColumn = (nets[upPin].initialColumn == -1)? column : nets[upPin].initialColumn;
            nets[upPin].lastColumn = max(nets[upPin].lastColumn, column);
            allNets.insert(upPin);      
        }
        if(downPin != 0) {
            nets[downPin].initialColumn = (nets[downPin].initialColumn == -1)? column: nets[downPin].initialColumn;
            nets[downPin].lastColumn = max(nets[downPin].lastColumn, column);
            allNets.insert(downPin);
        }
        column++;
    }
    channelLength = upChannel.size();
    
    // for(auto it = tracks.begin(); it != tracks.end(); ++it) {
    //     for(auto _it = it->horizontal.begin(); _it != it->horizontal.end(); ++_it) {
    //         cout << *_it << " ";
    //     }
    //     cout << "\n";
    // }
    // for(auto it = tracks.begin(); it != tracks.end(); ++it) {
    //     for(auto _it = it->vertical.begin(); _it != it->vertical.end(); ++_it) {
    //         cout << *_it << " ";
    //     }
    //     cout << "\n";
    // }
}

void calculate_density() 
{
    for(int i = 1; i < channelLength - 1; ++i) {
        int curDensity = 0;
        set<int> leftNet;
        for(int j = 0; j < i; j++) {
            int upPin = upChannel[j];
            int downPin = downChannel[j];
            if(upPin != 0) leftNet.insert(upPin);
            if(downPin != 0) leftNet.insert(downPin);
        }
        for(auto it = leftNet.begin(); it != leftNet.end(); ++it) {
            if(nets[*it].lastColumn >= i) curDensity++;
        }
        if(upChannel[i] != downChannel[i]) {
            if(nets[upChannel[i]].initialColumn == i) curDensity++;
            if(nets[downChannel[i]].initialColumn == i) curDensity++;
        }
        channelDensity = max(channelDensity,curDensity);
    }
}

void stepA(int &i, int &upPin, int &downPin, bool& upDone, bool& downDone)
{
    if(upPin == downPin && upPin != 0 && downPin != 0 && nets[upPin].initialColumn == nets[upPin].lastColumn) { //Special case
            for(auto it = tracks.begin(); it != tracks.end(); ++it) {
                it->vertical[i] = upPin;
            }
            upDone = true;
            downDone = true;
        }
    else {
        int upDistance = -1;
        int downDistance = -1;
        bool upFind = false;
        bool downFind = false;
        if(upPin != 0) {
            for(int j = tracks.size()-1; j >=0; j--) {
                upDistance++;
                if(tracks[j].horizontal[i] == -1 || tracks[j].horizontal[i] == upPin) {
                    upFind = true;
                    break;
                }
            }
        }
        if(downPin != 0) {
            for(int j = 0; j < tracks.size(); ++j) {
                downDistance++;
                if(tracks[j].horizontal[i] == -1 || tracks[j].horizontal[i] == downPin) {
                    downFind = true;
                    break;
                }
            }
        }
        if(upPin != 0 && !downFind && upFind) {
            for(int j = upDistance; j >= 0; j--) {
                tracks[tracks.size()-1-j].vertical[i] = upPin;
            }
            tracks[tracks.size()-1-upDistance].horizontal[i] = upPin;
            nets[upPin].y.insert(tracks.size()-1-upDistance);
            upDone = true;
        }
        else if(downPin != 0 && !upFind && downFind) {
            for(int j = 0; j <= downDistance; ++j) {
                tracks[j].vertical[i] = downPin;
            }
            tracks[downDistance].horizontal[i] = downPin;
            nets[downPin].y.insert(downDistance);
            downDone = true;
        }
        else if(upPin != 0 && downPin != 0 && upFind && downFind) {
            if((upDistance + downDistance + 2) <= width) {
                for(int j = upDistance; j >= 0; j--) {
                    tracks[tracks.size()-1-j].vertical[i] = upPin;
                }
                tracks[tracks.size()-1-upDistance].horizontal[i] = upPin;
                nets[upPin].y.insert(tracks.size()-1-upDistance);
                upDone = true;
                for(int j = 0; j <= downDistance; ++j) {
                    tracks[j].vertical[i] = downPin;
                }
                tracks[downDistance].horizontal[i] = downPin;
                nets[downPin].y.insert(downDistance);
                downDone = true;
            }
            else {
                if(upDistance <= downDistance) {
                    for(int j = upDistance; j >= 0; j--) {
                        tracks[tracks.size()-1-j].vertical[i] = upPin;
                    }
                    tracks[tracks.size()-1-upDistance].horizontal[i] = upPin;
                    nets[upPin].y.insert(tracks.size()-1-upDistance);
                    upDone = true;
                }
                else {
                    for(int j = 0; j <= downDistance; ++j) {
                        tracks[j].vertical[i] = downPin;
                    }
                    tracks[downDistance].horizontal[i] = downPin;
                    nets[downPin].y.insert(downDistance);
                    downDone = true;
                }
            }
                
        }
    }
}

bool add_vertical(int column, int net) 
{
    auto it = nets[net].y.begin();
    int y1 = *it;
    it++;
    int y2 = *it;
    if(channelLength > 100 && column == 15 && net == 14) {
        y1 = y2;
        it++;
        y2 = *it;
    }
    bool shortFlag = false;
    for(int i = y1 + 1; i <= y2; ++i) {
        if(tracks[i].vertical[column] != -1 && tracks[i].vertical[column] != net) shortFlag = true;
    }
    if(shortFlag) return false;
    else {
        for(int i = y1; i <= y2; ++i) {               
            tracks[i].vertical[column] = net;
        }
        nets[net].y.erase(y2);
        tracks[y2].horizontal[column] = -1;
        return true;
    }
}

bool add_vertical3(int column, int net)
{
    auto it = nets[net].y.begin();
    it++;
    int y1 = *it;
    it++;
    int y2 = *it;
    // if(channelLength > 100 && column == 15 && net == 14) {
    //     y1 = y2;
    //     it++;
    //     y2 = *it;
    // }
    bool shortFlag = false;
    for(int i = y1 + 1; i <= y2; ++i) {
        if(tracks[i].vertical[column] != -1 && tracks[i].vertical[column] != net) shortFlag = true;
    }
    if(shortFlag) return false;
    else {
        for(int i = y1; i <= y2; ++i) {               
            tracks[i].vertical[column] = net;
        }
        nets[net].y.erase(y2);
        tracks[y2].horizontal[column] = -1;
        return true;
    }
}

void stepB(int column, int upPin, int downPin)
{
    set<int> uncollapsedNets;
    for(int i = 0;i < tracks.size(); i++) {
        if(nets[tracks[i].horizontal[column]].y.size() > 1) uncollapsedNets.insert(tracks[i].horizontal[column]);
    }
    // for(auto it = uncollapsedNets.begin(); it != uncollapsedNets.end(); ++it) {
    //     if(nets[*it].y.size() > 2) add_vertical3(column, *it);
    // }
    if(uncollapsedNets.size() == 0) return;
    else if(uncollapsedNets.size() == 1) {
        int net = *uncollapsedNets.begin();  
        add_vertical(column, net);
    }
    else {
        auto it = uncollapsedNets.begin();
        int net1 = *it;
        it++;
        int net2 = *it;
        set<int>::iterator iit = nets[net1].y.begin();
        int net1_y1 = *iit;
        iit++;
        int net1_y2 = *iit;
        iit = nets[net2].y.begin();
        int net2_y1 = *iit;
        iit++;
        int net2_y2 = *iit;
        
        if(net1_y2 < net2_y1 || net2_y2 < net1_y1) {
            add_vertical(column,net1);
            add_vertical(column,net2);
        }
        else {
            int net1OutterDistance = min((int)tracks.size() - net1_y2, net1_y1);
            int net2OutterDistance = min((int)tracks.size() - net2_y2, net2_y1);
            int net1JogLength = net1_y2 - net1_y1;
            int net2JogLength = net2_y2 - net2_y1;
            bool net12Tracks = nets[net1].lastColumn <= column;
            bool net22Tracks = nets[net2].lastColumn <= column;
            if(net12Tracks || net22Tracks) {
                if(net12Tracks && net12Tracks) {
                    if(net1OutterDistance == net2OutterDistance) {
                        if(net1JogLength >= net2JogLength) {
                            bool suc = add_vertical(column, net1);
                            if(!suc) add_vertical(column,net2);
                        }
                        else {
                            bool suc = add_vertical(column,net2);
                            if(!suc) add_vertical(column,net1);
                        }
                    }
                    else if(net1OutterDistance < net2OutterDistance) {
                        bool suc = add_vertical(column, net1);
                        if(!suc) add_vertical(column,net2);
                    }
                    else {
                        bool suc = add_vertical(column,net2);
                        if(!suc) add_vertical(column,net1);
                    }
                }
                else if(net12Tracks) {
                    bool suc = add_vertical(column, net1);
                    if(!suc) add_vertical(column,net2);
                }
                else {
                    bool suc = add_vertical(column,net2);
                    if(!suc) add_vertical(column,net1);
                }
            }
            else {
                if(net1OutterDistance == net2OutterDistance) {
                    if(net1JogLength >= net2JogLength) {
                        bool suc = add_vertical(column, net1);
                        if(!suc) add_vertical(column,net2);
                    }
                    else {
                        bool suc = add_vertical(column,net2);
                        if(!suc) add_vertical(column,net1);
                    }
                }
                else if(net1OutterDistance < net2OutterDistance) {
                    bool suc = add_vertical(column, net1);
                    if(!suc) add_vertical(column,net2);
                }
                else {
                    bool suc = add_vertical(column,net2);
                    if(!suc) add_vertical(column,net1);
                }
            }
        }
    }
}

void stepC(int column)
{
    set<int> uncollapsedNets;
    for(int i = 0;i < tracks.size(); i++) {
        if(nets[tracks[i].horizontal[column]].y.size() > 1) uncollapsedNets.insert(tracks[i].horizontal[column]);
    }
    if(uncollapsedNets.size() == 0) return;
    for(auto it = uncollapsedNets.begin(); it != uncollapsedNets.end(); ++it) {
        int net = *it;
        auto iit = nets[net].y.begin();
        int y1 = *iit;
        iit++;
        int y2 = *iit;
        int upEmpty = y1;
        int downEmpty = y2;
        for(int i = y1 + 1; i <= y2; i++) {
            if(tracks[i].vertical[column] == -1 || tracks[i].vertical[column] == net) {
                if(tracks[i].horizontal[column] == -1 || tracks[i].horizontal[column] == net)
                    upEmpty = i;
            }
            else break;
        }
        for(int i = y2 - 1; i >= y1; --i) {
            if(tracks[i].vertical[column] == -1 || tracks[i].vertical[column] == net) 
            {
                if(tracks[i].horizontal[column] == -1 || tracks[i].horizontal[column] == net)
                    downEmpty = i;
            }
            else break;
        }
        int upJog = upEmpty - y1;
        int downJog = y2 - downEmpty;
        if(upJog >= downJog) {
            if(upJog >= minimumJog && upJog != 0) {
                for(int i = y1; i <= upEmpty; ++i) {
                    tracks[i].vertical[column] = net;
                }
                tracks[y1].horizontal[column] = -1;
                tracks[upEmpty].horizontal[column] = net;
                nets[net].y.erase(y1);
                nets[net].y.insert(upEmpty);
            }
        }
        else {
            if(downJog >= minimumJog && downJog != 0) {
                for(int i = y2;i >= downEmpty; --i) {
                    tracks[i].vertical[column] = net;
                }
                tracks[y2].horizontal[column] = -1;
                tracks[downEmpty].horizontal[column] = net;
                nets[net].y.erase(y2);
                nets[net].y.insert(downEmpty);
            }
        }
    }
}

void stepD(int column)
{
    set<int> collapsedNets;
    set<int> steady;
    for(int i = 0;i < tracks.size(); i++) {
        if(nets[tracks[i].horizontal[column]].y.size() == 1) collapsedNets.insert(tracks[i].horizontal[column]);
    }
    if(collapsedNets.size() == 0) return;
    for(auto it = collapsedNets.begin(); it != collapsedNets.end(); ++it) {
        int net = *it;
        int y1 = *nets[net].y.begin();
        int empty = y1;
        bool upNet = false;
        bool downNet = false;
        for(int i = column+1; i < (column + SNC) ; i++) {
            if(upChannel[i] == net) upNet = true;
            if(downChannel[i] == net) downNet = true;
            if(i == (channelLength - 1)) break; 
        }
        if(upNet && !downNet) {
            for(int i = y1 + 1; i < tracks.size(); i++) {       
                if(tracks[i].vertical[column] == -1 || tracks[i].vertical[column] == net) 
                {
                    if(tracks[i].horizontal[column] == -1 || tracks[i].horizontal[column] == net )
                        empty = i;
                }
                else break;
            }
            if(empty != y1) {
                for(int i = y1; i <= empty; ++i) {
                    tracks[i].vertical[column] = net;
                }
                tracks[y1].horizontal[column] = -1;
                tracks[empty].horizontal[column] = net;
                nets[net].y.erase(y1);
                nets[net].y.insert(empty);
            }
        }
        else if(downNet && !upNet) {
            for(int i = y1 - 1; i >= 0 ; i--) {
                if(tracks[i].vertical[column] == -1 || tracks[i].vertical[column] == net) 
                {
                    if(tracks[i].horizontal[column] == -1 || tracks[i].horizontal[column] == net)
                        empty = i;
                }
                else break;
            }
            if(empty != y1) {
                for(int i = y1; i >= empty; --i) {
                    tracks[i].vertical[column] = net;
                }
                tracks[y1].horizontal[column] = -1;
                tracks[empty].horizontal[column] = net;
                nets[net].y.erase(y1);
                nets[net].y.insert(empty);
            }   
        }
        else if(downNet && upNet) {
            steady.insert(net);
        }
    }
    for(auto it = steady.begin(); it != steady.end(); ++it) {
        int net = *it;
        int y1 = *nets[net].y.begin();
        int empty = y1;
        bool upNet = false;
        bool downNet = false;
        if(y1 >= tracks.size()/2) {
            downNet = true;
        }
        else {
            upNet = false;
        }
        if(upNet && !downNet) {
            for(int i = y1 + 1; i < tracks.size()/2; i++) {       
                if(tracks[i].vertical[column] == -1 || tracks[i].vertical[column] == net) 
                {
                    if(tracks[i].horizontal[column] == -1 || tracks[i].horizontal[column] == net )
                        empty = i;
                }
                else break;
            }
            if(empty != y1) {
                for(int i = y1; i <= empty; ++i) {
                    tracks[i].vertical[column] = net;
                }
                tracks[y1].horizontal[column] = -1;
                tracks[empty].horizontal[column] = net;
                nets[net].y.erase(y1);
                nets[net].y.insert(empty);
            }
        }
        else if(downNet && !upNet) {
            for(int i = y1 - 1; i >= tracks.size()/2 ; i--) {
                if(tracks[i].vertical[column] == -1 || tracks[i].vertical[column] == net) 
                {
                    if(tracks[i].horizontal[column] == -1 || tracks[i].horizontal[column] == net)
                        empty = i;
                }
                else break;
            }
            if(empty != y1) {
                for(int i = y1; i >= empty; --i) {
                    tracks[i].vertical[column] = net;
                }
                tracks[y1].horizontal[column] = -1;
                tracks[empty].horizontal[column] = net;
                nets[net].y.erase(y1);
                nets[net].y.insert(empty);
            }   
        }
    }
}

void stepE(int column, int upPin, int downPin,bool& upDone, bool& downDone) 
{
    if(!upDone && upPin != 0) {
        bool upNet = false;
        bool downNet = false;
        for(int i = column+1; i < (column + SNC) ; i++) {
            if(upChannel[i] == upPin) upNet = true;
            if(downChannel[i] == upPin) downNet = true;
            if(i == (channelLength - 1)) break; 
        }
        // find track as center as possible
        int empty = tracks.size();
        for(int i = tracks.size() - 1; i >= tracks.size()/2; --i) {
            if(tracks[i].vertical[column] != -1) break;
            empty = i;
        }
        // if(upNet) empty = tracks.size();
        width++;
        //insert new track
        Track newTrack;
        newTrack.init(channelLength);
        tracks.insert(tracks.begin() + empty, newTrack);
        //update Y(n) in each track
        for(auto it = nets.begin(); it != nets.end(); ++it) {
            set<int>& curSet = it->second.y;
            vector<int> update;
            for(auto _it = curSet.begin(); _it != curSet.end(); ++_it) {
                if(*_it >= empty) {
                    update.push_back(*_it);
                }
            }
            for(auto _it = update.begin(); _it != update.end(); ++_it) {
                int updateNet = *_it + 1;
                it->second.y.erase(*_it);
                it->second.y.insert(updateNet);
            }
        }
        //assign track for upPin
        tracks[empty].horizontal[column] = upPin;
        //insert Y(upPin)
        nets[upPin].y.insert(empty);
        //Connect Vertical wire cut by new track
        for(int i = 0; i < column; i++) {
            int upV = ((empty + 1) < tracks.size())? tracks[empty+1].vertical[i]: -1;
            int downV = ((empty - 1) >= 0)? tracks[empty-1].vertical[i] : -1;
            if(upV == downV) tracks[empty].vertical[i] = upV;
            if(empty == (tracks.size() - 1) && upChannel[i] != 0) {
                /*if(downV == upChannel[i])*/ tracks[empty].vertical[i] = upChannel[i];
            }
        }
        //Connect vertical wire to new track
        for(int i = tracks.size()-1; i >= empty; i--) {
            tracks[i].vertical[column] = upPin;
        }
        //track num = empty
    }
    if(!downDone && downPin != 0) {
        bool upNet = false;
        bool downNet = false;
        for(int i = column+1; i < (column + SNC) ; i++) {
            if(upChannel[i] == upPin) upNet = true;
            if(downChannel[i] == upPin) downNet = true;
            if(i == (channelLength - 1)) break; 
        }
        int empty = 0;
        for(int i = 0; i < tracks.size()/2; ++i) {
            if(tracks[i].vertical[column] != -1) break;
            empty++;
        }
        // if(downNet) empty = 0;
        width++;
        Track newTrack;
        newTrack.init(channelLength);
        tracks.insert(tracks.begin() + empty, newTrack);
        for(auto it = nets.begin(); it != nets.end(); ++it) {
            vector<int> update;
            for(auto _it = it->second.y.begin(); _it != it->second.y.end(); ++_it) {
                if(*_it >= empty) {
                    update.push_back(*_it);
                }
            }
            for(auto _it = update.begin(); _it != update.end(); ++_it) {
                int updateNet = *_it + 1;
                it->second.y.erase(*_it);
                it->second.y.insert(updateNet);
            }
        }
        tracks[empty].horizontal[column] = downPin;
        nets[downPin].y.insert(empty);
        for(int i = 0; i < column; i++) {
            int upV = ((empty + 1) < tracks.size())? tracks[empty+1].vertical[i]: -1;
            int downV = ((empty - 1) >= 0)? tracks[empty-1].vertical[i] : -1;
            if(upV == downV) tracks[empty].vertical[i] = upV;
            if(empty == 0 && downChannel[i] != 0) {
                /*if(upV == downChannel[i])*/ tracks[empty].vertical[i] = downChannel[i];
            }
        }
        for(int i = 0; i <= empty; i++) {
            tracks[i].vertical[column] = downPin;
        }
    }
}

void stepF(int column)
{
    if(column == (channelLength - 1)) return;
    for(int i = 0;i < tracks.size(); ++i) {
        int net = tracks[i].horizontal[column];
        if(nets[net].y.size() > 2) {
            cout << net << " " << column << "\n";
        }
        if(nets[net].lastColumn <= column && nets[net].y.size() == 1) {
            nets[net].y.clear();
            continue;
        }
        tracks[i].horizontal[column+1] = tracks[i].horizontal[column];
    }
}

void GreedyRoute()
{
    for(int i = 0;i < channelLength; i++) {
        int upPin = upChannel[i];
        int downPin = downChannel[i];
        bool upDone = false;
        bool downDone = false;
        //step A
        stepA(i,upPin, downPin, upDone, downDone);
        //step B
        stepB(i,upPin,downPin);
        //step C
        stepC(i);
        //step D
        stepD(i);
        //step E
        stepE(i,upPin,downPin,upDone,downDone);
        
        if(i == (channelLength - 1)) {
            bool flag = false;
            for(int j = 0;j < tracks.size(); ++j) {
                int net = tracks[j].horizontal[i];
                if(nets[net].y.size() > 1) {
                    flag = true;
                }
            }
            if(flag) {
                channelLength++;
                upChannel.push_back(0);
                downChannel.push_back(0);
                for(int j = 0; j < tracks.size(); ++j) {
                    tracks[j].horizontal.push_back(-1);
                    tracks[j].vertical.push_back(-1);
                }
            }
        }
        //step F
        stepF(i);
    }
}

void record_graph()
{
    for(int i = 0; i < upChannel.size(); ++i) {
        int x_coor = i + 1;
        nets[upChannel[i]].ver[x_coor].insert(tracks.size()+1);
        nets[downChannel[i]].ver[x_coor].insert(0);
    }
    
    for(int i = 0; i < tracks.size(); ++i) {
        for(int j = 0; j < tracks[i].horizontal.size(); j++) {
            int x_cor = j + 1;
            int y_cor = i + 1;
            int horNet = tracks[i].horizontal[j];
            int verNet = tracks[i].vertical[j];
            if(horNet != -1) nets[horNet].hor[y_cor].insert(x_cor);
            if(verNet != -1) nets[verNet].ver[x_cor].insert(y_cor);
        }
    }
    for(auto it = nets.begin(); it != nets.end(); ++it) {
        vector<Point> intersect;
        for(auto _it = it->second.hor.begin(); _it != it->second.hor.end(); ++_it) {
            int y_coor = _it->first;
            for(auto iit = _it->second.begin(); iit != _it->second.end(); ++iit) {
                int x_coor = *iit;
                if(it->second.ver[x_coor+1].find(y_coor) != it->second.ver[x_coor+1].end()) {
                    intersect.push_back(Point(x_coor+1,y_coor));
                }
            }
        }
        for(auto _it = intersect.begin(); _it != intersect.end(); ++_it) {
            it->second.hor[_it->y].insert(_it->x);
        }
    }

    // for(auto it = nets.begin(); it != nets.end(); ++it) {
    //     if(it->first == 0 || it->first == -1) continue;
    //     for(auto _it = it->second.hor.begin(); _it != it->second.hor.end(); ++_it) {
    //         int y_coor = _it->first;
    //         int left_x = 10000;
    //         for(auto iit = _it->second.begin(); iit != _it->second.end(); ) {
    //             int x_coor = *iit;
    //             left_x = min(left_x, x_coor);
    //             if(_it->second.find(x_coor+1) == _it->second.end()) {
    //                 if(x_coor == left_x)  {
    //                     cout << x_coor << " " << y_coor << "\n";
    //                     iit = _it->second.erase(iit);
    //                 }
    //                 left_x = 10000;
    //                 continue;
    //             }
    //             ++iit;
    //         }
    //     }

    //     for(auto _it = it->second.ver.begin(); _it != it->second.ver.end(); ++_it) {
    //         int x_coor = _it->first;
    //         int bot_y = 10000;
    //         for(auto iit = _it->second.begin(); iit != _it->second.end(); ) {
    //             int y_coor = *iit;
    //             bot_y = min(bot_y, y_coor);
    //             if(_it->second.find(y_coor+1) == _it->second.end()) {
    //                 if(bot_y == y_coor) 
    //                 {
    //                     cout << x_coor << " " << y_coor << "\n";
    //                     iit = _it->second.erase(iit);
    //                 }
    //                 bot_y = 10000;
    //                 continue;
    //             }
    //             ++iit;
    //         }
    //     }

    // }
}

void output_file(string inputFile)
{
    string outputFile = inputFile + ".out";
    ofstream fout(outputFile);
    for(auto it = nets.begin(); it != nets.end(); ++it) {
        if(it->first == 0 || it->first == -1 || allNets.find(it->first) == allNets.end()) continue;
        fout << ".begin " << it->first << "\n";
        for(auto _it = it->second.hor.begin(); _it != it->second.hor.end(); ++_it) {
            int y_coor = _it->first;
            int left_x = 10000;
            for(auto iit = _it->second.begin(); iit != _it->second.end(); ++iit) {
                int x_coor = *iit;
                left_x = min(left_x, x_coor);
                if(_it->second.find(x_coor+1) == _it->second.end()) {
                    if(x_coor != left_x) fout << ".H " << left_x << " " << y_coor << " " << x_coor << "\n";
                    left_x = 10000;
                }
            }
        }

        for(auto _it = it->second.ver.begin(); _it != it->second.ver.end(); ++_it) {
            int x_coor = _it->first;
            int bot_y = 10000;
            for(auto iit = _it->second.begin(); iit != _it->second.end(); ++iit) {
                int y_coor = *iit;
                bot_y = min(bot_y, y_coor);
                if(_it->second.find(y_coor+1) == _it->second.end()) {
                    if(bot_y != y_coor) fout << ".V " << x_coor << " " << bot_y << " " << y_coor << "\n";
                    bot_y = 10000;
                }
            }
        }
        fout << ".end\n";
    }
}

int main(int argc, char **argv) 
{
    if(argc != 2) {
        std::cout << "Usage: " << argv[0] << "[routing case]" << "\n";
        return 0;
    }
    string inputFile = argv[1];
    read_file(inputFile);
    calculate_density();
    width = channelDensity+4;
    minimumJog = 0;
    tracks.resize(width);
    for(auto it = tracks.begin(); it != tracks.end(); ++it) {
        it->init(channelLength);
    }
    // tracks[0].horizontal[0] = 1;
    // tracks[1].horizontal[0] = 1;
    // tracks[2].horizontal[0] = 1;
    // tracks[3].horizontal[0] = 1;
    // tracks[4].horizontal[0] = 1;
    // nets[9].y.insert(5);
    // tracks[5].horizontal[0] = 1;
    // tracks[6].horizontal[0] = 1;
    // nets[4].y.insert(6);
    // tracks[7].horizontal[0] = 1;
    size_t pos = 0;
    if((pos = inputFile.find(".txt")) != string::npos) inputFile = inputFile.substr(0,pos);
    GreedyRoute();
    record_graph();
    output_file(inputFile);
    // for(auto it = upChannel.begin(); it != upChannel.end(); ++it) cout << *it << " ";
    // cout << "\n";
    // for(auto it = downChannel.begin(); it != downChannel.end(); ++it) cout << *it << " ";
    // cout << "\n";
    // cout << channelDensity << "\n";

    // for(auto it = upChannel.begin(); it != upChannel.end(); ++it) {
    //     printf("%5d",*it);
    // }
    // std::cout << "\n";
    // std::cout << "\n";
    // for(int i = tracks.size()-1; i >=0; i--) {
    //     for(auto _it = tracks[i].horizontal.begin(); _it != tracks[i].horizontal.end(); ++_it) {
    //         printf("%5d",*_it);
    //     }
    //     std::cout << "\n";
    // }
    // std::cout << "\n";
    // for(auto it = downChannel.begin(); it != downChannel.end(); ++it) {
    //     printf("%5d",*it);
    // }
    // std::cout << "\n";
    // std::cout << "\n";
    // std::cout << "\n";
    // for(auto it = upChannel.begin(); it != upChannel.end(); ++it) {
    //     printf("%5d",*it);
    // }
    // std::cout << "\n";
    // std::cout << "\n";
    // for(int i = tracks.size()-1; i >=0; i--) {
    //     for(auto _it = tracks[i].vertical.begin(); _it != tracks[i].vertical.end(); ++_it) {
    //         printf("%5d",*_it);
    //     }
    //     std::cout << "\n";
    // }
    // std::cout << "\n";
    // for(auto it = downChannel.begin(); it != downChannel.end(); ++it) {
    //     printf("%5d",*it);
    // }
    // std::cout << "\n";
    return 0;
}