//DATTA FORWARD ...
#include "datta.h"

typedef Network<int> MyNetwork;
typedef State<int> MyState;

void checkAllStates(const MyNetwork& n, MyState& s, int fxd, unordered_set<MyState>& stateSet) {
	if (fxd >= n.controlGenes.size()) {
		MyState next = n.nextState(s);
		cerr << "F " << next << " <" << s << ">" << endl;
		stateSet.insert(next.toNoControl());
		return;
	}
	for (int t=0; t<2; t++) {
		s.state[n.controlGenes[fxd]] = t;
		checkAllStates(n, s, fxd+1, stateSet);
	}
}

void run(MyNetwork& net, int M) {
	//Initialize
	cerr << "I  " << endl;
	unordered_set<MyState> initStateSet;
	cerr << "I initStateSet " << endl;
	MyState s = net.initState;
	s.fillControlToFalse();
	initStateSet.insert(s);
	
	cerr << "Init done" << endl;

	unordered_set<MyState> nextStateSet = initStateSet;
	for (int time=0; time <M; time++) {
		cerr << "Time step " << time << " ... nextSize: " << nextStateSet.size() << endl;
		unordered_set<MyState> currentStateSet;
		for (unordered_set<MyState>::iterator i=nextStateSet.begin(); i!=nextStateSet.end(); i++) {
			MyState s = *i;
			checkAllStates(net, s, 0, currentStateSet);
		}
		nextStateSet = currentStateSet;
	}
	cerr << nextStateSet.size() << endl;

	MyState desireState = net.initState;
	desireState.fillControlToFalse();
	if (nextStateSet.find(net.desireState) != nextStateSet.end()) {
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

	//MyState s1(net), s2(net);
	//for (int i=0; i<net.n; i++) {
	//	s1.state[i] = (i%2);
	//	s2.state[i] = (i%2);
	//}
	//s1.state[0] = 1;
	//
	//cout << "EQ " << (s1 == s2) << endl;

	cerr << "Net:" << net << endl;

	//M = 1;
	run(net, M);
	return 0;
}
