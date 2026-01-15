// mytest.cpp
// student: Andrew Soth
// professor: Kartchner

#include "irrigator.h"
#include <stdexcept>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <random>
using namespace std;

// ------------------------------
// Random class copied for testing
// ------------------------------
enum RANDOM {UNIFORMINT, UNIFORMREAL, NORMAL, SHUFFLE};
class Random {
public:
    Random(){}
    Random(int min, int max, RANDOM type=UNIFORMINT, int mean=50, int stdev=20) : m_min(min), m_max(max), m_type(type)
    {
        if (type == NORMAL){
            //the case of NORMAL to generate integer numbers with normal distribution
            m_generator = mt19937(m_device());
            //the data set will have the mean of 50 (default) and standard deviation of 20 (default)
            //the mean and standard deviation can change by passing new values to constructor 
            m_normdist = normal_distribution<>(mean,stdev);
        }
        else if (type == UNIFORMINT) {
            //the case of UNIFORMINT to generate integer numbers
            // Using a fixed seed value generates always the same sequence
            // of pseudorandom numbers, e.g. reproducing scientific experiments
            // here it helps us with testing since the same sequence repeats
            m_generator = mt19937(10);  // 10 is the fixed seed value
            m_unidist = uniform_int_distribution<>(min,max);
        }
        else if (type == UNIFORMREAL) { //the case of UNIFORMREAL to generate real numbers
            m_generator = mt19937(10);  // 10 is the fixed seed value
            m_uniReal = uniform_real_distribution<double>((double)min,(double)max);
        }
        else {  //the case of SHUFFLE to generate every number only once
            m_generator = mt19937(m_device());
        }
    }
    void setSeed(int seedNum){ 
        // we have set a default value for seed in constructor
        // we can change the seed by calling this function after constructor call
        // this gives us more randomness
        m_generator = mt19937(seedNum); 
    }
    void init(int min, int max){
        m_min = min; 
        m_max = max; 
        m_type = UNIFORMINT;
        m_generator = mt19937(10);  // 10 is the fixed seed value
        m_unidist = uniform_int_distribution<>(min,max);
    }
    void getShuffle(vector<int> & array){
        // this function provides a list of all values between min and max
        // in a random order, this function guarantees the uniqueness
        // of every value in the list
        // the user program creates the vector param and passes here
        // here we populate the vector using m_min and m_max
        for (int i = m_min; i<=m_max; i++){ array.push_back(i); }
        shuffle(array.begin(),array.end(),m_generator);
    }

    void getShuffle(int array[]){
        // this function provides a list of all values between min and max
        // in a random order, this function guarantees the uniqueness
        // of every value in the list
        // the param array must be of the size (m_max-m_min+1)
        // the user program creates the array and pass it here
        vector<int> temp;
        for (int i = m_min; i<=m_max; i++){ 
            temp.push_back(i); 
        }
        shuffle(temp.begin(), temp.end(), m_generator);
        vector<int>::iterator it;
        int i = 0;
        for (it=temp.begin(); it != temp.end(); it++){
            array[i] = *it;
            i++;
        }
    }

    int getRandNum(){
        // this function returns integer numbers
        // the object must have been initialized to generate integers
        int result = 0;
        if(m_type == NORMAL){
            //returns a random number in a set with normal distribution
            //we limit random numbers by the min and max values
            result = m_min - 1;
            while(result < m_min || result > m_max)
                result = m_normdist(m_generator);
        }
        else if (m_type == UNIFORMINT){
            //this will generate a random number between min and max values
            result = m_unidist(m_generator);
        }
        return result;
    }
    double getRealRandNum(){
        // this function returns real numbers
        // the object must have been initialized to generate real numbers
        double result = m_uniReal(m_generator);
        // a trick to return numbers only with two deciaml points
        // for example if result is 15.0378, function returns 15.03
        // to round up we can use ceil function instead of floor
        result = floor(result*100.0)/100.0;
        return result;
    }
    string getRandString(int size){
        // the parameter size specifies the length of string we ask for
        // to use ASCII char the number range in constructor must be set to 97 - 122
        // and the Random type must be UNIFORMINT (it is default in constructor)
        string output = "";
        for (int i=0;i<size;i++){
            output = output + (char)getRandNum();
        }
        return output;
    }


    int getMin(){return m_min;}
    int getMax(){return m_max;}
private:
    int m_min;
    int m_max;
    RANDOM m_type;
    random_device m_device;
    mt19937 m_generator;
    normal_distribution<> m_normdist; //normal distribution
    uniform_int_distribution<> m_unidist; //integer uniform distribution
    uniform_real_distribution<double> m_uniReal; //real uniform distribution
};

// ------------------------------
// Priority functions
// ------------------------------
int priorityFn1(const Crop &crop) {
    //needs MAXHEAP
    //priority value is determined based on some criteria
    //priority value falls in the range [30-116]
    //the highest priority would be 110+6 = 116
    //the lowest priority would be 30+0 = 30
    //the larger value means the higher priority
    int minValue = 30;
    int maxValue = 116;
    int priority = crop.getTemperature() + crop.getType();
    if (priority >= minValue && priority <= maxValue) {
        return priority;
    }
    else {
        return 0; // this is an invalid order object
    }
}
int priorityFn2(const Crop &crop) {
    //needs MINHEAP
    //priority value is determined based on some criteria
    //priority value falls in the range [1-103]
    //the highest priority would be 1+0 = 1
    //the lowest priority would be 100+3 = 103
    //the smaller value means the higher priority
    int minValue = 1;
    int maxValue = 103;
    int priority = crop.getMoisture() + crop.getTime();
    if (priority >= minValue && priority <= maxValue) {
        return priority;
    }
    else {
        return 0; // this is an invalid order object
    }
}

// ------------------------------
// Tester with helpers and tests
// ------------------------------
class Tester {
public:
    // ---------- Helpers ----------
    // Access Region internals via friendship (Region, Crop declare Tester as friend)

    // Validates that the heap rooted at reg.m_heap satisfies the ordering constraints
    // imposed by reg.m_priorFunc and reg.m_heapType
    static bool checkHeapProperty(const Region& reg){
        if (reg.m_size == 0) return true;
        return checkNode(reg, reg.m_heap, reg.m_priorFunc, reg.m_heapType);
    }

    // Removes all crops from a copy of the region and verofoes that the priorities
    // are returned in a sorted order according to the heap type. Ensures complete
    // removal and empty heap at the end
    static bool checkRemovalOrder(Region regCopy){
        if (regCopy.m_size == 0) return true;

        // Store original size to verify all removed
        int originalSize = regCopy.m_size;
        vector<int> seq;

        // Extract all crops and record priorities
        for (int i=0; i < originalSize; i++){
            try {
                Crop c = regCopy.getNextCrop();
                int p = regCopy.m_priorFunc(c);
                seq.push_back(p);
            } catch (...) {
                return false; // Failed to extract
            }
        }

        // Verify we extracted correct number
        if ((int)seq.size() != originalSize) return false;

        // Verify heap is now empty
        if (regCopy.m_size != 0) return false;

        // Verify sequence is properly sorted
        if (regCopy.m_heapType == MINHEAP){
            for (size_t i=1; i<seq.size(); i++){
                if (seq[i-1] > seq[i]) return false;
            }
        } else {
            for (size_t i=1; i<seq.size(); i++){
                if (seq[i-1] < seq[i]) return false;
            }
        }
        return true;
    }

    // Iteratively removes crops from region copy and checks that heap property
    // is preserved after each removal
    static bool checkRemovalMaintainsHeap(Region regCopy){
        while (regCopy.m_size > 0) {
            regCopy.getNextCrop();
            if (!checkHeapProperty(regCopy)) return false;
        }
        return true;
    }

    // For LEFTIST heaps, verifies that null path length values are correctly 
    // computed for every node
    static bool checkLeftistNPLValues(const Region& reg){
        if (reg.m_structure != LEFTIST) return true;
        if (reg.m_size == 0) return true;
        return checkNPL(reg.m_heap);
    }

    // For LEFTIST heaps, ensures that leftist property holds:
    // left child's npl >= right child's npl at every node
    static bool checkLeftistProperty(const Region& reg){
        if (reg.m_structure != LEFTIST) return true;
        if (reg.m_size == 0) return true;
        return checkLeftist(reg.m_heap);
    }

    // Compares two regions to ensure they contain identical sets
    // of crop IDs after a rebuild
    static bool sameIDsAfterRebuild(const Region& before, const Region& after){
        unordered_set<int> A, B;
        collectIDs(before.m_heap, A);
        collectIDs(after.m_heap, B);
        return A == B && before.m_size == after.m_size;
    }

    // Recursively traverses a tree of Crop nodes and collects all crop IDs
    // into the provided unordered_set
    static void collectIDs(Crop* node, unordered_set<int>& out){
        if (!node) return;
        out.insert(node->m_cropID);
        collectIDs(node->m_left, out);
        collectIDs(node->m_right, out);
    }

    // Recursively counts the total number of nodes in a tree of Crop objects
    static int countNodes(Crop* node){
        if (!node) return 0;
        return 1 + countNodes(node->m_left) + countNodes(node->m_right);
    }

    // Constructs a Region populated with a specified number of Crop objects.
    // Each crop is generated with varied attributes to ensure diverse priority valuess
    static Region buildRegion(prifn_t pf, HEAPTYPE ht, STRUCTURE st, int regPrior, int count, int seed=42){
        Region r(pf, ht, st, regPrior);
        Random idGen(MINCROPID, MAXCROPID);
        Random temperatureGen(MINTEMP, MAXTEMP);
        Random moistureGen(MINMOISTURE, MAXMOISTURE);
        Random timeGen(MINTIME, MAXTIME);
        Random typeGen(MINTYPE, MAXTYPE);
        idGen.setSeed(seed);
        temperatureGen.setSeed(seed+1);
        moistureGen.setSeed(seed+2);
        timeGen.setSeed(seed+3);
        typeGen.setSeed(seed+4);

        // VARIED priorities - different for each crop
        for (int i=0; i<count; i++){
            Crop c(idGen.getRandNum(),
                   temperatureGen.getRandNum(),  // VARIED temp
                   moistureGen.getRandNum(),
                   timeGen.getRandNum(),         // VARIED time
                   typeGen.getRandNum());
            r.insertCrop(c);
        }
        return r;
    }

    // ---------- Internal helpers (recursive) ----------
    // Recursively verifies that the heap property holds for the subtree rooted at
    // the given Crop node. The check is performed using the provided priority function
    // and heap type
    static bool checkNode(const Region& reg, Crop* node, prifn_t pf, HEAPTYPE ht){
        if (!node) return true;
        int p = pf(*node);
        if (node->m_left){
            int pl = pf(*node->m_left);
            if (ht == MINHEAP){
                if (p > pl) return false;
            } else {
                if (p < pl) return false;
            }
            if (!checkNode(reg, node->m_left, pf, ht)) return false;
        }
        if (node->m_right){
            int pr = pf(*node->m_right);
            if (ht == MINHEAP){
                if (p > pr) return false;
            } else {
                if (p < pr) return false;
            }
            if (!checkNode(reg, node->m_right, pf, ht)) return false;
        }
        return true;
    }

    // Recursively verifies that the Null Path Length (NPL) values
    // are correctly maintained throughout a leftist heap.
    static bool checkNPL(Crop* node){
        if (!node) return true;
        int leftN = node->m_left ? node->m_left->m_npl : -1;
        int rightN = node->m_right ? node->m_right->m_npl : -1;
        int computed = 1 + min(leftN, rightN);

        // NPL must be correct
        if (node->m_npl != computed) return false;

        // For leaves, NPL should be 0
        if (!node->m_left && !node->m_right && node->m_npl != 0) return false;

        return checkNPL(node->m_left) && checkNPL(node->m_right);
    }

    // Recursively verifies that the leftist property holds for the
    // subtree rooted at the given Crop node. In a leftist heap, the
    // null path length (NPL) of the left child must be greater than
    // or equal to the NPL of the right child at every node.
    static bool checkLeftist(Crop* node){
        if (!node) return true;
        int leftN = node->m_left ? node->m_left->m_npl : -1;
        int rightN = node->m_right ? node->m_right->m_npl : -1;

        // Leftist property: left NPL >= right NPL
        if (leftN < rightN) return false;

        return checkLeftist(node->m_left) && checkLeftist(node->m_right);
    }

    // ---------- REGION CLASS TESTS ----------

    // Test 1: Min-heap insertion with 300 nodes
    bool testMinHeapInsertion(){
        Region r = buildRegion(priorityFn2, MINHEAP, SKEW, 10, 300, 101);
        return r.numCrops() == 300 && checkHeapProperty(r);
    }

    // Test 2: Max-heap insertion with 300 nodes
    bool testMaxHeapInsertion(){
        Region r = buildRegion(priorityFn1, MAXHEAP, SKEW, 10, 300, 102);
        return r.numCrops() == 300 && checkHeapProperty(r);
    }

    // Test 3: Min-heap removal order
    bool testMinHeapRemovalOrder(){
        Region r = buildRegion(priorityFn2, MINHEAP, SKEW, 10, 300, 103);
        return checkRemovalOrder(r);
    }

    // Test 4: Max-heap removal order
    bool testMaxHeapRemovalOrder(){
        Region r = buildRegion(priorityFn1, MAXHEAP, SKEW, 10, 300, 104);
        return checkRemovalOrder(r);
    }

    // Test 5: Leftist heap NPL values correct
    bool testLeftistNPLValues(){
        Region r = buildRegion(priorityFn2, MINHEAP, LEFTIST, 10, 300, 105);
        return checkLeftistNPLValues(r);
    }

    // Test 6: Leftist property holds
    bool testLeftistProperty(){
        Region r = buildRegion(priorityFn2, MINHEAP, LEFTIST, 10, 300, 106);
        return checkLeftistProperty(r);
    }

    // Test 7: Removal maintains heap property
    bool testRemovalMaintainsHeapProperty(){
        Region r = buildRegion(priorityFn2, MINHEAP, SKEW, 10, 100, 107);
        return checkRemovalMaintainsHeap(r);
    }

    // Test 8: Change priority function rebuild with same nodes
    bool testChangePriorityFnRebuild(){
        Region r = buildRegion(priorityFn2, MINHEAP, SKEW, 10, 200, 108);
        Region before(r);
        r.setPriorityFn(priorityFn1, MAXHEAP);
        return checkHeapProperty(r) && sameIDsAfterRebuild(before, r);
    }

    // Test 9: Change structure (SKEW to LEFTIST)
    bool testChangeStructure(){
        Region r = buildRegion(priorityFn2, MINHEAP, SKEW, 10, 150, 109);
        Region before(r);
        r.setStructure(LEFTIST);
        bool heapOK = checkHeapProperty(r);
        bool leftistOK = checkLeftistProperty(r);
        bool nplOK = checkLeftistNPLValues(r);
        bool sameNodes = sameIDsAfterRebuild(before, r);
        return heapOK && leftistOK && nplOK && sameNodes;
    }

    // Test 10: Merge empty with normal
    bool testMergeEmptyWithNormal(){
        Region empty(priorityFn2, MINHEAP, SKEW, 1);
        Region normal = buildRegion(priorityFn2, MINHEAP, SKEW, 2, 150, 110);
        int expectedSize = normal.numCrops();
        empty.mergeWithQueue(normal);
        bool hostOK = (empty.numCrops() == expectedSize) && checkHeapProperty(empty);
        bool rhsEmpty = (normal.numCrops() == 0) && (normal.m_heap == nullptr);
        return hostOK && rhsEmpty;
    }

    // Test 11: Merge normal with empty
    bool testMergeNormalWithEmpty(){
        Region normal = buildRegion(priorityFn2, MINHEAP, SKEW, 1, 120, 111);
        Region empty(priorityFn2, MINHEAP, SKEW, 2);
        int expectedSize = normal.numCrops();
        normal.mergeWithQueue(empty);
        bool hostOK = (normal.numCrops() == expectedSize) && checkHeapProperty(normal);
        bool rhsEmpty = (empty.numCrops() == 0) && (empty.m_heap == nullptr);
        return hostOK && rhsEmpty;
    }

    // Test 12: Merge two normal queues
    bool testMergeTwoNormal(){
        Region r1 = buildRegion(priorityFn2, MINHEAP, SKEW, 1, 80, 112);
        Region r2 = buildRegion(priorityFn2, MINHEAP, SKEW, 2, 70, 113);
        int expectedSize = r1.numCrops() + r2.numCrops();
        r1.mergeWithQueue(r2);
        bool hostOK = (r1.numCrops() == expectedSize) && checkHeapProperty(r1);
        bool rhsEmpty = (r2.numCrops() == 0);
        return hostOK && rhsEmpty;
    }

    // Test 13: Self-merge protection
    bool testSelfMergeProtection(){
        Region r = buildRegion(priorityFn2, MINHEAP, SKEW, 1, 50, 114);
        int originalSize = r.numCrops();
        r.mergeWithQueue(r); // Should do nothing
        return r.numCrops() == originalSize && checkHeapProperty(r);
    }

    // Test 14: Copy constructor normal
    bool testCopyConstructorNormal(){
        Region r = buildRegion(priorityFn2, MINHEAP, LEFTIST, 10, 50, 115);
        Region copy(r);
        for (int i=0;i<10;i++) r.getNextCrop();
        return (r.numCrops() == 40) && (copy.numCrops() == 50) && checkHeapProperty(copy);
    }

    // Test 15: Copy constructor edge (empty)
    bool testCopyConstructorEdgeEmpty(){
        Region r(priorityFn2, MINHEAP, LEFTIST, 10);
        Region copy(r);
        return (copy.numCrops() == 0) && (copy.m_heap == nullptr);
    }

    // Test 16: Assignment operator normal
    bool testAssignmentOperatorNormal(){
        Region r1 = buildRegion(priorityFn2, MINHEAP, SKEW, 10, 80, 116);
        Region r2 = buildRegion(priorityFn1, MAXHEAP, LEFTIST, 20, 30, 117);
        r2 = r1;
        for (int i=0;i<20;i++) r1.getNextCrop();
        return (r1.numCrops() == 60) && (r2.numCrops() == 80) && checkHeapProperty(r2);
    }

    // Test 17: Assignment operator edge (rhs empty)
    bool testAssignmentOperatorEdgeEmpty(){
        Region rhs(priorityFn2, MINHEAP, LEFTIST, 10);
        Region lhs = buildRegion(priorityFn2, MINHEAP, LEFTIST, 20, 40, 118);
        lhs = rhs;
        return (lhs.numCrops() == 0) && (lhs.m_heap == nullptr);
    }

    // Test 18: Assignment operator self-assignment
    bool testAssignmentOperatorSelf(){
        Region r = buildRegion(priorityFn2, MINHEAP, SKEW, 10, 60, 119);
        int originalSize = r.numCrops();
        r = r; // Self-assignment
        return (r.numCrops() == originalSize) && checkHeapProperty(r);
    }

    // Test 19: Dequeue empty throws out_of_range
    bool testDequeueEmptyThrows(){
        Region r(priorityFn2, MINHEAP, LEFTIST, 10);
        try {
            (void)r.getNextCrop();
            return false; // Should have thrown
        } catch (const out_of_range&) {
            return true;
        } catch (...) {
            return false;
        }
    }

    // Test 20: Merge different priority functions throws domain_error
    bool testMergeDifferentPriorityThrows(){
        Region a(priorityFn2, MINHEAP, LEFTIST, 10);
        Region b(priorityFn1, MAXHEAP, LEFTIST, 20);
        b.insertCrop(Crop(123456, 70, 30, MORNING, BEAN));
        try {
            a.mergeWithQueue(b);
            return false;
        } catch (const domain_error&) {
            return true;
        } catch (...) {
            return false;
        }
    }

    // Test 21: Merge different structures throws domain_error
    bool testMergeDifferentStructureThrows(){
        Region c(priorityFn2, MINHEAP, LEFTIST, 10);
        Region d(priorityFn2, MINHEAP, SKEW, 20);
        d.insertCrop(Crop(234567, 65, 40, NOON, MELON));
        try {
            c.mergeWithQueue(d);
            return false;
        } catch (const domain_error&) {
            return true;
        } catch (...) {
            return false;
        }
    }

    // ---------- IRRIGATOR CLASS TESTS ----------

    // Test 22: Irrigator constructor
    bool testIrrigatorConstructor(){
        Irrigator irr(10);
        return true; // If it compiles and doesn't crash, constructor works
    }

    // Test 23: Add region to Irrigator
    bool testIrrigatorAddRegion(){
        Irrigator irr(5);
        Region r1 = buildRegion(priorityFn2, MINHEAP, SKEW, 10, 20, 120);
        Region r2 = buildRegion(priorityFn2, MINHEAP, SKEW, 5, 15, 121);
        return irr.addRegion(r1) && irr.addRegion(r2);
    }

    // Test 24: Get region from Irrigator
    bool testIrrigatorGetRegion(){
        Irrigator irr(5);
        Region r1 = buildRegion(priorityFn2, MINHEAP, SKEW, 20, 25, 122);
        Region r2 = buildRegion(priorityFn2, MINHEAP, SKEW, 10, 30, 123);
        irr.addRegion(r1);
        irr.addRegion(r2);

        Region retrieved;
        bool result = irr.getRegion(retrieved);
        // Should get region with priority 10 (lower priority = higher in min-heap)
        return result && (retrieved.m_regPrior == 10);
    }

    // Test 25: Get Nth region
    bool testIrrigatorGetNthRegion(){
        Irrigator irr(10);
        for (int i = 1; i <= 5; i++){
            Region r = buildRegion(priorityFn2, MINHEAP, SKEW, i*10, 10, 124+i);
            irr.addRegion(r);
        }

        Region second;
        bool result = irr.getNthRegion(second, 2);
        // Should get 2nd lowest priority
        return result && (second.m_regPrior == 20);
    }

    // Test 26: Get crop from Irrigator
    bool testIrrigatorGetCrop(){
        Irrigator irr(5);
        Region r = buildRegion(priorityFn2, MINHEAP, SKEW, 10, 50, 130);
        irr.addRegion(r);

        Crop c;
        bool result = irr.getCrop(c);
        return result && (c.getCropID() >= MINCROPID && c.getCropID() <= MAXCROPID);
    }

    // Test 27: Get crop removes empty regions
    bool testIrrigatorGetCropRemovesEmpty(){
        Irrigator irr(5);

        // Add empty region with higher priority
        Region empty(priorityFn2, MINHEAP, SKEW, 5);
        irr.addRegion(empty);

        // Add region with crops
        Region withCrops = buildRegion(priorityFn2, MINHEAP, SKEW, 10, 20, 131);
        irr.addRegion(withCrops);

        Crop c;
        bool result = irr.getCrop(c);

        // Should successfully get crop (empty region removed)
        return result;
    }

    // Test 28: Set priority function on Nth region in Irrigator
    bool testIrrigatorSetPriorityFn(){
        Irrigator irr(5);
        Region r = buildRegion(priorityFn2, MINHEAP, SKEW, 10, 50, 132);
        irr.addRegion(r);

        bool result = irr.setPriorityFn(priorityFn1, MAXHEAP, 1);
        return result;
    }

    // Test 29: Set structure on Nth region in Irrigator
    bool testIrrigatorSetStructure(){
        Irrigator irr(5);
        Region r = buildRegion(priorityFn2, MINHEAP, SKEW, 10, 50, 133);
        irr.addRegion(r);

        bool result = irr.setStructure(LEFTIST, 1);
        return result;
    }
};

// ------------------------------
// Main: run all 29 tests
// ------------------------------
int main(){
    Tester T;
    int passed = 0;
    int total = 29;

    cout << "========================================" << endl;
    cout << "  COMPREHENSIVE TEST SUITE - PROJECT 3" << endl;
    cout << "========================================" << endl << endl;

    cout << "REGION CLASS TESTS:" << endl;
    cout << "1. Min-heap insertion (300 nodes): " << (T.testMinHeapInsertion() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "2. Max-heap insertion (300 nodes): " << (T.testMaxHeapInsertion() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "3. Min-heap removal order: " << (T.testMinHeapRemovalOrder() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "4. Max-heap removal order: " << (T.testMaxHeapRemovalOrder() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "5. Leftist NPL values correct: " << (T.testLeftistNPLValues() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "6. Leftist property holds: " << (T.testLeftistProperty() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "7. Removal maintains heap property: " << (T.testRemovalMaintainsHeapProperty() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "8. Change priority function rebuild: " << (T.testChangePriorityFnRebuild() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "9. Change structure (SKEW to LEFTIST): " << (T.testChangeStructure() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "10. Merge empty with normal: " << (T.testMergeEmptyWithNormal() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "11. Merge normal with empty: " << (T.testMergeNormalWithEmpty() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "12. Merge two normal queues: " << (T.testMergeTwoNormal() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "13. Self-merge protection: " << (T.testSelfMergeProtection() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "14. Copy constructor normal: " << (T.testCopyConstructorNormal() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "15. Copy constructor edge (empty): " << (T.testCopyConstructorEdgeEmpty() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "16. Assignment operator normal: " << (T.testAssignmentOperatorNormal() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "17. Assignment operator edge (empty): " << (T.testAssignmentOperatorEdgeEmpty() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "18. Assignment operator self-assignment: " << (T.testAssignmentOperatorSelf() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "19. Dequeue empty throws out_of_range: " << (T.testDequeueEmptyThrows() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "20. Merge different priority throws: " << (T.testMergeDifferentPriorityThrows() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "21. Merge different structure throws: " << (T.testMergeDifferentStructureThrows() ? (passed++, "PASSED") : "FAILED") << endl;

    cout << endl << "IRRIGATOR CLASS TESTS:" << endl;
    cout << "22. Irrigator constructor: " << (T.testIrrigatorConstructor() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "23. Add region to Irrigator: " << (T.testIrrigatorAddRegion() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "24. Get region from Irrigator: " << (T.testIrrigatorGetRegion() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "25. Get Nth region: " << (T.testIrrigatorGetNthRegion() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "26. Get crop from Irrigator: " << (T.testIrrigatorGetCrop() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "27. Get crop removes empty regions: " << (T.testIrrigatorGetCropRemovesEmpty() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "28. Set priority function on region: " << (T.testIrrigatorSetPriorityFn() ? (passed++, "PASSED") : "FAILED") << endl;
    cout << "29. Set structure on region: " << (T.testIrrigatorSetStructure() ? (passed++, "PASSED") : "FAILED") << endl;

    cout << endl << "========================================" << endl;
    cout << "       FINAL TEST RESULTS" << endl;
    cout << "========================================" << endl;
    cout << "Tests passed: " << passed << "/" << total << endl;

    if (passed == total) {
        cout << "STATUS: ALL TESTS PASSED!" << endl;
    } else {
        cout << "STATUS: " << (total - passed) << " TEST(S) FAILED" << endl;
    }
    cout << "========================================" << endl;

    return 0;
}
