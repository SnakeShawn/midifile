#include "Options.h"
#include "MidiFile.h"
#include <iostream>
#include <algorithm>

#define MAX_LEN 999999

using namespace std;
struct LNote{
	int key;
	int start;
	int length;
};
vector<LNote> noteVec;
vector<int> startVec;

Options options;

int main(int argc, char** argv){
	options.process(argc, argv);

	int status;
	MidiFile midifile;
	if (options.getArgCount()){
		status = midifile.read(options.getArg(1));
	} else {
		status = midifile.read(cin);
	}

	int tpq = midifile.getTicksPerQuarterNote();
	midifile.linkNotePairs();
	midifile.joinTracks();
	
	midifile.doTimeAnalysis();

	MidiEvent* mev;	
	double duration;

	for (int track=0; track < midifile.getTrackCount(); track++) {
		for (unsigned int i=0; i<midifile[track].size(); i++) {
			mev = &midifile[track][i];
			if (!mev->isNoteOn()){
				continue;
			}

			duration=mev->getDurationInSeconds();
			if (duration <=0) continue;
			int startTmp = mev->seconds*1000;
			int lengthTmp = duration*1000;
			noteVec.push_back({mev->getKeyNumber(), startTmp, lengthTmp});
			vector<int>::iterator ret, ret2;
			ret = std::find(startVec.begin(), startVec.end(), startTmp);
			if(ret == startVec.end())
				startVec.push_back(startTmp);
			ret2 = std::find(startVec.begin(), startVec.end(), startTmp+lengthTmp);
			if(ret2 == startVec.end())
				startVec.push_back(startTmp+lengthTmp);
			
		}
	}
	sort(startVec.begin(), startVec.end());
	vector<int>::iterator iter_end;
	iter_end = std::unique(startVec.begin(), startVec.end() );
	startVec.erase(iter_end, startVec.end() );

   	for (unsigned int i=0; i<startVec.size(); i++){
		int start = startVec[i];
//		cout<< start <<":"<< endl;
		
		vector<LNote> tmpVec;
		int minLen = MAX_LEN;
		int nextStart = -1;
		if (i+1 < startVec.size())
			nextStart = startVec[i+1];
   		for (unsigned int j=0; j<noteVec.size(); j++){
			if (start == noteVec[j].start){
				tmpVec.push_back(noteVec[j]);
				if (noteVec[j].length<minLen)
					minLen = noteVec[j].length; 
			}
		}	
		if (nextStart<start+minLen && nextStart!=-1) minLen = nextStart - start; 
		for (unsigned int k=0; k<tmpVec.size(); k++){	// cout the Note array.
			cout<<tmpVec[k].key;
			if(k+1!=tmpVec.size())cout<<',';
			LNote ln = {tmpVec[k].key, tmpVec[k].start+minLen, tmpVec[k].length-minLen};
			if (ln.length > 0) noteVec.push_back(ln);
		}
		if(minLen != MAX_LEN && !tmpVec.empty())cout<<'\t'<<minLen<<endl;	//cout the notes' length.
		tmpVec.clear();
	
	}

	return 0;
}
