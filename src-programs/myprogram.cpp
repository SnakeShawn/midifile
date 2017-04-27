#include "MidiFile.h"
#include "Options.h"
#include <math.h>
#include <algorithm>
#include <iostream>
#include <iomanip>

#define MAX_LEN	99999

using namespace std;

struct LNote{
int key;
int start;
double length;
};

vector<LNote> noteVec;
vector<int> startVec;

int main(int argc, char** argv) {
   Options options;
   options.process(argc, argv);
   MidiFile midifile;
   if (options.getArgCount() > 0) {
      midifile.read(options.getArg(1));
   } else {
      midifile.read(cin);
   }

   int tpq = midifile.getTicksPerQuarterNote();
   midifile.linkNotePairs();
   int tracks = midifile.getTrackCount();
   //cout << "TPQ: " << tpq << endl;
   if (tracks > 1) {
      //cout << "TRACKS: " << tracks << endl;
   }
   //cout << "===================" << endl;

   MidiEvent* mev;
   double duration;

   //cout << "Ticks\tDUR\tTrack\tNote" << endl;
   for (int track=0; track < midifile.getTrackCount(); track++){
	for (int i=0; i < midifile[track].size(); i++){
   		mev = &midifile[track][i];
		MidiEvent& data = midifile[track][i];
//		if (midifile[track][i][0] == 0xff && midifile[track][i][1] == 0x51){
		if (data[0] == 0xff){
			if (data[1] == 0x51){
				int microseconds = 0;
				microseconds = microseconds | (midifile[track][i][3] << 16);
				microseconds = microseconds | (midifile[track][i][4] << 8);
				microseconds = microseconds | (midifile[track][i][5] << 0);
				double tempo = 60.0/microseconds * 1000000.0;
				//cout << tempo << endl;
			}
			else if (data[1] == 0x58){
				//cout<<"meter\t"<<(int)data[2]<<'/'<<pow(2.0,data[3])<<endl;
			}
		}
		if(!mev->isNoteOn()){
			continue;		
		}
		duration = mev->getTickDuration();
		//cout << mev->tick << '\t' << duration << '\t' << mev->track << '\t' 
//			<< mev->getKeyNumber() << endl;
		
		noteVec.push_back({mev->getKeyNumber(), mev->tick, duration});		

		vector<int>::iterator ret,ret2;
		ret = std::find(startVec.begin(), startVec.end(), mev->tick);
 	        if(ret == startVec.end())
			startVec.push_back(mev->tick);
		ret2 = std::find(startVec.begin(), startVec.end(), mev->tick+duration);
 	        if(ret2 == startVec.end())
			startVec.push_back(mev->tick+duration);

	}
   }

//cout<<"\n%=============================\n";
   sort(startVec.begin(), startVec.end());
   for (int i=0; i<startVec.size(); i++){
	vector<LNote> tmpVec;
	int minLen = MAX_LEN;
	int start = startVec[i];
	int nextStart = -1;
	if (i+1 < startVec.size())
		nextStart = startVec[i+1];
   	for (int j=0; j<noteVec.size(); j++){
		if (start == noteVec[j].start){
			tmpVec.push_back(noteVec[j]);
			if (noteVec[j].length<minLen) minLen = noteVec[j].length; 
			
			//vector<LNote>::iterator it = noteVec.begin()+j;
			//noteVec.erase(it);
		}
	}
	if (nextStart<start+minLen && nextStart!=-1) minLen = nextStart - start; 
	for (int k=0; k<tmpVec.size(); k++){
		cout<<tmpVec[k].key;
		if(k+1!=tmpVec.size())cout<<',';
		LNote ln = {tmpVec[k].key, tmpVec[k].start+minLen, tmpVec[k].length-minLen};
		if (ln.length > 0) noteVec.push_back(ln);
	}
	if(minLen != MAX_LEN && !tmpVec.empty())cout<<'\t'<<minLen<<endl;
	tmpVec.clear();
   }

   return 0;
}

