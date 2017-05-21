//Goliaei NET CONTROL ALGORITHM IMPLEMENTATION ...
//
#include "datta.h"
#include <boost/regex.hpp>

typedef Network<int> MyNetwork;
typedef State<int> MyState;

template<class T>
void uniq(vector<T>& v) {
	sort(v.begin(), v.end());
	typename vector<T>::iterator it = unique(v.begin(), v.end());
	v.resize(distance(v.begin(), it));
}

//To represent genes and their connections.
struct Graph {
	int n;
	//eIn: index of nodes to node[i]
	vector<vector<int>> eIn, eOut;

	Graph(MyNetwork& net) : n(net.n), eIn(net.n, vector<int>()), eOut(net.n, vector<int>()) {
		boost::regex re("[a-zA-Z_][a-zA-Z_0-9]*");        // Create the reg exp
		for (int i=0; i<n; i++) {
			//cerr << "G l " << i << " ... '" << net.formula[i] << "'" << endl;
			for (boost::sregex_token_iterator         // Create an iterator using a
				p(net.formula[i].begin(), net.formula[i].end(), re, 0);  // sequence and that reg exp
				p != boost::sregex_token_iterator();    // Create an end-of-reg-exp
				p++) {
				//cerr << "    F " << *p << endl;
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
void checkAllStates(const MyNetwork& n, MyState& s, int fxd, unordered_set<MyState>& stateSet) {
	if (fxd >= n.controlGenes.size()) {
		MyState next = n.nextState(s);
		cerr << "F " << next << " <" << s << ">" << endl;
		stateSet.insert(next.toNoControl());
		return;
	}
	for (int t=0; t<2; t++) {
		s.state[n.controlGenes[fxd]] = t;
		fillControlAndProduceNextStates(n, s, fxd+1, stateSet);
	}
}
*/

//Strongly Connected Components
struct SCC {
	const Graph& g;
	vector<vector<int>> comp;

	vector<int> preOrder;
	int mark[maxn];
	int isEndComponent[maxn];

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

	int getNodeComponent(int v) const {
		for (int i=0; i<comp.size(); i++)
			for (int j=0; j<comp[i].size(); j++)
				if (v == comp[i][j])
					return i;
		return -1;
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

		in.clear();
		for (int i=0; i<comp.size(); i++) {
			in.push_back(vector<int>());
			for (int j=0; j<comp[i].size(); j++)
				for (int k=0; k<g.eIn[comp[i][j]].size(); k++) {
					if (find(comp[i].begin(), comp[i].end(), g.eIn[comp[i][j]][k]) == comp[i].end())
					in[i].push_back(g.eIn[comp[i][j]][k]);
				}
			uniq(in[i]);
		}

		for (int c=0; c<comp.size(); c++)
			isEndComponent[c] = true;

		for (int i=0; i<g.n; i++) {
			int ic = getNodeComponent(i);
			for (int j=0; j<g.eIn[i].size(); j++) {
				int jc = getNodeComponent(g.eIn[i][j]);
				if (ic != jc)
					isEndComponent[jc] = false;
			}
		}

	}
};

ostream& operator<<(ostream& os, const SCC& scc) {
	os << "SCC {" << endl;
	for (int i=0; i<scc.comp.size(); i++)
		os << "  " << scc.comp[i] << endl;
	return os << "}" << endl;
}

ostream& operator<<(ostream& os, const unordered_set<MyState>& stateSet) {
	for (unordered_set<MyState>::const_iterator i=stateSet.begin(); i!=stateSet.end(); i++)
		os << *i << " ";
	return os << endl;
}

const int maxt = 50;

//Means putting value=val to vertex=v in time=t
struct ControlCondition {
	int t, v, val;
	ControlCondition(int _t=0, int _v=0, int _val=0) : t(_t), v(_v), val(_val) {}
	bool operator<(const ControlCondition& c) const {
		if (t != c.t)
			return t < c.t;
		if (v != c.v)
			return v < c.v;
		return val < c.val;
	}
};

struct Simulator {
	const MyNetwork& net;

	Simulator(const MyNetwork& _net) : net(_net) {}

	bool testCondition(const set<ControlCondition>& conditions, int M) {
		MyState s = net.initState;
		for (int i=0; i<M; i++) {
			//cerr << "SIM " << i << " " << s << endl;
			for (set<ControlCondition>::iterator j=conditions.begin(); j!=conditions.end(); j++) {
				if (j->t == i)
					s.state[j->v] = j->val;
			}
			s = net.nextState(s);
		}
		//cerr << "SIM " << "*" << " " << s << endl;
		for (int j=0; j<net.n; j++)
			if (!net.isControl[j] && s.state[j] != net.desireState.state[j]) {
				cerr << "Error in " << j << " " << net.isControl[j] << " " << s.state[j] << " " << net.desireState.state[j] << endl;
				return false;
			}
		return true;
	}
};

struct GNet {
	//vector<ControlCondition> possibleStatePath[maxt+1][maxn][2];
	bool possibleState[maxt+1][maxn][2];
	vector<ControlCondition> possibleStateConditions[maxt+1][maxn][2];
	const MyNetwork& net;
	const Graph& g;
	const SCC& scc;
	vector<vector<int>> componentInputs;
	int M;

	GNet(const MyNetwork& _net, const Graph& _g, const SCC& _scc, int _M) : net(_net), g(_g), scc(_scc), M(_M) {}

	void fillControlAndProduceNextStates(int c, int t, MyState& s, int i, unordered_set<MyState>& stateSet, vector<ControlCondition>& controlConditions) {
		if (i >= componentInputs[c].size()) {
			MyState n = net.nextState(s), nn(net);
			//zero state of genes not in this component
			for (int i=0; i<scc.comp[c].size(); i++)
				nn.state[scc.comp[c][i]] = n.state[scc.comp[c][i]];
			//cerr << "   I Stat " << nn << "(from: " << s << ")" << endl;
			for (int j=0; j<scc.comp[c].size(); j++) {
				int v = scc.comp[c][j];
				possibleState[t+1][v][nn.state[v]] = true;
				possibleStateConditions[t+1][v][nn.state[v]] = controlConditions;
				//possibleStatePath[t+1][v][nn.state[v]] = controlConditions;
				//possibleStatePath[time+1][v][(*i).state[v]] = 
			}
			stateSet.insert(nn);
			return;
		}
		int v = componentInputs[c][i];
		for (int o=0; o<2; o++)
			if (possibleState[t][v][o]) {
				//cerr << "  S[" << t << "][" << v << "] " << o << endl; 
				s.state[v] = o;
				int controlCoditionsPrevSize = controlConditions.size();
				//controlConditions.push_back(ControlCondition(t, scc.in[c][i], o));
				//TODO: following is time consuming, should be computed when result is found.
				controlConditions.insert(controlConditions.end(), possibleStateConditions[t][v][o].begin(), possibleStateConditions[t][v][o].end());
				fillControlAndProduceNextStates(c, t, s, i+1, stateSet, controlConditions);
				controlConditions.resize(controlCoditionsPrevSize);
			}
	}

	//fill possibleState[*][scc.comp[c][*]][*]
	void subDatta(int c) {
		//cerr << "    subDatta " << c << endl;
		unordered_set<MyState> initStateSet;
		MyState s = net.initState;
		s.fillControlToFalse();
		initStateSet.insert(s);

		for (int j=0; j<scc.comp[c].size(); j++) {
			int v = scc.comp[c][j];
			possibleState[0][v][s.state[v]] = true;
			possibleState[0][v][1-s.state[v]] = false;
		}

		unordered_set<MyState> currentStateSet = initStateSet;
		for (int time=0; time <M; time++) { //fills t=time+1
			//cerr << "C " << c << " Time step " << time << " ... currentSize: " << currentStateSet.size() << endl;
			for (int j=0; j<scc.comp[c].size(); j++) {
				int v = scc.comp[c][j];
				possibleState[time+1][v][0] = possibleState[time+1][v][1] = false;
			}

			unordered_set<MyState> nextStateSet;
			for (unordered_set<MyState>::iterator i=currentStateSet.begin(); i!=currentStateSet.end(); i++) {
				MyState s = *i;
				vector<ControlCondition> controlConditions;
				//controlConditions = s.path;
				fillControlAndProduceNextStates(c, time, s, 0, nextStateSet, controlConditions);
			}
			currentStateSet = nextStateSet;

			//cerr << "C " << c << " Time step " << time << " ... nextSize: " << currentStateSet.size() << " " << currentStateSet << endl;

			//fill possibleState[time+1]
			//for (unordered_set<MyState>::iterator i=currentStateSet.begin(); i!=currentStateSet.end(); i++)
			//	for (int j=0; j<scc.comp[c].size(); j++) {
			//		int v = scc.comp[c][j];
			//		possiblestate[time+1][v][(*i).state[v]] = true;
			//	}
		}
	}

	// It does not fix time=M, since it does not affect any furter times.
	//back track on component c, first on t then on i.
	void fixVertex(int c, int i, int t) {
		if (i >= scc.comp[c].size()) {
			//cerr << "  VF "<< c << " ";
			//for (int j=0; j<scc.comp[c].size(); j++) 
			//	cerr << scc.comp[c][j] << " ";
			//cerr << endl;
			//for (int tt=0; tt<M; tt++) {
			//	cerr << "      t=" << tt << " ";
			//	for (int j=0; j<scc.comp[c].size(); j++)
			//		cerr << possibleState[tt][scc.comp[c][j]][0] << "|" << possibleState[tt][scc.comp[c][j]][1] << " ";
			//	cerr << endl;
			//}
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

	//backtrack on component c, then next component
	void checkComponent(int c) {
		//cerr << "Comp " << c << endl;
		if (c >= scc.comp.size()) {
			for (int i=0; i<net.n; i++)
				if (!net.isControl[i] && !possibleState[M][i][net.desireState.state[i]]) {
					//cerr << "Not found here" << endl;
					return;
				}
			//Found!
			resultFound();
			return;
		}
		if (scc.comp[c].size() == 1 && net.isControl[scc.comp[c][0]]) {
			int v = scc.comp[c][0];
			for (int t=0; t<M; t++) {
				possibleState[t][v][0] = possibleState[t][v][1] = true;
				possibleStateConditions[t][v][0].clear();
				possibleStateConditions[t][v][0].push_back(ControlCondition(t, v, 0));
				possibleStateConditions[t][v][1].clear();
				possibleStateConditions[t][v][1].push_back(ControlCondition(t, v, 1));
			}
		} else {
			subDatta(c);
		}

			//cerr << "  VF "<< c << " ... ";
			//for (int j=0; j<scc.comp[c].size(); j++) 
			//	cerr << scc.comp[c][j] << " ";
			//cerr << endl;
			//for (int tt=0; tt<M; tt++) {
			//	cerr << "      t=" << tt << " ";
			//	for (int j=0; j<scc.comp[c].size(); j++)
			//		cerr << possibleState[tt][scc.comp[c][j]][0] << "|" << possibleState[tt][scc.comp[c][j]][1] << " ";
			//	cerr << endl;
			//}
		if (!scc.isEndComponent[c]) {
			fixVertex(c, 0, 0);
		} else {
			checkComponent(c+1);
		}
	}


	int bestSolutionCost;
	set<ControlCondition> bestSolution;

	vector<vector<int>> fullControlCondition(const set<ControlCondition>& cc) {
		vector<vector<int>> r(M, vector<int>(net.n, 0));
		int lastT[maxn];
		for(int i=0; i<net.n; i++)
			lastT[i] = -1;

		for (set<ControlCondition>::iterator i=cc.begin(); i!=cc.end(); i++) {
			for (int j=lastT[i->v]+1; j<=i->t; j++)
				r[j][i->v] = i->val;
			lastT[i->v] = i->t;
		}
		return r;
	}

	void resultFound() {
		Simulator sim(net);
		set<ControlCondition> conditions;
		for (int i=0; i<net.n; i++) 
			if (!net.isControl[i]) {
				conditions.insert(possibleStateConditions[M][i][net.desireState.state[i]].begin(), possibleStateConditions[M][i][net.desireState.state[i]].end());
			}

		int change = computeChanges(conditions);
		if (change < bestSolutionCost) {
			bestSolution = conditions;
			bestSolutionCost = change;
			//cerr << "  BEST: " << change << endl;
		}

		sim.testCondition(conditions, M);
		printSolution(conditions, change);
	}

	int computeChanges(const set<ControlCondition>& conditions) {
		vector<vector<int>> fConditions = fullControlCondition(conditions);

		int change = 0;
		for (int i=0; i<net.n; i++) {
			if (net.isControl[i]) {
				for (int t=1; t<M; t++)
					if (fConditions[t][i] != fConditions[t-1][i])
						change++;
			}
		}
		return change;
	}

	void printSolution(const set<ControlCondition>& conditions, int change) {
		cout << "Solution with changes=" <<  change << endl;
		cout << "T NODE VALUE" << endl;
		for (set<ControlCondition>::iterator i=conditions.begin(); i!=conditions.end(); i++) {
			cout << i->t << " " << net.id2Name[i->v] << " " << i->val << endl;
		}
		cout << endl;
	}

	void run() {
		bestSolutionCost = INT_MAX;
		componentInputs.clear();
		for (int c=0; c<scc.comp.size(); c++) {
			set<int> inputs;
			for (int i=0; i<scc.comp[c].size(); i++) {
				int v = scc.comp[c][i];
				for (int j=0; j<g.eIn[v].size(); j++) {
					inputs.insert(g.eIn[v][j]);
				}
			}
			componentInputs.push_back(vector<int>(inputs.begin(), inputs.end()));
		}
		checkComponent(0);
	}

};

int main(int argc, char* argv[]) {
	if (argc < 2) {
		cerr << "Not enough arguments" << endl;
		return 0;
	}
	ifstream fi(argv[1]);
	MyNetwork net(fi);
	int M;
	fi >> M;

	Graph g(net);
	//cerr << "Graph loaded" << endl;
	//cerr << g << endl;

	SCC scc(g);
	//cerr <<  scc << endl;

	GNet gnet(net, g, scc, M);

	gnet.run();

	//cout << "SOLUTION: " << gnet.bestCostSolution << endl;
	cout << "Best Solution (with minimum changes = " << gnet.bestSolutionCost << "):" << endl;
	gnet.printSolution(gnet.bestSolution, gnet.bestSolutionCost);
	return 0;
}
