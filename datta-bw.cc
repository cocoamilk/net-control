//DATTA BACKWARD ...
//This is original DATTA algorith,
#include "datta.h"

typedef Network<int> MyNetwork;
typedef State<int> MyState;

void checkAllStates(const MyNetwork& n, MyState& s, int fxd, unordered_set<MyState>& stateSet, const unordered_set<MyState>& validNextStateSet) {
	if (fxd >= n.n) {
		MyState next = n.nextState(s);
		if (validNextStateSet.find(next) != validNextStateSet.end()) {
			cerr << "F " << next << " <" << s << ">" << endl;
			MyState nonControlState = s;
			nonControlState.fillControlToFalse();
			stateSet.insert(nonControlState);
		}
		return;
	}
	for (int t=0; t<2; t++) {
		s.state[fxd] = t;
		checkAllStates(n, s, fxd+1, stateSet, validNextStateSet);
	}
}

void run(MyNetwork& net, int M) {

	//Initialize
	cerr << "I  " << endl;
	unordered_set<MyState> desireSateSet;
	cerr << "I desireStateSet " << endl;
	MyState s = net.desireState;
	s.fillControlToFalse();
	desireSateSet.insert(s);
	
	cerr << "Init done" << endl;

	unordered_set<MyState> nextStateSet = desireSateSet;
	for (int time=M-1; time >=0; time--) {
		cerr << "Time step " << time << " ... nextSize: " << nextStateSet.size() << endl;
		unordered_set<MyState> currentStateSet;
		MyState s = MyState(net);
		checkAllStates(net, s, 0, currentStateSet, nextStateSet);
		nextStateSet = currentStateSet;
	}
	cerr << nextStateSet.size() << endl;

	MyState initState = net.initState;
	initState.fillControlToFalse();
	if (nextStateSet.find(initState) != nextStateSet.end()) {
		cout << "Found!" << endl;
	} else
		cout << "Not Found!" << endl;
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		cerr << "Not enough arguments" << endl;
		return 0;
	}
	ifstream fi(argv[1]);
	MyNetwork net(fi);
	int M;
	fi >> M;

	MyState s1(net), s2(net);
	for (int i=0; i<net.n; i++) {
		s1.state[i] = (i%2);
		s2.state[i] = (i%2);
	}
	s1.state[0] = 1;

	cout << "EQ " << (s1 == s2) << endl;

	cerr << "Net:" << net << endl;

	//M = 1;
	run(net, M);
	return 0;
}
