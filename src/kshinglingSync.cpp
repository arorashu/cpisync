//
// Created by Bowen Song on 9/23/18.
//

#include <Exceptions.h>
#include "kshinglingSync.h"


//kshinglingSync::kshinglingSync(GenSync::SyncProtocol sync_protocol,GenSync::SyncComm sync_comm,
//                               size_t symbol_size , int m_bar, int num_Parts, int num_ExpElem, int port_num) {
//    setSyncProtocol = sync_protocol;
//    setSyncComm = sync_comm;
//    mbar = m_bar; // 4 is constant if front or/and back is modified
//    bits = symbol_size;//*(k+2);  // ascii symbol size is 8
//    cycleNum = -1;
//    numParts = num_Parts;
//    numExpElem = num_ExpElem;
//    portNum = port_num;
//}
//kshinglingSync::~kshinglingSync() = default;
//
//
//GenSync kshinglingSync::SyncHost(K_Shingle& host_content) {
//
//    auto Set = host_content.getShingleSet();
//    auto cyc = to_string(host_content.reconstructStringBacktracking().second); // get the cycle number of the string
//
//    GenSync host = GenSync::Builder().
//            setProtocol(setSyncProtocol).
//            setComm(setSyncComm).
//            setMbar(mbar).
//            setNumPartitions(numParts).
//            setBits(bits).
//            setPort(portNum).
//            setNumExpectedElements(numExpElem).
//            build();
//
//    for (auto tmp : Set) {
//        host.addElem(&tmp);
//    }
//    host.addElem(&cyc);
//    return host;
//}
//
//
//forkHandleReport kshinglingSync::SyncNreport(GenSync &server, GenSync client){
//    return forkHandle(server,client, false); // by default we want to get rid of Extra elements : flag false
//}
//
//string kshinglingSync::getString(GenSync host,K_Shingle& host_content){
//    auto content = host.dumpElements();
//    auto tmpnum = -1;
//    host_content.clear_shingleSet();
//    for (auto dop : content){
//        tmpnum = dop->to_string().find_last_of(":");
//        if(tmpnum<0){
//            cycleNum = stoi(dop->to_string());
//        }else {
//            host_content.insert(dop);
//        }
//    }
//    if(cycleNum==0){
//        return "";
//    }
//    return host_content.reconstructStringBacktracking(cycleNum).first;
//}



kshinglingSync::kshinglingSync(GenSync::SyncProtocol set_sync_protocol, const size_t shingle_size,
        const char stop_word) : myKshingle(shingle_size, stop_word), setSyncProtocol(set_sync_protocol), shingleSize(shingle_size){
    oneway = true;
}


//Alice
bool kshinglingSync::SyncClient(const shared_ptr<Communicant> &commSync, shared_ptr<SyncMethod> & setHost,
        DataObject &selfString, DataObject &otherString, bool Estimate) {
    Logger::gLog(Logger::METHOD, "Entering kshinglingSync::SyncClient");
    bool syncSuccess = true;

    // call parent method for bookkeeping
    SyncMethod::SyncClient(commSync, setHost, selfString, otherString);
    // create kshingle

    // connect to server
    commSync->commConnect();
    // ensure that the kshingle size and stopword equal those of the server
    if (!commSync->establishKshingleSend(myKshingle.getElemSize(), myKshingle.getStopWord(), oneway)) {
        Logger::gLog(Logger::METHOD_DETAILS,
                     "Kshingle parameters do not match up between client and server!");
        syncSuccess = false;
    }

    // send cycNum
    if (!oneway)commSync->commSend(cycleNum);
    cycleNum = commSync->commRecv_long();


    // estimate difference
    if (Estimate and needEst()) {
        StrataEst est = StrataEst(myKshingle.getElemSize());

        for (auto item : myKshingle.getShingleSet_str()) {
            est.insert(new DataObject(item)); // Add to estimator
        }

        // since Kshingling are the same, Strata Est parameters would also be the same.
        commSync->commSend(est.getStrata(), false);

        mbar = commSync->commRecv_long(); // cast long to long long

    }

    // reconcile difference + delete extra
    configurate(setHost, myKshingle.getSetSize());
    for (auto item : myKshingle.getShingleSet_str()) {
        setHost->addElem(new DataObject(item)); // Add to GenSync
    }
    // choose to send if not oneway (default is one way)
//

    return syncSuccess;
}

//Bob
bool kshinglingSync::SyncServer(const shared_ptr<Communicant> &commSync,  shared_ptr<SyncMethod> & setHost,
        DataObject &selfString, DataObject &otherString, bool Estimate) {
    Logger::gLog(Logger::METHOD, "Entering kshinglingSync::SyncServer");
    bool syncSuccess = true;

    SyncMethod::SyncServer(commSync, setHost, selfString, otherString);

    commSync->commListen();
    if (!commSync->establishKshingleRecv(myKshingle.getElemSize(), myKshingle.getStopWord(), oneway)) {
        Logger::gLog(Logger::METHOD_DETAILS,
                     "Kshingle parameters do not match up between client and server!");
        syncSuccess = false;
    }

    // send cycNum
    auto tmpcycleNum = cycleNum;
    if (!oneway) cycleNum = commSync->commRecv_long();
     commSync->commSend(tmpcycleNum);

    // estimate difference
    if (Estimate and needEst()) {
        StrataEst est = StrataEst(myKshingle.getElemSize());

        for (auto item : myKshingle.getShingleSet_str()) {
            est.insert(new DataObject(item)); // Add to estimator
        }

        // since Kshingling are the same, Strata Est parameters would also be the same.
        auto theirStarata = commSync->commRecv_Strata();
        mbar = (est -= theirStarata).estimate();
        mbar = mbar + mbar / 2; // get an upper bound
        commSync->commSend(mbar); // Dangerous cast

    }

    // reconcile difference + delete extra
    configurate(setHost, myKshingle.getSetSize());
    for (auto item : myKshingle.getShingleSet_str()) {
        setHost->addElem(new DataObject(item)); // Add to GenSync
    }

//    // since using forkHandle only
//    signal(SIGCHLD, SIG_IGN);
//    myHost.listenSync(0, false);
    return syncSuccess;
}

void kshinglingSync::configurate(shared_ptr<SyncMethod>& setHost, idx_t set_size) {


    if (setSyncProtocol == GenSync::SyncProtocol::CPISync) {
        eltSize = 14 + (myKshingle.getshinglelen_str() + 2) * 6;
        int err = 8;// negative log of acceptable error probability for probabilistic syncs
        setHost = make_shared<ProbCPISync>(mbar, eltSize, err);
    } else if (setSyncProtocol == GenSync::SyncProtocol::InteractiveCPISync) {
        eltSize = 14 + (myKshingle.getshinglelen_str() + 2) * 6;
        //make_shared<InterCPISync>(mBar, eltSizeSq, err, (ceil(log(set_size))>1)?:2)
        //(ceil(log(set_size))>1)?:2;
    } else if (setSyncProtocol == GenSync::SyncProtocol::IBLTSyncSetDiff) {
        (mbar == 0)? mbar = 10 : mbar;
        eltSize = myKshingle.getElemSize();
        setHost = make_shared<IBLTSync_SetDiff>(mbar, eltSize, true);
    }
}

bool kshinglingSync::reconstructString(DataObject* & recovered_string, const list<DataObject *> & Elems) {
    if (cycleNum != 0)
        myKshingle.clear_shingleSet();

        for (auto elem: Elems) {
            //change here - send pair
            myKshingle.updateShingleSet_str(ZZtoStr(elem->to_ZZ()));
        }
        recovered_string = new DataObject(myKshingle.reconstructStringBacktracking(cycleNum).first);
    return cycleNum != 0;
}

vector<DataObject*> kshinglingSync::addStr(DataObject* datum){
    // call parent add
    SyncMethod::addStr(datum);
    myKshingle.clear_shingleSet();

    myKshingle.inject(datum->to_string());
    cycleNum = myKshingle.reconstructStringBacktracking().second;
    vector<DataObject*> res;
    for (auto item : myKshingle.getShingleSet_str()){
        auto tmp = new DataObject(StrtoZZ(item));
        res.push_back(tmp);
    }
    return res;
}

long kshinglingSync::getVirMem(){
    return myKshingle.virtualMemUsed();
}

//size_t kshinglingSync::injectString(string str) {
//    myKshingle.inject(str);
//    for (auto item : myKshingle.getShingleSet_str()) addElem(new DataObject(item));
//    return myKshingle.reconstructStringBacktracking().second;
//}

string kshinglingSync::getName(){ return "This is a kshinglingSync of string reconciliation";}