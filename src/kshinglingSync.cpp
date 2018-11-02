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
        const char stop_word) : myKshingle(shingle_size, stop_word), setSyncProtocol(set_sync_protocol) {
    oneway = false;

    // configure set reconciliation
    configurate();
}

bool kshinglingSync::SyncClient(const shared_ptr<Communicant> &commSync, DataObject &selfString,
        DataObject &otherString) {
    Logger::gLog(Logger::METHOD, "Entering kshinglingSync::SyncClient");

    bool syncSuccess = true;
    // call parent method for bookkeeping
    SyncMethod::SyncClient(commSync, selfString, otherString);

    // create kshingle

    // connect to server
    commSync->commConnect();
    // ensure that the kshingle size and stopword equal those of the server
    if(!commSync->establishKshingleSend(myKshingle.size(), myKshingle.eltSize(), oneway)) {
        Logger::gLog(Logger::METHOD_DETAILS,
                     "Kshingle parameters do not match up between client and server!");
        syncSuccess = false;
    }
    commSync->commSend(myKshingle, true);
    // estimate difference
    // reconcile difference + delete extra
    // choose to send if not oneway (default is one way)

    return true;
}

bool kshinglingSync::SyncServer(const shared_ptr<Communicant> &commSync, DataObject &selfString,
                                DataObject &otherString) {
    Logger::gLog(Logger::METHOD, "Entering kshinglingSync::SyncServer");

    return true;
}

void kshinglingSync::configurate(int port_num, GenSync::SyncComm setSyncComm) {
//    myhost = GenSync::Builder().
//            setProtocol(setSyncProtocol).
//            setComm(setSyncComm).
//            setMbar(mbar).
//            setNumPartitions(numParts).
//            setBits(bits).
//            setPort(portNum).
//            setNumExpectedElements(numExpElem).
//            build();
}

string kshinglingSync::getName(){ return "This is a kshinglingSync of string reconciliation";}