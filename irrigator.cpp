// CMSC 341 - Fall 2025 - Project 3
// student: Andrew Soth
// professor: Kartchner
#include "irrigator.h"

// private functions are located after the template functions

// default constructor - all values are set to the default / initial values
// called when something creates a region without parameters
Region::Region(){ 
  m_heap = nullptr;       // initialize the root of the heap to empty
  m_size = 0;             // no crops yet
  m_priorFunc = nullptr;  // no priority function assigned
  m_heapType = NOTYPE;    // default to NOTYPE
  m_structure = NOSTRUCT; // default to NOSTRUCT
  m_regPrior = 0;         // defaults to 0
}

// parameterized constructor - setup a region with the inputted values
// called when valid input are provided
Region::Region(prifn_t priFn, HEAPTYPE heapType, STRUCTURE structure, int regPrior) {
  // checks if any of the parameter are invalid, a default object is created instead
  if (priFn == nullptr || heapType == NOTYPE || structure == NOSTRUCT || regPrior <= 0) {
    m_heap = nullptr;
    m_size = 0;
    m_priorFunc = nullptr;
    m_heapType = NOTYPE;
    m_structure = NOSTRUCT;
    m_regPrior = 0;
  }
  
  // creates an object with the parameter input
  else {
    m_heap = nullptr;         // initialize the root of the heap to empty
    m_size = 0;               // no crops yet
    m_priorFunc = priFn;      // store the prioity function pointer
    m_heapType = heapType;    // store heap type (MINHEAP or MAXHEAP)
    m_structure = structure;  // store structure type (SKEW or LEFTIST)
    m_regPrior = regPrior;    // region-level priority (used by Irrigator)
  }
}

// destructor constructor - deallocates the memory and re-initializes the member variables
// calls when region objects are no longer in use
Region::~Region() {
  clear();
}

// clear - clears the queue, delete all the nodes , and re-initializes the member variables
// calls in the destuctor constructor
void Region::clear() {
  clearHeap(m_heap);  // recursively delete the nodes
  m_heap = nullptr;   // set the root of the heap to empty
  m_size = 0;         // no crops
}

// copy constructor - creates a deep copy of an region object including pointers
// calls when want a deep copy of a region object
Region::Region(const Region& rhs) {
  // copy simple members
  m_size = rhs.m_size;
  m_priorFunc = rhs.m_priorFunc;
  m_heapType = rhs.m_heapType;
  m_structure = rhs.m_structure;
  m_regPrior = rhs.m_regPrior;

  // deep copy the heap
  m_heap = copyHeap(rhs.m_heap);  // recursively copies the nodes
  
}

// assignment operator - creates a copy of the object
// calls when having to set two regions equal to each other like copying
Region& Region::operator=(const Region& rhs) {
  // self-assigment check
  if (this == &rhs) {
    return *this;
  }

  // free old heap
  clear();

  // copies the simple variable members
  m_size = rhs.m_size;
  m_priorFunc = rhs.m_priorFunc;
  m_heapType = rhs.m_heapType;
  m_structure = rhs.m_structure;
  m_regPrior = rhs.m_regPrior;

  // deep copy heap
  m_heap = copyHeap(rhs.m_heap);

  // returns the object
  return *this;

}

// mergeWithQueue - takes rhs and complete merge it into this queue
// rhs will be empty afterwards
void Region::mergeWithQueue(Region& rhs) {
  // check for self-merge
  if (this == &rhs) {
    return;
  }

  // check for different priority functions
  if (m_priorFunc != rhs.m_priorFunc) {
    throw domain_error ("Regions have different priority function");
  }

  // check for different structure
  if (m_structure != rhs.m_structure) {
    throw domain_error ("Regions have different structures");
  } 

  // check for different heap type
  if (m_heapType != rhs.m_heapType) {
    throw domain_error("Region have different heap types");
  }

  // merge rhs's heaps into this heap
  m_heap = merge(m_heap, rhs.m_heap);

  // update size
  m_size += rhs.m_size;

  // leave rhs empty
  rhs.m_heap = nullptr;
  rhs.m_size = 0;
}

// insertCrop - inserts a crop object into the queue and maintains the heap type and structure
bool Region::insertCrop(const Crop& crop) {
  // if the crop is invalid, it is rejected
  if (m_heapType == NOTYPE || m_structure == NOSTRUCT || m_priorFunc == nullptr) {
    // cannot insert into an empty object
    return false;
  }
  
  // compute priority using the region's priority function
  int priority = m_priorFunc(crop);

  // if priority is invalid (<=0), do not insert
  if (priority <= 0) {
    return false;
  }

  // create a new node for crop
  Crop* newNode = new Crop(crop);

  // initialize the node fields
  newNode->m_left = nullptr;
  newNode->m_right = nullptr;
  newNode->m_npl = 0;

  // merge the new node into the existing heap
  m_heap = merge(m_heap, newNode);

  // update size
  m_size++;

  return true;
}

// return the number of crops in queue
int Region::numCrops() const {
  return m_size;
}

// return the priority function pointer
prifn_t Region::getPriorityFn() const {
  return m_priorFunc;
}

// removes the node and returns the highest priority crop from the queue
Crop Region::getNextCrop() {
  // checks if the queue is null
  if (m_heap == nullptr) {
    throw out_of_range("Region::getNextCrop() called on an empty heap");
  }

  // save the root crop to return
  Crop rootCrop = *m_heap;

  // save the children
  Crop* leftSub = m_heap->m_left;
  Crop* rightSub = m_heap->m_right;

  // delete the root node
  delete m_heap;

  // merge left and right subheaps
  m_heap = merge(leftSub, rightSub);

  // update size
  m_size--;

  return rootCrop;
}

// sets a new priority function, sets corresponding heap type, rebuild the heap, and does not re-allocate memory
void Region::setPriorityFn(prifn_t priFn, HEAPTYPE heapType) {
  // checks for priority and heap type if valid
  if (priFn == nullptr || heapType == NOTYPE) {
    // clears the variable members out
    clear();
    return;
  }

  // update configuration
  m_priorFunc = priFn;
  m_heapType = heapType;

  // saving the old heap root
  Crop* oldHeap = m_heap;

  // resetting the region
  m_heap = nullptr;
  m_size = 0;

  // traverse the old heap and reinsert the nodes into the new heap
  rebuildHeap(oldHeap);
}

// sets heap to a new structure, rebuilds the heap, and reuses the nodes
void Region::setStructure(STRUCTURE structure){
  // checks if the passing parameter structure is valid
  if (structure == NOSTRUCT) {
    // clears the variable members out
    clear();
    return;
  }
  
  // update configuration
  m_structure = structure;

  // rebuild the heap
  Crop* oldHeap = m_heap;
  m_heap = nullptr;
  m_size = 0;

  // transfer the nodes into the new structure
  rebuildHeap(oldHeap);
}

// returns the structure of the heap
STRUCTURE Region::getStructure() const {
  return m_structure;
}

// returns the heap type of the heap
HEAPTYPE Region::getHeapType() const {
  return m_heapType;
}

// prints the contents of the queue using preorder traversal 
// first crop printed should have the highest priority
void Region::printCropsQueue() const {
  if (m_heap == nullptr) {
    cout << "Empty heap" << endl;
    return;
  }
  
  // recursively prints the heap in preorder traversal
  printHelper(m_heap);
}

void Region::dump() const {
  if (m_size == 0) {
    cout << "Empty heap.\n" ;
  } else {
    cout << "Region " << m_regPrior << ": => ";
    dump(m_heap);
  }
  cout << endl;
}

void Region::dump(Crop *pos) const {
  if ( pos != nullptr ) {
    cout << "(";
    dump(pos->m_left);
    if (m_structure == SKEW)
        cout << m_priorFunc(*pos) << ":" << pos->m_cropID;
    else
        cout << m_priorFunc(*pos) << ":" << pos->m_cropID << ":" << pos->m_npl;
    dump(pos->m_right);
    cout << ")";
  }
}

ostream& operator<<(ostream& sout, const Crop& crop) {
  sout << "Crop ID: " << crop.getCropID() 
        << ", current temperature: " << crop.getTemperature()
        << ", current soil moisture: " << crop.getMoisture() << "%"
        << ", current time: " << crop.getTimeString()
        << ", plant type: " << crop.getTypeString();
  return sout;
}

/******************************************
* Private function *
******************************************/
// helper function that recursively delete the nodes in the tree
void Region::clearHeap(Crop* node) {
  // checks if node is already null
  if (node == nullptr) {
    return;
  }

  // recursively goes down the left and right subtree to delete
  clearHeap(node->m_left);
  clearHeap(node->m_right);

  delete node;
}

// helper function that can recurisvely clone the data
Crop* Region::copyHeap(Crop* node) {
  // checks if node is already null
  if (node == nullptr) {
    return nullptr;
  }

  // create a new node by copying the data
  Crop* newNode = new Crop (*node);

  // reset children before recursion
  newNode->m_left = nullptr;
  newNode->m_right = nullptr;

  // recursively copy children
  newNode->m_left = copyHeap(node->m_left);
  newNode->m_right = copyHeap(node->m_right);

  return newNode;
}

// takes two crops and merge them together into 1 heap
Crop* Region::merge(Crop* h1, Crop* h2) {
  // checks if either parameter is null
  if (h1 == nullptr) {
    return h2;
  }
  if (h2 == nullptr) {
    return h1;
  }

  // compare priorities depending on heap type
  int p1 = m_priorFunc(*h1);
  int p2 = m_priorFunc(*h2);

  // if the left subtree is greater than the right subtree, swap if heap type is minheap
  if (m_heapType == MINHEAP) {
    if (p1 > p2) {
      swapValues(h1, h2);
    }
  }
  // if the right subtree is greater than the left subtree, swap if heap type is maxheap
  else if (m_heapType == MAXHEAP) {
    if (p1 < p2) {
      swapValues(h1, h2);
    }
  }

  // recursively merge into the right child
  h1->m_right = merge(h1->m_right, h2);

  // apply skew rules
  if (m_structure == SKEW) {
    // swap children in skew heap
    swapValues(h1->m_left, h1->m_right);
  }
  // apply leftist rules
  else if (m_structure == LEFTIST) {
    // ensure leftist property: npl(left) >= npl(right)
    int leftNpl = h1->m_left ? h1->m_left->m_npl : -1;
    int rightNpl = h1->m_right ? h1->m_right->m_npl : -1;
    
    // npl needs to be bigger on the left side than the right side
    if (leftNpl < rightNpl) {
      swapValues(h1->m_left, h1->m_right);
    }
    
    // update this node's npl
    h1->m_npl = 1 + minValue(
      h1->m_left  ? h1->m_left->m_npl : -1,
      h1->m_right ? h1->m_right->m_npl : -1
    );

  }

  return h1;
}

// swaps two Crop pointer reference
void Region::swapValues(Crop*& a, Crop*& b) {
  Crop* temp = a;   // temp stores a
  a = b;            // set a to b
  b = temp;         // set b to a (which is the temp)
}

// compares two int value and return the smaller int value
int Region::minValue(int a, int b) {
  return (a < b) ? a : b;
}

// 
void Region::printHelper(Crop* node) const {
  // checks if parameter input is valid
  if (node == nullptr) {
    return;
  }

  // print priority in buckets before crop details
  cout << "[" << m_priorFunc(*node) << "]" << *node << endl;

  // preorder traversal print
  printHelper(node->m_left);
  printHelper(node->m_right);

}

// rebuilds the heap starting from a given node
// performs a post-order traversal
// detaches the current node, reinserts it into the heap, then recurses on its children.
void Region::rebuildHeap(Crop* node) {
  // checks if parameter input is valid
  if (node == nullptr) {
    return;
  }

  // save children
  Crop* left = node->m_left;
  Crop* right = node->m_right;

  // disconnect node
  node->m_left = nullptr;
  node->m_right = nullptr;
  node->m_npl = 0;

  // reinsert node into new heap
  m_heap = merge(m_heap, node);
  m_size++;

  // recursively rebuild heap on the children
  rebuildHeap(left);
  rebuildHeap(right);
}

// returns the region priority
int Region::getRegPrior() const {
  return m_regPrior;
}

 
//////////////////////////////////////////////////////////////

// constructor - initializes an irrigator wuth a given capacity
Irrigator::Irrigator(int size){
  // throws invalid arguement if size <= 0
  if (size <= 0) {
    throw invalid_argument("Irrigator size cannot be negative");
  }
  
  m_capacity = size;  // sets the size of the array
  m_size = 0;         // no region objects yet

  // allocat array of Region objects
  m_heap = new Region[m_capacity + 1];
  
}

// destructor - releases dynamically allocated Region array
Irrigator::~Irrigator(){
  delete[] m_heap;  // deletes the array
}

// addRegion - inserts a region into the min-heap based on regPrior
bool Irrigator::addRegion(Region & aRegion){
  // check capacity
  if (m_size >= m_capacity - 1) {
    // heap is full
    return false;
  }
  
  m_size++;
  // insert the region at the end of the array
  // copy assignment into an array slot
  m_heap[m_size] = aRegion;
  int index = m_size;
  

  // sift-up to restore heap property
  while (index > ROOTINDEX) {
    int parent = index / 2;

    // compare priorities: higher priority should bubble up
    if (m_heap[index].getRegPrior() < m_heap[parent].getRegPrior()) {
      swapValues(m_heap[index], m_heap[parent]);
      index = parent;
    }
    else {
      break;
    }
  }

  return true;
}

// getRegion - removes and returns the region with the smallest regPrior
bool Irrigator::getRegion(Region & aRegion){
  // return false if heap is empty
  if (m_size == 0) {
    // nothing to dequeue
    return false;
  }

  // save the root region (lowest regPrior)
  aRegion = m_heap[ROOTINDEX];

  // replace the root with the last element
  m_heap[ROOTINDEX] = m_heap[m_size];
  m_size--;

  // sift-down from the root to restore min-heap property
  int index = ROOTINDEX;
  while (true) {
    // gets the left and right child of the parent (index)
    int left = 2 * index;       
    int right = 2 * index + 1;

    // assumes current node is smallest
    int smallest = index;

    // if left child exists and has smaller regPrior, update smallest
    if (left <= m_size && m_heap[left].getRegPrior() < m_heap[smallest].getRegPrior()) {
      smallest = left;
    }
    // if right child exists and has smaller regPrior, update smallest
    if (right <= m_size && m_heap[right].getRegPrior() < m_heap[smallest].getRegPrior()) {
      smallest = right;
    }

    // if either child is smaller, swap with the smallest child and continue sifting down
    if (smallest != index) {
      swapValues(m_heap[index], m_heap[smallest]);
      index = smallest;
    }
    else {
      // heap property restored; stop sifting
      break;
    }
  }

  return true;
}

// getNthRegion - removes and returns the nth region (by regPrior order) from the min-heap
// returns true if successful, false if n is out of range.
bool Irrigator::getNthRegion(Region & aRegion, int n){
  // validate the input is within the range
  if (n <= 0 || n > m_size) {
    return false;
  }

  // allocate a temp array to hold removed regions
  Region* temp = new Region[n];

  // extract n regions
  for (int i = 1; i <= n; i++) {
    Region r;
    getRegion(r);   // removes the smallest region
    temp[i-1] = r;  // store it in a temp array

    if (i == n) {
      aRegion = r;  // this is the nth region
    }
  }
  
  // reinsert all except the nth
  for (int i = 0; i < n - 1; i++) {
    addRegion(temp[i]);
  }

  // clean up temp array
  delete[] temp;
  return true;
}

void Irrigator::dump(){
    dump(ROOTINDEX);
    cout << endl;
}

void Irrigator::dump(int index){
  if (index <= m_size){
    cout << "(";
    dump(index*2);
    cout << m_heap[index].m_regPrior;
    dump(index*2 + 1);
    cout << ")";
  }
}

// setPriorityFn - updates the priority function and heap type of the nth region
// returns true if successful, false if n is out of range.
bool Irrigator::setPriorityFn(prifn_t priFn, HEAPTYPE heapType, int n){
  // validate n
  Region target;
  if (!getNthRegion(target, n)) {
    return false;   // nth region does not exist
  }

  // update the heap type and priority funcion into the target region
  target.setPriorityFn(priFn, heapType);

  // reinsert the modified region back into the heap
  addRegion(target);

  return true;
}

// setStructure - updates the heap structure of the nth region
// returns true if successful, false if n is out of range.
bool Irrigator::setStructure(STRUCTURE structure, int n){
  // validate n
  Region target;
  if (!getNthRegion(target, n)) {
    return false;   // nth region does not exist
  }

  // convert heap structure into the target region
  target.setStructure(structure);
  
  // reinsert the modified region back into the heap
  addRegion(target);

  return true;
}

// getCrop - removes and returns the next available Crop from the heap of regions.
// returns true if a crop was successfully retrieved, false if the heap is empty.
bool Irrigator::getCrop(Crop & aCrop){
  // checks if there is regions in the heap, returns false if there is none
  if (m_size == 0) {
    return false;
  }

  // remove the region with the smallest regPrior (top of the heap)
  Region topRegion;
  getRegion(topRegion);

  // if this region has no crops, skip it and try again recursively
  if (topRegion.numCrops() == 0) {
    // skip empty region
    return getCrop(aCrop);
  }

  // extract the next crop from the region
  aCrop = topRegion.getNextCrop();

  // if the region still has crops left, reinsert it into the heap
  if (topRegion.numCrops() > 0) {
    addRegion(topRegion);
  }

  return true;
  
}

/******************************************
* Private function *
******************************************/
// swaps two Region reference
void Irrigator::swapValues(Region &a, Region &b) {
  Region temp = a;  // temp stores a
  a = b;            // set a to b
  b = temp;         // set b to a (temp)
}

