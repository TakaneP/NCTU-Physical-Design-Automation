#include<iostream>
#include<fstream>
#include<set>
#include<vector>
#include<list>
#include<map>
#include<string>
#include<sstream>
#include<random>
#include<ctime>
using namespace std;

random_device rd;
default_random_engine gen = default_random_engine(rd());
uniform_real_distribution<> dis(0.0,1.0);
unsigned int netNb,nodeNb;
long pMax = 0;
vector<set<unsigned int>> netList;
vector<set<unsigned int>> cellArray;
vector<int> cellGain;
vector<bool> cellGroup;
set<unsigned int> A,B;
vector<unsigned int> switchCells;
//cell is free or not
vector<bool> bitmap;
map<int,set<unsigned int>> bucketA,bucketB;
//netList number in left/right
vector<unsigned int> netListLeftNb, netListRightNb;
int maxGainA= 0, maxGainB = 0;
long nonLockedA = 0, nonLockedB = 0,sizeA = 0,sizeB = 0;

void read_file(string fileName) {
	ifstream fin(fileName);
	string line;
	getline(fin,line);
	istringstream iss(line);
	iss >> netNb >> nodeNb;
	netList.resize(netNb);
	netListLeftNb.resize(netNb,0);
	netListRightNb.resize(netNb,0);
	cellArray.resize(nodeNb);	
	cellGroup.resize(nodeNb);
	bitmap.resize(nodeNb,false);

	unsigned int node;

	for(int i = 0; i < netNb; ++i) {
		getline(fin,line);
		istringstream iss(line);
		while(iss >> node) {
			node--;
			pMax++;
			netList[i].insert(node);
			cellArray[node].insert(i);
			if(!bitmap[node]) {
				if((dis(gen) < 0.5 && sizeA < (0.5 * nodeNb)) || sizeB >= (0.5 * nodeNb)/*node < 0.5*nodeNb*/) {
					A.insert(node);				
					cellGroup[node] = 0;
					// nonLockedA++;
					 sizeA++;
				}
				else {
					B.insert(node);
					cellGroup[node] = 1;
					// nonLockedB++;
					 sizeB++;
				}
			}
			if(cellGroup[node] == 0) netListLeftNb[i]++;
			else netListRightNb[i]++;
			bitmap[node] = true;
		}
	}
	fin.close();
}

void initial_gain() {
	switchCells.clear();

	cellGain.clear();
	cellGain.resize(nodeNb);
	
	bitmap.clear();
	bitmap.resize(nodeNb,0);
	
	bucketA.clear();
	bucketB.clear();
	maxGainA = -pMax;
	maxGainB = -pMax;

	sizeA = A.size();
	nonLockedA = A.size();
	sizeB = B.size();
	nonLockedB = B.size();
	

	bool left;
	int gain;
	// i => cell, iterate cells
	for(unsigned int i = 0; i < (nodeNb); ++i) {
		gain = 0;
		// check in set A or B
		left  = (cellGroup[i] == 0)? true : false;
		// *it => net, iterate nets that connecting this cell, use netlist left/right num to calculate gain
		for(auto it = cellArray[i].begin(); it != cellArray[i].end(); ++it) {
			// cout << "Initail netNb "<< *it << " " << netListLeftNb[*it] << " " << netListRightNb[*it] << endl;
			if(left) {
				if(netListLeftNb[*it] == 1) 
					gain++;
				
				else if(netListRightNb[*it] == 0)
					gain--;		
				
				// cout << "Left Gain" << gain << endl;
			}
			else {
				if(netListRightNb[*it] == 1)
					gain++;
				else if(netListLeftNb[*it] == 0)
					gain--;		
				// cout << "Right Gain" << gain << endl;
			}
		}
		
		if(left) {
			maxGainA = max(maxGainA,gain);
			bucketA[gain].insert(i);
			cellGain[i] = gain;
		}
		else {
			maxGainB = max(maxGainB,gain);
			bucketB[gain].insert(i);
			cellGain[i] = gain;
		}
	}
	// cout << "Max Gain" << maxGainA << " " << maxGainB << endl;
}

void remove_A() {
	unsigned int selectedCell = *bucketA[maxGainA].begin();
	bitmap[selectedCell] = true; //set moved cell locked
	bucketA[maxGainA].erase(bucketA[maxGainA].begin());
	//iterate netlist connecting to the selected cell
	for(auto it = cellArray[selectedCell].begin(); it != cellArray[selectedCell].end(); ++it) {
		unsigned int currentNet = *it;
		unsigned int beforeMoveLeft = netListLeftNb[currentNet];
		unsigned int beforeMoveRight = netListRightNb[currentNet];
		netListLeftNb[currentNet]--;
		netListRightNb[currentNet]++;
		//Update cell gains before move
		//iterate cells on this netlist
		for(auto _it = netList[*it].begin(); _it != netList[*it].end(); ++_it) {
			unsigned int currentCell = *_it;
			if(bitmap[currentCell]) continue; //if cell is locked
			//increase gain on left if right is empty
			if(beforeMoveRight == 0) {
				bucketA[cellGain[currentCell]].erase(currentCell);
				if(cellGain[currentCell] == maxGainA) maxGainA++;
				cellGain[currentCell]++;
				bucketA[cellGain[currentCell]].insert(currentCell);
			
				
			} // decrease gain of the only right cell
			else if(beforeMoveRight == 1 && cellGroup[currentCell] == 1) {
				bucketB[cellGain[currentCell]].erase(currentCell);
				if(cellGain[currentCell] == maxGainB && bucketB[cellGain[currentCell]].empty()) maxGainB--;
				cellGain[currentCell]--;
				bucketB[cellGain[currentCell]].insert(currentCell);
				

			}
			// decrease all cells on the right if left = 0 after moving
			if(netListLeftNb[currentNet] == 0) {
				bucketB[cellGain[currentCell]].erase(currentCell);
				if(cellGain[currentCell] == maxGainB && bucketB[cellGain[currentCell]].empty()) maxGainB--;
				cellGain[currentCell]--;
				bucketB[cellGain[currentCell]].insert(currentCell);
				
			} // increase only cell on left after moving if left = 1 afer moving
			else if(netListLeftNb[currentNet] == 1 && cellGroup[currentCell] == 0) {
				bucketA[cellGain[currentCell]].erase(currentCell);
				if(cellGain[currentCell] == maxGainA) maxGainA++;
				cellGain[currentCell]++;
				bucketA[cellGain[currentCell]].insert(currentCell);
				
			}
		}
	}
	//update data structures
	cellGroup[selectedCell] = 1;
	switchCells.push_back(selectedCell);
	sizeA--;
	sizeB++;
	nonLockedA--;
	while(bucketA[maxGainA].empty() && nonLockedA != 0) {
		// cout << nonLockedA << " " << maxGainA << endl;
		maxGainA--;
	}
	if(nonLockedA == 0) {
		maxGainA = -pMax-1;
	}
}

void remove_B() {
	unsigned int selectedCell = *bucketB[maxGainB].begin();
	bitmap[selectedCell] = true; //set moved cell locked
	bucketB[maxGainB].erase(bucketB[maxGainB].begin());
	//iterate netlist connecting to the selected cell
	for(auto it = cellArray[selectedCell].begin(); it != cellArray[selectedCell].end(); ++it) {
		unsigned int currentNet = *it;
		unsigned int beforeMoveLeft = netListLeftNb[currentNet];
		unsigned int beforeMoveRight = netListRightNb[currentNet];
		netListLeftNb[currentNet]++;
		netListRightNb[currentNet]--;
		//Update cell gains before move
		//iterate cells on this netlist
		for(auto _it = netList[*it].begin(); _it != netList[*it].end(); ++_it) {
			unsigned int currentCell = *_it;
			if(bitmap[currentCell]) continue; //if cell is locked
			//increase gain on right if left is empty
			if(beforeMoveLeft == 0) {
				bucketB[cellGain[currentCell]].erase(currentCell);
				if(cellGain[currentCell] == maxGainB) maxGainB++;
				cellGain[currentCell]++;
				bucketB[cellGain[currentCell]].insert(currentCell);
			
			} // decrease gain of the only left cell
			else if(beforeMoveLeft == 1 && cellGroup[currentCell] == 0) {
				bucketA[cellGain[currentCell]].erase(currentCell);
				if(cellGain[currentCell] == maxGainA && bucketA[cellGain[currentCell]].empty()) maxGainA--;
				cellGain[currentCell]--;
				bucketA[cellGain[currentCell]].insert(currentCell);
				
			}
			// decrease all cells on the left if right = 0 after moving
			if(netListRightNb[currentNet] == 0) {
				bucketA[cellGain[currentCell]].erase(currentCell);
				if(cellGain[currentCell] == maxGainA && bucketA[cellGain[currentCell]].empty()) maxGainA--;
				cellGain[currentCell]--;
				bucketA[cellGain[currentCell]].insert(currentCell);
				
			} // increase only cell on right after moving if right = 1 after moving
			else if(netListRightNb[currentNet] == 1 && cellGroup[currentCell] == 1) {
				bucketB[cellGain[currentCell]].erase(currentCell);
				if(cellGain[currentCell] == maxGainB) maxGainB++;
				cellGain[currentCell]++;
				bucketB[cellGain[currentCell]].insert(currentCell);
				
			}
		}
	}

	//update data structures
	cellGroup[selectedCell] = 0;
	switchCells.push_back(selectedCell);
	sizeB--;
	sizeA++;
	nonLockedB--;
	while(bucketB[maxGainB].empty() && nonLockedB != 0) {
		// cout << nonLockedB << " " << maxGainB << endl;
		maxGainB--;
	}
	if(nonLockedB == 0) {
		maxGainB = -pMax-1;
	}
}


void FM() {
	time_t start,end;
	double onePassTIme;
	double diff;
	bool flag = false;
	start = time(NULL);
	while(1) {
		// for(auto it = cellGroup.begin();it != cellGroup.end();++it) {
		// 	cout << *it << " ";
		// }
		// cout << "\n";
		initial_gain();	
		long maxGainDelta = 0,totalGainDelta = 0;
		vector<long> gainDelta(nodeNb);
		int pos = 0;
		for(int i = 0; i < nodeNb; i++) {
			// cout << i;
			
			 // cout << "Non locked " << nonLockedA << " " << nonLockedB << "\n";
			 // cout << "Max gain " << maxGainA << " " << maxGainB << "\n";
			 // cout << "Size " << sizeA << " " << sizeB <<"\n";
			// Remove A still doesn't violate area constraint 
			if(/*((maxGainB >= maxGainA || (sizeA - 1) <= 0.45*nodeNb) && nonLockedB > 0 && (sizeA+1) <= 0.55*nodeNb) || nonLockedA == 0*/maxGainB >= maxGainA) {
				if(((sizeA+1) <= 0.55*nodeNb && (sizeB-1) >= 0.45*nodeNb && nonLockedB > 0) || nonLockedA == 0) {
					// cout << "B" << endl;
					remove_B();
				}
				else {
					// cout << "A" << endl;
					remove_A();
				}
				
			} //Remove B
			else {
				if(((sizeB+1) <= 0.55*nodeNb && (sizeA-1) >= 0.45*nodeNb && nonLockedA > 0) || nonLockedB == 0) {
					// cout << "A" << endl;
					remove_A();		
				}
				else {
					// cout << "B" << endl;
					remove_B();
				}
			}
			// cout << "Switch cell " << switchCells.back() << "\n";
			// cout << "Gain " << cellGain[switchCells.back()] << endl;
			totalGainDelta += cellGain[switchCells.back()];
			// cout << "totalGainDelta " << totalGainDelta << "\n";
			if(totalGainDelta > maxGainDelta) {
				maxGainDelta = totalGainDelta;
				pos = i;
			}
		}
		// cout << maxGainDelta << endl;
		if(maxGainDelta <= 0) return;
		else {
			for(int i = 0; i < nodeNb; ++i)  {
				// cout << "k " << pos << endl;
				if(i <= pos) {
					// cout << "y";
					if(cellGroup[switchCells[i]] == 1) {
						A.erase(switchCells[i]);
						B.insert(switchCells[i]);
					}
					else {
						B.erase(switchCells[i]);
						A.insert(switchCells[i]);
					}
				}
				else {
					//revert change
					cellGroup[switchCells[i]] = (cellGroup[switchCells[i]])? 0:1;
					for(auto it = cellArray[switchCells[i]].begin(); it != cellArray[switchCells[i]].end(); ++it) {
						// cout << "after netNb " << *it << " "<< netListLeftNb[*it] << " " << netListRightNb[*it] << endl;
						if(cellGroup[switchCells[i]] == 0) {
							netListRightNb[*it]--;
							netListLeftNb[*it]++;
						}
						else {
							netListRightNb[*it]++;
							netListLeftNb[*it]--;
						}
					}
					
				}
			}
		}
		end = time(NULL);
		diff = difftime(end,start);
		if(!flag) {
			onePassTIme = diff + 5.0;
			flag = true;
		}
		if((30.0-diff) < onePassTIme ) {
			// cout << diff << "\n";
			return;
		}
	}
}

int main(int argc, char const *argv[])
{
	if(argc != 2) {
		printf("Set input file as argument 1\n");
	}
	string fileName = argv[1];
	read_file(fileName);
	FM();
	ofstream fout("output.txt");
	for(int i = 0;i < nodeNb; ++i) {
		if(cellGroup[i] == 0) fout << "0" << "\n";
		else fout << "1" << "\n";
	}
	fout.close();
	// for(auto it = bucketA.begin(); it != bucketA.end(); ++it) {
	// 	cout << it->first << " ";
	// 	for(auto _it = it->second.begin(); _it != it->second.end(); ++_it) {
	// 		cout << *_it << " ";
	// 	}
	// 	cout << endl;
	// }

	// for(auto it = A.begin(); it != A.end(); ++it) {
	// 	cout << *it << " ";
	// }
	// for(auto it = netList.begin(); it != netList.end(); ++it) {
	// 	for(auto _it = it->begin(); _it != it->end(); ++_it) {
	// 		printf("%d ",*_it);
	// 	}
	// 	printf("\n");
	// }

	return 0;
	
}
