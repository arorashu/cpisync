//
// Created by Bowen Song on 9/27/18.
//
#include "kshinglingSyncPerfTest.h"

// K-Shingling method does not scale because as string size increase,
// the lenght of each shingle needs to increase inorder to compensate the expoential time of baktracking
// In addition, as shingle size increase, the size for each element increases thus increase the communicaotn cost.
// At last, as shingle size increaases, the amount of differencecs between two shinlge set increases, due to shingle's redundancy
// set size increase as string sizxe increase.

CPPUNIT_TEST_SUITE_REGISTRATION(KshingleSyncPerf);
KshingleSyncPerf::KshingleSyncPerf() = default;
KshingleSyncPerf::~KshingleSyncPerf() = default;

// DEFAULT SETTINGS
const int shingleLen = 4;
const int editDist = 20;
const int strSize = 5;

//const pair<int,int> strSizeRange = make_pair(500, 50000);
//const pair<int,int> shingleLenRange = make_pair(2,ceil(log2(strSizeRange.second)));

const int tesPts = 1;// Test Pts per graph
const int target_confidence = 1;// Confidence interval
const int confidenceCap = 40; // after edit distance exceed confidenceCap, confidence go to 1.

//const pair<int,int> editDistRange = make_pair(1, 1000); // range of edit distance
// Declear what set reconciliation we are testing
//auto setReconProto = {GenSync::SyncProtocol::CPISync,GenSync::SyncProtocol::InteractiveCPISync
//                      ,GenSync::SyncProtocol::IBLTSyncSetDiff};
auto setReconProto = {GenSync::SyncProtocol::IBLTSyncSetDiff};
// Declear what string reconciliation we are testing



void KshingleSyncPerf::kshingleTest3D(){
    vector<int> editDistRange;
//   vector<int> strSizeRange = {400, 600, 800, 1000, 1400, 1800};//, 2000, 2200, 2600, 3000};//, 5000, 7000, 9000, 10000};
//
//    test.kshingle3D(GenSync::SyncProtocol::CPISync,editDistRange,strSizeRange,target_confidence, randSampleTxt);
//    test.kshingle3D(GenSync::SyncProtocol::InteractiveCPISync,editDistRange,strSizeRange,target_confidence, randSampleTxt);
//    test.kshingle3D(GenSync::SyncProtocol::IBLTSyncSetDiff,editDistRange,strSizeRange,target_confidence, randSampleTxt);


    vector<int> strSizeRange = {400, 600, 800, 1000, 1400, 1800};//, 5000, 7000, 9000, 10000};
    kshingle3D(GenSync::SyncProtocol::CPISync, editDistRange, strSizeRange, tesPts, target_confidence, randAsciiStr, 8002);


    kshingle3D(GenSync::SyncProtocol::InteractiveCPISync,editDistRange,strSizeRange,tesPts, target_confidence, randAsciiStr, 8003);
    

    kshingle3D(GenSync::SyncProtocol::IBLTSyncSetDiff,editDistRange,strSizeRange,tesPts, target_confidence, randAsciiStr, 8004);

    
//     vector<int> strSizeRange = {200};
//   test.kshingle3D(GenSync::SyncProtocol::CPISync,editDistRange,strSizeRange,target_confidence, randSampleTxt);
//    test.kshingle3D(GenSync::SyncProtocol::InteractiveCPISync,editDistRange,strSizeRange,target_confidence, randSampleTxt);
//    test.kshingle3D(GenSync::SyncProtocol::IBLTSyncSetDiff,editDistRange,strSizeRange,target_confidence, randSampleTxt);


//    PerformanceData test3 = PerformanceData(tesPts);
//    test3.kshingleBook3D(editDistRange,strSizeRange);
//    test3.genReport("kshingleBookTest3D");
//
//    PerformanceData test2 = PerformanceData(tesPts);
//    test2.kshingleCode3D(editDistRange,strSizeRange);
//    test2.genReport("kshingleTestCode3D");


}

void KshingleSyncPerf::setsofcontent3D() {
clock_t start;
    vector<int> editDistRange;
//   vector<int> strSizeRange = {400, 600, 800, 1000, 1400, 1800};//, 2000, 2200, 2600, 3000};//, 5000, 7000, 9000, 10000};
//
//    test.kshingle3D(GenSync::SyncProtocol::CPISync,editDistRange,strSizeRange,target_confidence, randSampleTxt);
//    test.kshingle3D(GenSync::SyncProtocol::InteractiveCPISync,editDistRange,strSizeRange,target_confidence, randSampleTxt);
//    test.kshingle3D(GenSync::SyncProtocol::IBLTSyncSetDiff,editDistRange,strSizeRange,target_confidence, randSampleTxt);


    vector<int> strSizeRange = {100000};//, 400000, 800000, 1000000, 1400000, 1800000};
    thread t1 (setsofcontent, GenSync::SyncProtocol::InteractiveCPISync, editDistRange, strSizeRange, tesPts,
              target_confidence, randSampleTxt, 8002);
//    t1.join();
t1.detach();


    strSizeRange = {100000};
    thread t2 (setsofcontent, GenSync::SyncProtocol::InteractiveCPISync, editDistRange, strSizeRange, tesPts,
              target_confidence, randSampleCode, 8003);
//    t2.join();
t2.detach();
bool isDone = false;
    while (!isDone) {
        try {
            t1.join();
            t2.join();
            isDone = true;
        } catch (exception) {}
    }
cout<<"My Total Time: "<<(double) (clock() - start) / CLOCKS_PER_SEC<<endl;
//    std::this_thread::sleep_for (std::chrono::seconds(50));
}

void KshingleSyncPerf::testStrataEst3D() {
    strataEst3D(make_pair(1000,100000), tesPts, target_confidence);
}