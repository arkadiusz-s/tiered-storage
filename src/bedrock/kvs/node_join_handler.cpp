#include "kvs/kvs_handlers.hpp"

void node_join_handler(
    unsigned thread_id, unsigned& seed, Address ip,
    std::shared_ptr<spdlog::logger> logger, zmq::socket_t* join_puller,
    std::unordered_map<unsigned, GlobalHashRing>& global_hash_ring_map,
    std::unordered_map<unsigned, LocalHashRing>& local_hash_ring_map,
    std::unordered_map<Key, unsigned>& key_size_map,
    std::unordered_map<Key, KeyInfo>& placement,
    std::unordered_set<Key>& join_remove_set, SocketCache& pushers,
    ServerThread& wt, AddressKeysetMap& join_addr_keyset_map) {
  std::string message = zmq_util::recv_string(join_puller);
  std::vector<std::string> v;
  split(message, ':', v);
  unsigned tier = stoi(v[0]);
  Address new_server_ip = v[1];

  // update global hash ring
  bool inserted = insert_to_hash_ring<GlobalHashRing>(
      global_hash_ring_map[tier], new_server_ip, 0);

  if (inserted) {
    logger->info("Received a node join for tier {}. New node is {}", tier,
                 new_server_ip);

    // only thread 0 communicates with other nodes and receives distributed
    // join messages; it communicates that information to non-0 threads on its
    // own machine
    if (thread_id == 0) {
      // send my IP to the new server node
      zmq_util::send_string(std::to_string(kSelfTierId) + ":" + ip,
                            &pushers[ServerThread(new_server_ip, 0)
                                         .get_node_join_connect_addr()]);

      // gossip the new node address between server nodes to ensure consistency
      for (const auto& global_pair : global_hash_ring_map) {
        GlobalHashRing hash_ring = global_pair.second;
        std::unordered_set<Address> observed_ip;

        for (const auto& hash_pair : hash_ring) {
          std::string this_ip = hash_pair.second.get_ip();
          // if the node is not myself and not the newly joined node, send the
          // ip of the newly joined node in case of a race condition
          if (this_ip.compare(ip) != 0 && this_ip.compare(new_server_ip) != 0 &&
              observed_ip.find(this_ip) == observed_ip.end()) {
            zmq_util::send_string(
                message,
                &pushers[hash_pair.second.get_node_join_connect_addr()]);
            observed_ip.insert(this_ip);
          }
        }

        logger->info("Hash ring for tier {} is size {}.",
                     std::to_string(global_pair.first),
                     std::to_string(global_pair.second.size()));
      }

      // tell all worker threads about the new node join
      for (unsigned tid = 1; tid < kThreadNum; tid++) {
        zmq_util::send_string(
            message,
            &pushers[ServerThread(ip, tid).get_node_join_connect_addr()]);
      }
    }

    if (tier == kSelfTierId) {
      bool succeed;

      for (const auto& key_pair : key_size_map) {
        Key key = key_pair.first;
        ServerThreadSet threads = get_responsible_threads(
            wt.get_replication_factor_connect_addr(), key, is_metadata(key),
            global_hash_ring_map, local_hash_ring_map, placement, pushers,
            kSelfTierIdVector, succeed, seed);

        if (succeed) {
          if (threads.find(wt) == threads.end()) {
            join_remove_set.insert(key);

            for (const ServerThread& thread : threads) {
              join_addr_keyset_map[thread.get_gossip_connect_addr()].insert(
                  key);
            }
          }
        } else {
          logger->error(
              "Missing key replication factor in node join "
              "routine. This should never happen.");
        }
      }
    }
  }
}