#include "datta.h"
#include <boost/regex.hpp>


struct Graph {
	int n;
	vector<vector<int>> eIn, eOut;

	Graph(const Network& net) : n(net.n), eIn(net.n, vector<int>()), eOut(net.n, vector<int>()) {
		boost::regex re("[a-zA-Z][a-zA-Z_0-9]*");        // Create the reg exp
		for (int i=0; i<n; i++) {
			for (boost::sregex_token_iterator         // Create an iterator using a
				p(formula[i].begin(), formula[i].end(), re, -1);  // sequence and that reg exp
				p != sboost::sregex_token_iterator end;    // Create an end-of-reg-exp
				p++) {
				int o = net.getNameId(*p);
				eIn[i].push_back(o);
				eOut[o].push_back(i);
			}
		}
		// marker
		
	}
};

void checkAllStates(const Network& n, State& s, int fxd, unordered_set<State>& stateSet) {
	if (fxd >= n.controlGenes.size()) {
		State next = n.nextState(s);
		cerr << "F " << next << " <" << s << ">" << endl;
		stateSet.insert(next.toNoControl());
		return;
	}
	for (int t=0; t<2; t++) {
		s.state[n.controlGenes[fxd]] = t;
		checkAllStates(n, s, fxd+1, stateSet);
	}
}

void run(Network& net, int M) {
	//Initialize
	cerr << "I  " << endl;
	unordered_set<State> initStateSet;
	cerr << "I initStateSet " << endl;
	State s = net.initState;
	s.fillControlToFalse();
	initStateSet.insert(s);
	
	cerr << "Init done" << endl;

	unordered_set<State> nextStateSet = initStateSet;
	for (int time=0; time <M; time++) {
		cerr << "Time step " << time << " ... nextSize: " << nextStateSet.size() << endl;
		unordered_set<State> currentStateSet;
		for (unordered_set<State>::iterator i=nextStateSet.begin(); i!=nextStateSet.end(); i++) {
			State s = *i;
			checkAllStates(net, s, 0, currentStateSet);
		}
		nextStateSet = currentStateSet;
	}
	cerr << nextStateSet.size() << endl;

	State desireState = net.initState;
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
	Network net(fi);
	int M;
	fi >> M;

	State s1(net), s2(net);
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
