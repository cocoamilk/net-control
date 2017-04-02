#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>
#include <map>
#include "cparse-master/shunting-yard.h"
#include <unordered_set>
using namespace std;

const int maxn = 100;

struct Network;
struct State;

template<class T>
ostream& operator<<(ostream& os, const vector<T>& v) {
	os << "[";
	for (int i=0; i<v.size(); i++)
		os << v[i] << " ";
	return os << "]";
}

istream& operator>>(istream&, Network&);

struct State {
	Network* network;
	//bool state[maxn];
	vector<bool> state;

	State(Network* n=0);
	State(Network&n);
	bool operator==(const State& s) const {
		return state == s.state;
	}

	void fillControlToFalse();
	
	State toNoControl();
};

namespace std{
template<>
struct hash<State>
{
	hash<vector<bool>> h;
	size_t operator()(State const & s) const noexcept
	{
		return h(s.state);
	}
};
}

struct Network {
	int n;

	//vector<vector<int> > eOut;
	calculator calc[maxn];
	string formula[maxn];
	bool isControl[maxn];
	map<string, int> name2Id;
	string id2Name[maxn];
	State desireState, initState;
	vector<int> controlGenes;

	int getNameId(string name) {
		if (name2Id.find(name) != name2Id.end())
			return name2Id[name];
		name2Id[name] = n;
		id2Name[n] = name;
		n++;
		//eOut.push_back(vector<int>());
		desireState.state.push_back(false);
		initState.state.push_back(false);
		return n-1;
	}

	Network(ifstream& fi) {
		fi >> *this;
	}

	State nextState(const State&) const;

};

State::State(Network* n) : network(n), state(n==0?0:n->n, false) { }

State::State(Network& n) : network(&n), state(n.n, false) {}

void State::fillControlToFalse() {
	for (int i=0; i<network->controlGenes.size(); i++)
		state[network->controlGenes[i]] = false;
}
State State::toNoControl() {
	State s = *this;
	s.fillControlToFalse();
	return s;
}


ostream& operator<<(ostream& os, const State& s) {
	for (int i=0; i<s.network->n; i++)
		os << s.network->id2Name[i] << ":" << s.state[i] << " ";
	return os;
}

ostream& operator<<(ostream& os, Network& net) {
	os << "Network: {" << endl;
	os << "  n: " << net.n << endl;
	for (int i=0; i<net.n; i++)
		os << "  " << net.id2Name[i] << ": " << /*net.calc[i] <<*/ /*" out:" << net.eOut[i] << */ " isCont:" << net.isControl[i] << endl;
	os << "  control: " << net.controlGenes << endl;
	os << "  st-init: " << net.initState << endl;
	os << "  st-desi: " << net.desireState << endl;
	os << "}";
	return os;
}

istream& operator>>(istream& fi, Network& net) {
	net.n = 0;
	memset(net.isControl, 0, sizeof net.isControl);
	string l;
	getline(fi, l);
	istringstream is(l.c_str());
	cerr << "L Net Des inline `" << l << "`" << endl;

	net.desireState = &net;
	net.initState = &net;

	map<string, string> desGenes;
	for (string gene, init; is>>gene >> init; ) {
		//cerr << "L Net Des process " << gene << " " << init << endl;
		desGenes[gene] = init;
		if (init == "C") {
			net.isControl[net.getNameId(gene)] = true;
			//cerr << "L Net Des isControlSetted "<< endl;
			net.controlGenes.push_back(net.getNameId(gene));
		} else if (init == "1" || init == "0") {
			//cerr << "L Net Des nameId "<< net.getNameId(gene) << endl;
			net.initState.state[net.getNameId(gene)] = (init == "1") ? 1 : 0;
			is >> init;
			int des;
			is >> des;
			net.desireState.state[net.getNameId(gene)] = des;
		} else
			cerr << "Invalid gene " << gene << " = " << init << endl;
	}

	cerr << "L Net Des done" << endl;

	for (string a, eq, f; fi >> a, (a[0] != '#'); ) {
		fi >> eq;
		getline(fi, f);
		if (eq != "=")
			cerr << "Invalid eq mark: " << eq << endl;
		cerr << "L Net E " << a << ": " << f << endl;
		int id = net.getNameId(a);
		net.formula[id] = f;
		net.calc[id] = calculator(f.c_str());
	}

	cerr << "L Net done" << endl;

	if (desGenes.size() != net.n)
		cerr << "Some genes without desire state ... " << desGenes.size() << " <> " << net.n << endl;

	return fi;
}
State Network::nextState(const State& s) const {
	Network& net = *s.network;
	State next(&net);
	TokenMap vars;
	for (int i=0; i<net.n; i++)
		vars[net.id2Name[i]] = int(s.state[i]);
	for (int i=0; i<net.n; i++) {
	//	cerr << "  ev: " << net.formula[i] << endl;
		if (isControl[i])
			next.state[i] = 0;
		else
			next.state[i] = net.calc[i].eval(vars).asInt();
	}
	return next;
}


