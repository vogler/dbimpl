#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <list>

using namespace std;

#define MAX_NODE_ELEMENTS ((treeK<<1) - 1)
#define MIN_NODE_ELEMENTS treeK
#define mp make_pair

typedef int dataType;
typedef int keyType;

struct dataNode {
	// contains the value of the node;
	dataType value;
};

typedef pair <keyType, dataNode*> keyData;

struct iNode {
	// The key as int together with a dataNode if it is a leaf.
	vector< pair< pair<keyData, iNode*>, iNode* > > key;

	// Stores the parent of this iNode.
	iNode* parent;
	iNode* next;
	// The height of the subtree starting in node.
	bool isLeaf;
};

const int treeK = 10;
iNode *root = NULL;
vector<iNode *> consolidate;


bool cmp(const pair< pair<keyData, iNode*>, iNode* > a, const pair< pair<keyData, iNode*>, iNode* > b) {
	return a.first.first.first < b.first.first.first;
}

// Returns the index of the first element in vector smaller or equal to key.
unsigned findIndexOfKey(keyType key, vector<pair< pair<keyData, iNode*>, iNode* > >& v) {
	// TODO:Binary search would be more efficient.
	for(size_t i = 0; i < v.size(); ++ i) {
		if(v[i].first.first.first > key)
			return i-1;
	}
	return v.size()-1;
}

void printNode(iNode *p) {
	if (p == NULL) {
		return;
	}
	size_t sz = p->key.size();
	for (size_t i = 0; i < sz; ++ i) {
		cout << p->key[i].first.first.first << " ";
	}
	cout << "\n";
}

void sortNodeForKey(iNode *p, keyType key) {
    sort(p->key.begin(), p->key.end(), cmp);
    unsigned ind = findIndexOfKey(key, p->key);
    assert(key == p->key[ind].first.first.first);
    if(ind > 0)
        p->key[ind-1].second = p->key[ind].first.second;
    if (ind < p->key.size()-1)
        p->key[ind+1].first.second = p->key[ind].second;
}

bool containsChild(iNode *p, iNode *child) {
	 if(p != NULL) {
		size_t sz = p->key.size();
		if (!sz) return false;
		for (size_t i = 0; i < sz; ++ i) {
			if((p->key[i].first.second) == child) return true;
		}

		if((p->key[sz-1].second) == child) return true;
	 }
	 return false;
}

void ins(iNode *&p, iNode *leftC, keyType key, dataNode* data) {
	if (p == NULL) {
		// Empty leaf node or new level is added to the B-Tree.
		p = new iNode;
		p->parent = NULL;
		p->next = NULL;

		root = p;
		p->isLeaf = true;
		p->key.push_back(mp(mp(mp(key, data), (iNode *)NULL),(iNode *)NULL));
		return;
	}

	if(p->isLeaf || leftC != NULL) {
		// Add Element at leaf or continue with inserting in parent.

		// Previous added a new level to the B-Tree.
		if(leftC != NULL) {
			p->isLeaf = false;
			p->key.push_back(mp(mp(mp(key, (dataNode*)NULL), leftC),leftC->next));
		} else {
			p->key.push_back(mp(mp(mp(key, data), (iNode *)NULL),(iNode *)NULL));
		}
        sortNodeForKey(p, key);

		if (p->key.size() > MAX_NODE_ELEMENTS) {
			// Split node.
			vector< pair< pair<keyData, iNode*>, iNode* > > l, r;
			size_t half = p->key.size() / 2;
			keyType upKey = p->key[half].first.first.first;

			l.assign(p->key.begin(), p->key.begin() + half);
			r.assign(p->key.begin() + half + 1, p->key.end());

			p->key.assign(l.begin(), l.end());
			consolidate.push_back(p);

			// Right node.
			iNode *p2 = new iNode;
			p2->isLeaf = p->isLeaf;
			p2->key.assign(r.begin(), r.end());
			//p2->parent = p->parent;
			p2->next = p->next;
			p2->parent = p->parent;
			p->next = p2;
			if(p->parent == NULL) {
				p->parent = new iNode;
				p->parent->parent = NULL;
				p->parent->isLeaf = false;
				(p->parent)->key.push_back(mp(mp(mp(upKey, (dataNode*)NULL), p),p2));
				root = p->parent;

				p2->parent = p->parent;
				assert (p2 == p->parent->key[0].second);
				assert (p == p->parent->key[0].first.second);
			} else {

				assert(p2 == p->next);
				ins(p->parent, p, upKey, NULL);
				assert(p2 == p->next);

				//int ind = findIndexOfKey()

				//assert(p2->parent == p->parent);
				if(containsChild(p->parent->next, p)) {
					p->parent = p->parent->next;
					p2->parent = p->parent;
					assert(p2->parent == p->parent);
				} else if(containsChild(p->parent->next, p2)) {
					p2->parent = p->parent->next;
					assert(p2->parent != p->parent);
				}
//				if (p2->parent != p->parent) cout << "**********splitted above ***********\n";

			}
		}
	} else {
		//Search for the leaf node where to add the key.
		int ind = findIndexOfKey(key, p->key);

        if(ind >= 0) {
            ins(p->key[ind].second, leftC, key, data);
        } else {
            ins(p->key[0].first.second, leftC, key, data);
        }
	}
}

void insert(iNode *&p, keyType key, dataNode* data) {
	ins(p, NULL, key, data);

	for (size_t i = 0; i < consolidate.size(); ++ i) {
		//cout<<consolidate[i]<<" "<<consolidate[i]->parent<<" ";
		consolidate[i]->next->parent = consolidate[i]->parent;
		//cout<<consolidate[i]->next->parent<<"\n";
	}
	//cout<<"*************************"<<endl;

	consolidate.clear();
}

void traverseKeyInOrder(iNode *p) {
    if(p != NULL) {
        size_t sz = p->key.size();
        if (!sz) return;
        for (size_t i = 0; i < sz-1; ++ i) {
            assert(p->key[i].second == p->key[i+1].first.second);

            traverseKeyInOrder(p->key[i].first.second);
            printf("%d ", p->key[i].first.first.first);
        }
        traverseKeyInOrder(p->key[sz-1].first.second);

        printf("%d ", p->key[sz-1].first.first.first);

        traverseKeyInOrder(p->key[sz-1].second);
    }
}

void traverseKey(iNode *p) {
	if(p != NULL) {
		size_t sz = p->key.size();

        cout << p << " and parent " << p->parent << "\n";

        if (!sz) return;
		for (size_t i = 0; i < sz; ++ i) {
			printf("%d ", p->key[i].first.first.first);
		}
        printf("\n");

        for (size_t i = 0; i < sz; ++ i) {
            traverseKey(p->key[i].first.second);
		}

		traverseKey(p->key[sz-1].second);
        cout << "Up in tree\n";
	}
}

dataType* lookup(iNode*& p, keyType key) {
	return 0;
}

void del(iNode*& p, keyType key) {

}

int main() {
	int N = 100;
    int val[] = {807, 249, 73, 658, 930, 272, 544, 878, 923, 709, 440, 165, 492, 42, 987, 503, 327, 729, 840, 612};

    iNode *t = NULL;
//    srand(time(0));
	for (int i = 0; i < N; ++ i) {
		long temp = i;//rand()%10001;
//		cout << temp << endl;
		dataNode *d = new dataNode;
		d->value = temp;
		insert(t, temp, d);
		t = root;
		assert(root->parent == NULL);
//		if(!(i % 3)) {
//			traverseKey(root);
//			cout <<"===================\n";
			//traverseKeyInOrder(root);
		}
	}
//    traverseKey(root);
//    cout << root << "\n";
    traverseKeyInOrder(root);

	for (int i = 0; i <= (N >> 1); ++ i) {
		//int temp = random(50000);
		del(root, i);
	}
	//traverseKey(root);

	return 0;
}
