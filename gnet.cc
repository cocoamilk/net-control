#include "datta.h"
#include <boost/regex.hpp>

template<class T>
void uniq(vector<T>& v) {
	sort(v.begin(), v.end());
	typename vector<T>::iterator it = unique(v.begin(), v.end());

	v.resize(distance(v.begin(), it));
}

struct Graph {
	int n;
	vector<vector<int>> eIn, eOut;

	Graph(Network& net) : n(net.n), eIn(net.n, vector<int>()), eOut(net.n, vector<int>()) {
		boost::regex re("[a-zA-Z_][a-zA-Z_0-9]*");        // Create the reg exp
		for (int i=0; i<n; i++) {
			cerr << "G l " << i << " ... '" << net.formula[i] << "'" << endl;
			for (boost::sregex_token_iterator         // Create an iterator using a
				p(net.formula[i].begin(), net.formula[i].end(), re, 0);  // sequence and that reg exp
				p != boost::sregex_token_iterator();    // Create an end-of-reg-exp
				p++) {
				cerr << "    F " << *p << endl;
				int o = net.getNameId(*p);
				eIn[i].push_back(o);
				eOut[o].push_back(i);
			}
			uniq(eIn[i]);
			uniq(eOut[i]);
		}
		// marker
		
	}
};

ostream& operator<<(ostream& os, Graph& g) {
	os << "Graph { " << g.n << endl;
	for (int i=0; i<g.n; i++) {
		os << "  " << i << ":<< " << g.eIn[i] << " ";
		os << ":>> " << g.eOut[i] << endl;
	}
	return os << "}" << endl;

}

/*
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

*/
struct SCC {
	const Graph& g;
	vector<vector<int>> comp;

	vector<int> preOrder;
	int mark[maxn];

	//id of vertices which have edge to this component
	vector<vector<int>> in;

	void dfs1(int v) {
		mark[v] = true;
		for (int i=0; i<g.eOut[v].size(); i++)
			if (!mark[g.eOut[v][i]])
				dfs1(g.eOut[v][i]);
		preOrder.push_back(v);
	}

	void dfs2(int v) {
		mark[v] = true;
		comp.back().push_back(v);
		for (int i=0; i<g.eIn[v].size(); i++)
			if (!mark[g.eIn[v][i]])
				dfs2(g.eIn[v][i]);
	}

	SCC(const Graph& _g) : g(_g) {
		memset(mark, 0, sizeof mark);
		for (int i=0; i<g.n; i++)
			if (!mark[i])
				dfs1(i);

		memset(mark, 0, sizeof mark);
		for (vector<int>::reverse_iterator i=preOrder.rbegin(); i!=preOrder.rend(); i++)
			if (!mark[*i]) {
				comp.push_back(vector<int>());
				dfs2(*i);
			}

		for (int i=0; i<comp.size(); i++) {
			in.push_back(vector<int>());
			for (int j=0; j<comp[i].size(); j++)
				for (int k=0; k<g.eIn[comp[i][j]].size(); k++) {
					if (find(comp[i].begin(), comp[i].end(), g.eIn[comp[i][j]][k]) == comp[i].end())
					in[i].push_back(g.eIn[comp[i][j]][k]);
				}
			uniq(in[i]);
		}

	}
};

ostream& operator<<(ostream& os, const SCC& scc) {
	os << "SCC {" << endl;
	for (int i=0; i<scc.comp.size(); i++)
		os << "  " << scc.comp[i] << endl;
	return os << "}" << endl;
}

ostream& operator<<(ostream& os, const unordered_set<State>& stateSet) {
	for (unordered_set<State>::const_iterator i=stateSet.begin(); i!=stateSet.end(); i++)
		os << *i << " ";
	return os << endl;
}

const int maxt = 50;

struct ControlCondition {
	int t, v, val;
	ControlCondition(int _t=0, int _v=0, int _val=0) : t(_t), v(_v), val(_val) {}
};

struct GNet {
	vector<ControlCondition> possibleStatePath[maxt+1][maxn][2];
	bool possibleState[maxt+1][maxn][2];
	const Network& net;
	const Graph& g;
	const SCC& scc;
	int M;

	void checkAllStates(int c, int t, State& s, int i, unordered_set<State>& stateSet, vector<ControlCondition>& controlConditions) {
		if (i >= scc.in[c].size()) {
			State n = net.nextState(s), nn(net);
			//zero state of genes not in this component
			for (int i=0; i<scc.comp[c].size(); i++)
				nn.state[scc.comp[c][i]] = n.state[scc.comp[c][i]];
			cerr << "   I Stat " << nn << "(from: " << s << ")" << endl;
			for (int j=0; j<scc.comp[c].size(); j++) {
				int v = scc.comp[c][j];
				possibleState[t+1][v][nn.state[v]] = true;
				possibleStatePath[t+1][v][nn.state[v]] = controlConditions;
				//possibleStatePath[time+1][v][(*i).state[v]] = 
			}
			stateSet.insert(nn);
			return;
		}
		for (int o=0; o<2; o++)
			if (possibleState[t][scc.in[c][i]][o]) {
				cerr << "  S[" << t << "][" << scc.in[c][i] << "] " << o << endl; 
				s.state[scc.in[c][i]] = o;
				int controlCoditionsPrevSize = controlConditions.size();
				controlConditions.push_back(possibleStatePath[t][scc.in[c][i]][o]);
				checkAllStates(c, t, s, i+1, stateSet);
				controlConditions.resize(controlCoditionsPrevSize);
			}
	}

	void subDatta(int c) {
		cerr << "    subDatta " << c << endl;
		unordered_set<State> initStateSet;
		State s = net.initState;
		s.fillControlToFalse();
		initStateSet.insert(s);

		for (int j=0; j<scc.comp[c].size(); j++) {
			int v = scc.comp[c][j];
			possibleState[0][v][s.state[v]] = true;
			possibleState[0][v][1-s.state[v]] = false;
		}

		unordered_set<State> currentStateSet = initStateSet;
		for (int time=0; time <M; time++) {
			cerr << "C " << c << " Time step " << time << " ... currentSize: " << currentStateSet.size() << endl;
			for (int j=0; j<scc.comp[c].size(); j++) {
				int v = scc.comp[c][j];
				possibleState[time+1][v][0] = possibleState[time+1][v][1] = false;
			}

			unordered_set<State> nextStateSet;
			for (unordered_set<State>::iterator i=currentStateSet.begin(); i!=currentStateSet.end(); i++) {
				State s = *i;
				vector<ControlCondition> controlConditions;
				controlConditions = s.path;
				checkAllStates(c, time, s, 0, nextStateSet);
			}
			currentStateSet = nextStateSet;

			cerr << "C " << c << " Time step " << time << " ... nextSize: " << currentStateSet.size() << " " << currentStateSet << endl;

			//fill possibleState[time+1]
			//for (unordered_set<State>::iterator i=currentStateSet.begin(); i!=currentStateSet.end(); i++)
			//	for (int j=0; j<scc.comp[c].size(); j++) {
			//		int v = scc.comp[c][j];
			//		possiblestate[time+1][v][(*i).state[v]] = true;
			//	}
		}
	}

	// It does not fix time=M, since it does not affect any furter times.
	void fixVertex(int c, int i, int t) {
		if (i >= scc.comp[c].size()) {
			cerr << "  VF "<< c << " ";
			for (int j=0; j<scc.comp[c].size(); j++) 
				cerr << scc.comp[c][j] << " ";
			cerr << endl;
			for (int tt=0; tt<M; tt++) {
				cerr << "      t=" << tt << " ";
				for (int j=0; j<scc.comp[c].size(); j++)
					cerr << possibleState[tt][scc.comp[c][j]][0] << "|" << possibleState[tt][scc.comp[c][j]][1] << " ";
				cerr << endl;
			}
			checkComponent(c+1);
			return;
		} else if (t >= M) {
			fixVertex(c, i+1, 0);
			return;
		}
		if (scc.comp[c].size() == 1 && g.eOut[scc.comp[c][0]].size() == 1)
			fixVertex(c, i+1, 0);
		else {
			int v = scc.comp[c][i];
			for (int o=0; o<2; o++)
				if (possibleState[t][v][o]) {
					int bo = possibleState[t][v][1-o];
					possibleState[t][v][1-o] = false;
					fixVertex(c, i, t+1);
					possibleState[t][v][1-o] = bo;
				}
		}
	}

	void checkComponent(int c) {
		cerr << "Comp " << c << endl;
		if (c >= scc.comp.size()) {
			for (int i=0; i<net.n; i++)
				if (!net.isControl[i] && !possibleState[M][i][net.desireState.state[i]]) {
					cerr << "Not found here" << endl;
					return;
				}
			//Found!
			cout << "Found!" << endl;
			return;
		}
		if (scc.comp[c].size() == 1 && net.isControl[scc.comp[c][0]]) {
			int v = scc.comp[c][0];
			for (int t=0; t<M; t++)
				possibleState[t][v][0] = possibleState[t][v][1] = true;
		} else {
			subDatta(c);
		}

			cerr << "  VF "<< c << " ... ";
			for (int j=0; j<scc.comp[c].size(); j++) 
				cerr << scc.comp[c][j] << " ";
			cerr << endl;
			for (int tt=0; tt<M; tt++) {
				cerr << "      t=" << tt << " ";
				for (int j=0; j<scc.comp[c].size(); j++)
					cerr << possibleState[tt][scc.comp[c][j]][0] << "|" << possibleState[tt][scc.comp[c][j]][1] << " ";
				cerr << endl;
			}
		fixVertex(c, 0, 0);
	}

	void run() {
		checkComponent(0);
	}

	GNet(const Network& _net, const Graph& _g, const SCC& _scc, int _M) : net(_net), g(_g), scc(_scc), M(_M) {}
};

int main(int argc, char* argv[]) {
	if (argc < 2) {
		cerr << "Not enough arguments" << endl;
		return 0;
	}
	ifstream fi(argv[1]);
	Network net(fi);
	int M;
	fi >> M;

	Graph g(net);
	cerr << "Graph loaded" << endl;
	cerr << g << endl;

	SCC scc(g);
	cerr <<  scc << endl;

	GNet gnet(net, g, scc, M);

	gnet.run();
	return 0;
}
