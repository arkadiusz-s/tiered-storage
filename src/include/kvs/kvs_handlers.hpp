#ifndef __KVS_HANDLERS_H__
#define __KVS_HANDLERS_H__

#include "spdlog/spdlog.h"
#include "utils/server_utility.hpp"

void node_join_handler(
    unsigned int thread_num, unsigned thread_id, unsigned& seed, Address ip,
    std::shared_ptr<spdlog::logger> logger, zmq::socket_t* join_puller,
    std::unordered_map<unsigned, GlobalHashRing>& global_hash_ring_map,
    std::unordered_map<unsigned, LocalHashRing>& local_hash_ring_map,
    std::unordered_map<std::string, KeyStat>& key_stat_map,
    std::unordered_map<std::string, KeyInfo>& placement,
    std::unordered_set<std::string>& join_remove_set, SocketCache& pushers,
    ServerThread& wt, AddressKeysetMap& join_addr_keyset_map);

void node_depart_handler(
    unsigned int thread_num, unsigned thread_id, Address ip,
    std::unordered_map<unsigned, GlobalHashRing>& global_hash_ring_map,
    std::shared_ptr<spdlog::logger> logger, zmq::socket_t* depart_puller,
    SocketCache& pushers);

void self_depart_handler(
    unsigned thread_num, unsigned thread_id, unsigned& seed, Address ip,
    std::shared_ptr<spdlog::logger> logger, zmq::socket_t* self_depart_puller,
    std::unordered_map<unsigned, GlobalHashRing>& global_hash_ring_map,
    std::unordered_map<unsigned, LocalHashRing>& local_hash_ring_map,
    std::unordered_map<std::string, KeyStat>& key_stat_map,
    std::unordered_map<std::string, KeyInfo>& placement,
    std::vector<Address>& routing_address,
    std::vector<Address>& monitoring_address, ServerThread& wt,
    SocketCache& pushers, Serializer* serializer);

void user_request_handler(
    unsigned& total_access, unsigned& seed, zmq::socket_t* request_puller,
    std::chrono::system_clock::time_point& start_time,
    std::unordered_map<unsigned, GlobalHashRing>& global_hash_ring_map,
    std::unordered_map<unsigned, LocalHashRing>& local_hash_ring_map,
    std::unordered_map<std::string, KeyStat>& key_stat_map,
    PendingMap<PendingRequest>& pending_request_map,
    std::unordered_map<
        std::string,
        std::multiset<std::chrono::time_point<std::chrono::system_clock>>>&
        key_access_timestamp,
    std::unordered_map<std::string, KeyInfo>& placement,
    std::unordered_set<std::string>& local_changeset, ServerThread& wt,
    Serializer* serializer, SocketCache& pushers);

void gossip_handler(
    unsigned& seed, zmq::socket_t* gossip_puller,
    std::unordered_map<unsigned, GlobalHashRing>& global_hash_ring_map,
    std::unordered_map<unsigned, LocalHashRing>& local_hash_ring_map,
    std::unordered_map<std::string, KeyStat>& key_stat_map,
    PendingMap<PendingGossip>& pending_gossip_map,
    std::unordered_map<std::string, KeyInfo>& placement, ServerThread& wt,
    Serializer* serializer, SocketCache& pushers);

void rep_factor_response_handler(
    unsigned& seed, unsigned& total_access,
    std::shared_ptr<spdlog::logger> logger,
    zmq::socket_t* rep_factor_response_puller,
    std::chrono::system_clock::time_point& start_time,
    std::unordered_map<unsigned, TierData>& tier_data_map,
    std::unordered_map<unsigned, GlobalHashRing>& global_hash_ring_map,
    std::unordered_map<unsigned, LocalHashRing>& local_hash_ring_map,
    PendingMap<PendingRequest>& pending_request_map,
    PendingMap<PendingGossip>& pending_gossip_map,
    std::unordered_map<
        std::string,
        std::multiset<std::chrono::time_point<std::chrono::system_clock>>>&
        key_access_timestamp,
    std::unordered_map<std::string, KeyInfo>& placement,
    std::unordered_map<std::string, KeyStat>& key_stat_map,
    std::unordered_set<std::string>& local_changeset, ServerThread& wt,
    Serializer* serializer, SocketCache& pushers);

void rep_factor_change_handler(
    Address ip, unsigned thread_id, unsigned thread_num, unsigned& seed,
    std::shared_ptr<spdlog::logger> logger,
    zmq::socket_t* rep_factor_change_puller,
    std::unordered_map<unsigned, GlobalHashRing>& global_hash_ring_map,
    std::unordered_map<unsigned, LocalHashRing>& local_hash_ring_map,
    std::unordered_map<std::string, KeyInfo>& placement,
    std::unordered_map<std::string, KeyStat>& key_stat_map,
    std::unordered_set<std::string>& local_changeset, ServerThread& wt,
    Serializer* serializer, SocketCache& pushers);

void send_gossip(AddressKeysetMap& addr_keyset_map, SocketCache& pushers,
                 Serializer* serializer);

std::pair<ReadCommittedPairLattice<std::string>, unsigned> process_get(
    const std::string& key, Serializer* serializer);

void process_put(const std::string& key, const unsigned long long& timestamp,
                 const std::string& value, Serializer* serializer,
                 std::unordered_map<std::string, KeyStat>& key_stat_map);
#endif
