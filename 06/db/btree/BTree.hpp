#ifndef BTREE_HPP_
#define BTREE_HPP_

#include <pthread.h>
#include <iostream>
#include <map>
#include "../buffer/SlottedPage.hpp"
using namespace std;


//typedef uint64_t TID;

template <class T, class CMP>
class BTree{
public:
  struct Node {
    bool leaf;
    //uint32_t count;
    Node *next;
    Node *upper;
    map<T, Node, CMP> children;
    map<T, TID, CMP> entries;
  };
//  struct Inner : Node {
//    Node *upper;
//    map<T, Node> children;
//  };
//  struct Leaf : Node {
//    Leaf *next;
//    map<T, TID> entries;
//  };
  typedef std::pair<T, TID> Pair;
  BTree(int bTreeSegment); // BTreeSegment
  ~BTree();
  void insert(T key, TID tid);
  void erase(T key);
  bool lookup(T key, TID& tid);
  uint64_t size(){ return this->_size; }
  
private:
  uint32_t k;
  uint64_t _size;
  Node root;
  
  Node& findNode(T key, bool safe=false){
    Node& n = this->root;
    while(true){
      if(n.leaf){
        return n;
      }
      // not a leaf node
      if(safe){ // safe inner pages
        if(n.children.size() > this->k*2-1){ // no more room -> split
          int middle = n.children.size()/2;
          Node n1, n2;
          int i=0;
          for(auto it = n.children.begin(); it != n.children.end(); ++it,++i){
            if(i<middle){
              n1.children[it->first] = it->second;
            }else if(i==middle){
              n1.children[it->first] = it->second;
              n.upper->children[it->first] = n1; // TODO: other children of parent?
            }else{
              n2.children[it->first] = it->second;
            }
          }
          n1.upper = n2.upper = n.upper;
          // link from upper to n2? -> replace n in upper node
          auto it = n.children.end();
          n.upper->children[it->first] = n2;
        }
      }
      bool found = false;
      for(auto it = n.children.begin(); it != n.children.end(); ++it){
        if(!n.children.key_comp()(it->first, key)){ // i >= key == !(i < key)
          n = it->second;
          found = true;
          break;
        }
      }
      if(!found){
        if(n.upper == NULL){ // when does this happen?
          return n; // TODO: maybe not a leaf
        }
        n = *n.upper;
      }
    }
    return n; // to avoid eclipse warning about no return
  }
};
#endif /* BTREE_HPP_ */

template <class T, class CMP>
BTree<T, CMP>::BTree(int bTreeSegment) : _size(0){
  Node n;
  n.leaf = true;
  this->root = n;
  this->k = 5;
}

template <class T, class CMP>
BTree<T, CMP>::~BTree(){
  
}

template <class T, class CMP>
void BTree<T, CMP>::insert(T key, TID tid){
  this->_size++;
  Node& n = findNode(key, true);
  if(n.entries.size() < this->k*2){
    n.entries[key] = tid;
    return;
  }
  // split
  int middle = n.entries.size()/2;
  Node n1, n2;
  int i=0;
  for(auto it = n.entries.begin(); it != n.entries.end(); ++it,++i){
    if(i<middle){
      n1.entries[it->first] = it->second;
    }else if(i==middle){
      if(n.entries.key_comp()(key, it->first)){ // key < middle
        n1.entries[key] = tid;
        n2.entries[it->first] = it->second;
        n.upper->children[key] = n1;
      }else{
        n1.entries[it->first] = it->second;
        n2.entries[key] = tid;
        n.upper->children[it->first] = n1;
      }
      // splitting of parent not needed b/c of safe inner pages
    }else{
      n2.entries[it->first] = it->second;
    }
  }
  n1.next = &n2;
  n2.next = n.next;
  n1.upper = n2.upper = n.upper;
  // link from upper to n2? -> replace n in upper node
  auto it = n.entries.end();
  n.upper->children[it->first] = n2;
}

template <class T, class CMP>
void BTree<T, CMP>::erase(T key){
  Node& n = findNode(key);
  if(n.entries.count(key)){
    this->_size--;
    n.entries.erase(key);
  }
  if(n.entries.size() >= this->k){
    return; // stil at least half full -> stop
  }
  // we accept underfull pages
  /*
  // neighbor half full?
  if(n.next->entries.size() >= this->k){
    // balance both nodes, update separator and stop
    
    return;
  }
  // merge neighbor
  
  // remove separator from parent
  */
}

template <class T, class CMP>
bool BTree<T, CMP>::lookup(T key, TID& tid){
  Node& n = findNode(key);
  if(n.leaf){
    if(n.entries.count(key)>0){
      tid = n.entries[key];
      return true;
    }
  }
  return false;
}
