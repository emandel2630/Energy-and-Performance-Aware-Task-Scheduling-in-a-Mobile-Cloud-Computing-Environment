#include <vector>
#include <iostream>
#include <numeric>
#include <algorithm>
#include <map>
#include <limits>    
#include <chrono>
#include <utility>
#include <cmath>
#include <unordered_map>
#include <cstdlib>  

using namespace std;


class Node {
public:
    int id;  // Node id
    vector<Node*> parents;  // List of parent Nodes
    vector<Node*> children;  // List of child Nodes
    vector<int> core_speed;  // List: [9, 7, 5] for core1, core2 and core3
    vector<int> cloud_speed;  // List [3, 1, 1] for cloud speed
    int remote_execution_time;  // Sum of cloud_speed, equivalent to Eq 12 in the paper
    int local_finish_time;  // Local finish time, initialized to 0
    int ft = 0; // general finish time
    int wireless_sending_finish_time;  // Wireless sending finish time
    int cloud_finish_time;  // Cloud finish time
    int wireless_recieving_finish_time;  // Wireless receiving finish time
    int local_ready_time;  // Local ready time
    int wireless_sending_ready_time;  // Cloud ready time
    int cloud_ready_time;  // Cloud ready time
    int wireless_recieving_ready_time;  // Wireless receiving ready time
    double priority_score;  // Priority score of the node
    int assignment;  // 0 (core), 1 (core), 2 (core), 3 (cloud)
    bool is_core;  // Is the task occurring on a core or on cloud
    vector<int> start_time;  // Start time for core1, core2, core3, cloud
    int is_scheduled;  // Has the task been scheduled

    // Constructor
    Node(int id, const vector<int>& core_speed, const vector<int>& cloud_speed, 
     const vector<Node*>& parents = vector<Node*>(), 
     const vector<Node*>& children = vector<Node*>(), 
     int assignment = -2, int local_ready_time = -1, 
     int wireless_sending_ready_time = -1, int cloud_ready_time = -1, 
     int wireless_recieving_ready_time = -1)
        : id(id), parents(parents), children(children), core_speed(core_speed),
          cloud_speed(cloud_speed), remote_execution_time(accumulate(cloud_speed.begin(), cloud_speed.end(), 0)),
          local_finish_time(0), ft(0), wireless_sending_finish_time(0), cloud_finish_time(0),
          wireless_recieving_finish_time(0), local_ready_time(local_ready_time), 
          wireless_sending_ready_time(wireless_sending_ready_time), cloud_ready_time(cloud_ready_time),
          wireless_recieving_ready_time(wireless_recieving_ready_time), priority_score(-1), 
          assignment(assignment), is_core(false), start_time(4, -1), is_scheduled(-1) {}

    // Method to print node information
    void print_info() const {
        cout << "NODE ID: " << id << "\n";
        cout << "Assignment: " << assignment + 1 << "\n";
        cout << "local READY time: " << local_ready_time << "\n";
        cout << "wireless sending READY time: " << wireless_sending_ready_time << "\n";
        cout << "cloud READY time: " << cloud_ready_time << "\n";
        cout << "wireless recieving READY time: " << wireless_recieving_ready_time << "\n";
        cout << "START time: " << start_time[assignment] << "\n";
        cout << "local FINISH time: " << local_finish_time << "\n";
        cout << "wireless sending FINISH time: " << wireless_sending_finish_time << "\n";
        cout << "cloud FINISH time: " << cloud_finish_time << "\n";
        cout << "wireless recieving FINISH time: " << wireless_recieving_finish_time << "\n\n";
    }

    Node* deep_copy_node(Node* original, unordered_map<Node*, Node*>& copied_nodes) {
        if (original == nullptr) {
            return nullptr;
        }

        // Check if already copied this node
        auto it = copied_nodes.find(original);
        if (it != copied_nodes.end()) {
            // Return the already copied node
            return it->second;
        }

        // Create a new node with the same data
        Node* copy = new Node(original->id, original->core_speed, original->cloud_speed);
        copied_nodes[original] = copy;

        // Copy the parents and children
        for (Node* parent : original->parents) {
            copy->parents.push_back(deep_copy_node(parent, copied_nodes));
        }
        for (Node* child : original->children) {
            copy->children.push_back(deep_copy_node(child, copied_nodes));
        }

        // Copy other attributes if necessary
        copy->remote_execution_time = original->remote_execution_time;
        copy->local_finish_time = original->local_finish_time;
        copy->ft = original->ft;
        copy->wireless_sending_finish_time = original->wireless_sending_finish_time;
        copy->cloud_finish_time = original->cloud_finish_time;
        copy->wireless_recieving_finish_time = original->wireless_recieving_finish_time;
        copy->local_ready_time = original->local_ready_time;
        copy->wireless_sending_ready_time = original->wireless_sending_ready_time;
        copy->cloud_ready_time = original->cloud_ready_time;
        copy->wireless_recieving_ready_time = original->wireless_recieving_ready_time;
        copy->priority_score = original->priority_score;
        copy->assignment = original->assignment;
        copy->is_core = original->is_core;
        copy->start_time = original->start_time;
        copy->is_scheduled = original->is_scheduled;

        return copy;
    }
};

class TaskForPlotting {
public:
    int node_id;
    int assignment;
    int cloud_start_time;
    int cloud_finish_time;
    int ws_start_time;
    int ws_finish_time;
    int wr_start_time;
    int wr_finish_time;
    int local_start_time;
    int local_finish_time;
    bool is_core;

    // Function to print task details
    void print() const {
        cout << "Node ID: " << node_id ;
        if (is_core) {
            cout << ", Assignment: Core " << assignment << ", Local Start Time: " << local_start_time << ", Local Finish Time: " << local_finish_time << endl;
        } else {
            cout << ", Assignment: Cloud" << ", Cloud Start Time: " << cloud_start_time << ", Cloud Finish Time: " << cloud_finish_time
                      << ", WS Start Time: " << ws_start_time << ", WS Finish Time: " << ws_finish_time
                      << ", WR Start Time: " << wr_start_time << ", WR Finish Time: " << wr_finish_time << endl;
        }
    }
};

vector<Node*> deep_copy_node_list(const vector<Node*>& original_list) {
        unordered_map<Node*, Node*> copied_nodes_map;
        vector<Node*> copied_list;

        // Create deep copies of each node
        for (Node* node : original_list) {
            Node* copied_node = node->deep_copy_node(node, copied_nodes_map);
            copied_list.push_back(copied_node);
        }

        return copied_list;
    }


// Function to compute the total time
double total_T(const vector<Node*>& nodes) {
    double total_t = 0;
    for (const auto& node : nodes) {
        // Only consider nodes with no children
        if (node->children.empty()) {
            total_t = max(node->local_finish_time, node->wireless_recieving_finish_time);
        }
    }
    return total_t;
}

// Function to compute total energy
double total_E(const vector<Node*>& nodes, const vector<double>& core_cloud_power = {1, 2, 4, 0.5}) {
    /**
    Compute total energy.
    core_cloud_power: [1, 2, 4, 0.5] for core1, core2, core3, cloud sending
    */
    double total_energy = 0;
    for (const auto& node : nodes) {
        double current_node_e = 0;
        if (node->is_core) {
            // Energy calculation for core nodes
            current_node_e = node->core_speed[node->assignment] * core_cloud_power[node->assignment];
        } else {
            // Energy calculation for cloud nodes
            current_node_e = node->cloud_speed[0] * core_cloud_power[3];
        }
        total_energy += current_node_e;
    }
    return total_energy;
}


// Assuming Node is a class with appropriate members
void primary_assignment(vector<Node*>& nodes) {
    // Loop through each node in the nodes list
    for (auto& node : nodes) {
        int t_l_min = *min_element(node->core_speed.begin(), node->core_speed.end());
        // Classify tasks: value of node->assignment 1:local 0:cloud
        if (t_l_min > node->remote_execution_time) { // EQ 11
            node->is_core = true; // Local
        } else {
            node->is_core = false; // Cloud
        }
    }
}

// Recursive helper function to calculate the priority of a task
double calculate_priority(Node* task, const vector<Node*>& task_graph, const vector<double>& weights, map<double, double>& priority_cache) {
    // Check if the task's priority has already been calculated
    if (priority_cache.find(task->id) != priority_cache.end()) {
        return priority_cache[task->id];
    }

    // Base case: If the task has no children, it's an exit task
    if (task->children.empty()) {
        priority_cache[task->id] = weights[task->id - 1];
        return weights[task->id - 1];
    }

    // Recursive case: Calculate task priority based on its successors
    double max_successor_priority = 0;
    for (const auto& successor : task->children) {
        double successor_priority = calculate_priority(successor, task_graph, weights, priority_cache);
        max_successor_priority = max(max_successor_priority, successor_priority);
    }
    double task_priority = weights[task->id - 1] + max_successor_priority;

    // Store the calculated priority in priority_cache and return it
    priority_cache[task->id] = task_priority;

    return task_priority;
}


// Function to calculate priorities for all tasks in the task graph
map<double, double> calculate_all_priorities(const vector<Node*>& task_graph, const vector<double>& weights) {
    map<double, double> priority_cache; // Map to store calculated priorities to avoid recalculating

    for (const auto& task : task_graph) {
        // Calculate priority for each task using a recursive helper function
        calculate_priority(task, task_graph, weights, priority_cache);
    }

    return priority_cache; // Return the map of priorities
}



void task_prioritizing(vector<Node*>& nodes) {
    /**
    Assign priority scores to tasks based on their characteristics and position in the task graph.
    :param nodes: A vector of Node pointers representing tasks.
    */
    int n = nodes.size();
    vector<double> w(n, 0.0); // Initialize a vector of weights for each node

    // Determine weights based on whether a node is a core or cloud task
    for (int i = 0; i < n; ++i) {
        if (nodes[i]->is_core) {
            w[i] = nodes[i]->remote_execution_time; // Use remote execution time for core nodes
        } else {
            double sumCoreSpeed = accumulate(nodes[i]->core_speed.begin(), nodes[i]->core_speed.end(), 0);
            w[i] = sumCoreSpeed / nodes[i]->core_speed.size(); // Average core speed for cloud nodes
        }
    }

    // Reverse the nodes vector to start priority calculation from the end of the task graph
    reverse(nodes.begin(), nodes.end());

    // Calculate priorities for all nodes
    map<double, double> priorities = calculate_all_priorities(nodes, w);

    // Reverse the nodes vector back to its original order
    reverse(nodes.begin(), nodes.end());

    // Update the priority_score attribute in each Node object
    for (int i = 0; i < n; ++i) {
        nodes[i]->priority_score = priorities[nodes[i]->id];
    }
}


vector<vector<int>> execution_unit_selection(vector<Node*>& nodes) {
    int k = 3;  // Number of cores available for task scheduling
    int n = nodes.size();  // Number of nodes for iteration

    // Initialize sequences for each core and the cloud
    vector<int> core1_seq;
    vector<int> core2_seq;
    vector<int> core3_seq;
    vector<int> cloud_seq;

    // Track the earliest ready time for each core and the cloud
    vector<int> coreEarliestReady(k + 1, 0);  // +1 for including the cloud

    // Prepare a list of nodes with their priority scores and IDs
    vector<pair<double, int>> node_priority_list;
    for (const auto& node : nodes) {  // Loop over nodes
        node_priority_list.emplace_back(node->priority_score, node->id);
    }

    // Sort the list of pairs by priority score
    sort(node_priority_list.begin(), node_priority_list.end());

    // Extract node IDs from the sorted list, now ordered by priority
    vector<int> pri_n;
    for (const auto& item : node_priority_list) {
        pri_n.push_back(item.second);  // Prio list with node id
    }

    // Schedule each node based on priority
    for (int a = n - 1; a >= 0; --a) {  // Iterate in reverse order
        int i = pri_n[a] - 1;  // Convert ID to index
        Node* node = nodes[i];

        // Calculate ready times and finish times for each node
        if (node->parents.empty()) {  // If the node has no parents, it can start immediately
            auto min_load_core_it = min_element(coreEarliestReady.begin(), coreEarliestReady.end());
            int min_load_core = distance(coreEarliestReady.begin(), min_load_core_it);

            // Schedule the parentless node on the earliest available resource
            node->local_ready_time = coreEarliestReady[min_load_core];
            node->wireless_sending_ready_time = coreEarliestReady[min_load_core];
            node->wireless_sending_finish_time = node->wireless_sending_ready_time + node->cloud_speed[0];
            node->cloud_ready_time = node->wireless_sending_finish_time;
            coreEarliestReady[min_load_core] = node->cloud_ready_time;
        } else {  // If the node has parents, calculate its ready time based on their finish times
            int max_j_l = 0;
            int max_j_ws = 0;
            int max_j_c = 0;
            for (const auto& parent : node->parents) {
                max_j_l = max(max_j_l, max(parent->local_finish_time, parent->wireless_recieving_finish_time));
                max_j_ws = max(max_j_ws, max(parent->local_finish_time, parent->wireless_recieving_finish_time));
                max_j_c = max(max_j_c, parent->wireless_recieving_finish_time - node->cloud_speed[2]);
            }
            node->local_ready_time = max_j_l;
            node->wireless_sending_ready_time = max_j_ws;
            node->wireless_sending_finish_time = max(node->wireless_sending_ready_time, coreEarliestReady[3]) + node->cloud_speed[0];
            node->cloud_ready_time = max(node->wireless_sending_finish_time, max_j_c);
        }

        // Determine whether to schedule the node on a core or in the cloud
        if (node->is_core) {
            // Scheduling for a node assigned to the cloud
            node->wireless_recieving_ready_time = node->cloud_ready_time + node->cloud_speed[1];
            node->wireless_recieving_finish_time = node->wireless_recieving_ready_time  + node->cloud_speed[2];
            node->ft = node->wireless_recieving_finish_time;
            node->local_finish_time = 0;
            coreEarliestReady[3] = node->wireless_sending_finish_time;
            node->start_time[3] = node->wireless_sending_ready_time;
            node->assignment = 3;  // Assign to cloud
            node->is_core = false;
            node->is_scheduled = 1;
        } else {
            // Find the most suitable core for scheduling
            double finish_time = numeric_limits<double>::infinity();
            int index = -1;
            for (int j = 0; j < k; ++j) {
                double ready_time = max(node->local_ready_time, coreEarliestReady[j]);
                if (finish_time > ready_time + node->core_speed[j]) {
                    finish_time = ready_time + node->core_speed[j];
                    index = j;
                }
            }
            node->local_ready_time = finish_time - node->core_speed[index];
            node->start_time[index] = node->local_ready_time;
            node->local_finish_time = finish_time;
            node->wireless_recieving_ready_time = node->cloud_ready_time + node->cloud_speed[1];
            node->wireless_recieving_finish_time = node->wireless_recieving_ready_time  + node->cloud_speed[2];

            // Decide whether to schedule the node on the selected core or in the cloud
            if (node->local_finish_time <= node->wireless_recieving_finish_time) {
                node->ft = node->local_finish_time;
                node->start_time[index] = node->local_ready_time;
                node->wireless_recieving_finish_time = 0;
                coreEarliestReady[index] = node->ft;
                node->assignment = index;
                node->is_core = true;
                node->is_scheduled = 1;
            } else {
                node->ft = node->wireless_recieving_finish_time;
                node->local_finish_time = 0;
                coreEarliestReady[3] = node->ft;
                node->start_time[3] = node->wireless_sending_ready_time;
                node->assignment = 3;  // Assign to cloud
                node->is_core = false;
                node->is_scheduled = 1;
            }
        }

        // Append the node ID to the appropriate sequence based on its assignment
        if (node->assignment == 0) {
            core1_seq.push_back(node->id);
        } else if (node->assignment == 1) {
            core2_seq.push_back(node->id);
        } else if (node->assignment == 2) {
            core3_seq.push_back(node->id);
        } else if (node->assignment == 3) {
            cloud_seq.push_back(node->id);
        }
    }

    // Compile the final sequences for all cores and the cloud
    vector<vector<int>> seq = {core1_seq, core2_seq, core3_seq, cloud_seq};
    return seq;
}


vector<vector<int>> new_sequence(vector<Node*>& nodes, int targetNodeId, int targetLocation, vector<vector<int>>& seq) {
    /**
    Compute a new scheduling sequence by migrating a target node to a new location (core or cloud).
    :param nodes: Vector of all nodes (tasks) in the system.
    :param targetNodeId: ID of the target node to be migrated.
    :param targetLocation: The destination location for migration, represented as an index (0-3 corresponds to core1, core2, core3, cloud).
    :param seq: The current scheduling sequence for all cores and the cloud, each as a vector of node IDs.
    :return: The updated scheduling sequence after migration.
    */
    // Create a map to map node IDs to their index in the nodes vector for quick access
    map<int, int> nodeIdToIndexMap; // {key: node ID, value: index in nodes vector}
    Node* target_node = nullptr;
    for (int i = 0; i < nodes.size(); ++i) {
        nodeIdToIndexMap[nodes[i]->id] = i;
        // Identify the target node based on the provided target ID
        if (nodes[i]->id == targetNodeId) {
            target_node = nodes[i];
        }
    }

    // Determine the ready time of the target node based on its current assignment (core or cloud)
    int target_node_rt = target_node->is_core ? target_node->local_ready_time : target_node->wireless_sending_ready_time;

    // Remove the target node from its original sequence
    auto& original_seq = seq[target_node->assignment];
    original_seq.erase(remove(original_seq.begin(), original_seq.end(), targetNodeId), original_seq.end());

    // Prepare to insert the target node into the new sequence
    vector<int>& s_new = seq[targetLocation]; // The new sequence where the node is to be migrated
    vector<int> s_new_prim; // A temporary vector to hold the new sequence with the target node
    bool flag = false;
    for (int _node_id : s_new) {
        Node* node = nodes[nodeIdToIndexMap[_node_id]];
        // Add nodes to the new sequence maintaining the order based on their start times
        if (node->start_time[targetLocation] < target_node_rt) {
            s_new_prim.push_back(node->id);
        }
        if (node->start_time[targetLocation] >= target_node_rt && !flag) {
            s_new_prim.push_back(target_node->id);
            flag = true;
        }
        if (node->start_time[targetLocation] >= target_node_rt && flag) {
            s_new_prim.push_back(node->id);
        }
    }
    if (!flag) {
        // If the target node has not been added, append it at the end
        s_new_prim.push_back(target_node->id);
    }

    // Update the sequence with the new order
    s_new = s_new_prim;

    // Update the assignment of the target node to the new location
    target_node->assignment = targetLocation;
    // Update whether the target node is on a core or the cloud based on the new location
    target_node->is_core = targetLocation != 3;

    return seq;
}


tuple<vector<int>, vector<int>, vector<int>, vector<int>, vector<Node*>> 
initialize_kernel(const vector<Node*>& updated_node_list, const vector<vector<int>>& updated_seq) {
    /**
    Helper function for kernel algorithm
    :param updated_node_list: Node list
    :param updated_seq: Current core sequence: [core1_seq, core2_seq, core3_seq, cloud_seq], each one is a vector of nodes
    */

    // Initialize the ready times for local cores and the cloud
    vector<int> localCoreReadyTimes = {0, 0, 0};
    vector<int> cloudStageReadyTimes = {0, 0, 0};
    // Initialize vectors to track the readiness of each node for scheduling
    vector<int> dependencyReadiness(updated_node_list.size(), -1);  // -1 indicates not ready. Index matches node ID
    vector<int> sequenceReadiness(updated_node_list.size(), -1);  // Similar to dependencyReadiness but for a different readiness condition
    dependencyReadiness[updated_node_list[0]->id - 1] = 0;  // The first node is initially ready
    for (const auto& each_seq : updated_seq) {
        if (!each_seq.empty()) {
            sequenceReadiness[each_seq[0] - 1] = 0;  // The first node in each sequence is initially ready
        }
    }

    // Create a map to map node IDs to their index in the node list
    map<int, int> node_index;
    for (int i = 0; i < updated_node_list.size(); ++i) {
        node_index[updated_node_list[i]->id] = i;
        // Initialize ready times for different stages for each node
        updated_node_list[i]->local_ready_time = updated_node_list[i]->wireless_sending_ready_time = 
        updated_node_list[i]->cloud_ready_time = updated_node_list[i]->wireless_recieving_ready_time = -1;
    }

    // Initialize a stack for processing nodes in LIFO order
    vector<Node*> stack;
    stack.push_back(updated_node_list[0]);  // Start with the first node

    return {localCoreReadyTimes, cloudStageReadyTimes, dependencyReadiness, sequenceReadiness, stack};
}


void calculate_and_schedule_node(Node* currentNode, vector<int>& localCoreReadyTimes, vector<int>& cloudStageReadyTimes) {
    /**
     * Helper function for the kernel algorithm. This function calculates the ready time for a node and schedules it either on a local core or on the cloud, updating the necessary finish times and source readiness.
     *
     * :param currentNode: The node to be scheduled. This should be an object with attributes like 'is_core', 'parents', 'assignment', 'core_speed', etc.
     * :param localCoreReadyTimes: A vector representing the readiness times of the local cores. It is updated based on the node's scheduling.
     * :param cloudStageReadyTimes: A vector representing the readiness times at different stages of cloud processing. It is updated based on the node's scheduling.
     */

    // Calculate local ready time for local tasks.
    if (currentNode->is_core) {
        currentNode->local_ready_time = 0; // Ready time is 0 if no parents.
        if (!currentNode->parents.empty()) {
            // Calculate ready time based on the finish time of the parent nodes.
            for (auto& parent : currentNode->parents) {
                int p_ft = max(parent->local_finish_time, parent->wireless_recieving_finish_time);
                if (p_ft > currentNode->local_ready_time) {
                    currentNode->local_ready_time = p_ft;
                }
            }
        }
    }

    // Schedule the node on its assigned core or cloud.
    if (currentNode->assignment >= 0 && currentNode->assignment <= 2) { // If assigned to a local core.
        currentNode->start_time = vector<int>(4, -1);
        int core_index = currentNode->assignment;
        currentNode->start_time[core_index] = max(localCoreReadyTimes[core_index], currentNode->local_ready_time);
        currentNode->local_finish_time = currentNode->start_time[core_index] + currentNode->core_speed[core_index];
        // Reset other finish times as they are not applicable for local tasks.
        currentNode->wireless_sending_finish_time = currentNode->cloud_finish_time = currentNode->wireless_recieving_finish_time = -1;
        localCoreReadyTimes[core_index] = currentNode->local_finish_time; // Update the core's ready time.
    }

    if (currentNode->assignment == 3) { // If assigned to the cloud.
        // Calculate ready and finish times for each stage of cloud processing.
        currentNode->wireless_sending_ready_time = 0;
        if (!currentNode->parents.empty()) {
            for (auto& parent : currentNode->parents) {
                int p_ws = max(parent->local_finish_time, parent->wireless_sending_finish_time);
                if (p_ws > currentNode->wireless_sending_ready_time) {
                    currentNode->wireless_sending_ready_time = p_ws;
                }
            }
        }
        currentNode->wireless_sending_finish_time = max(cloudStageReadyTimes[0], currentNode->wireless_sending_ready_time) + currentNode->cloud_speed[0];
        currentNode->start_time[3] = max(cloudStageReadyTimes[0], currentNode->wireless_sending_ready_time);
        cloudStageReadyTimes[0] = currentNode->wireless_sending_finish_time;

        // Cloud processing stage.
        int p_max_ft_c = 0;
        for (auto& parent : currentNode->parents) {
            p_max_ft_c = max(p_max_ft_c, parent->cloud_finish_time);
        }
        currentNode->cloud_ready_time = max(currentNode->wireless_sending_finish_time, p_max_ft_c);
        currentNode->cloud_finish_time = max(cloudStageReadyTimes[1], currentNode->cloud_ready_time) + currentNode->cloud_speed[1];
        cloudStageReadyTimes[1] = currentNode->cloud_finish_time;

        // Receiving stage.
        currentNode->wireless_recieving_ready_time = currentNode->cloud_finish_time;
        currentNode->wireless_recieving_finish_time = max(cloudStageReadyTimes[2], currentNode->wireless_recieving_ready_time) + currentNode->cloud_speed[2];
        currentNode->local_finish_time = -1; // Reset local finish time as it's not applicable for cloud tasks.
        cloudStageReadyTimes[2] = currentNode->wireless_recieving_finish_time;
    }
}

void update_readiness_and_stack(Node* currentNode, vector<Node*>& updated_node_list, 
                                const vector<vector<int>>& updated_seq, 
                                vector<int>& dependencyReadiness, vector<int>& sequenceReadiness, 
                                vector<Node*>& stack) {
    // Update readiness of other nodes based on the current node's scheduling.
    const auto& corresponding_seq = updated_seq[currentNode->assignment];
    auto it = find(corresponding_seq.begin(), corresponding_seq.end(), currentNode->id);
    int currentNode_index = distance(corresponding_seq.begin(), it);
    int next_node_id = (currentNode_index != corresponding_seq.size() - 1) ? corresponding_seq[currentNode_index + 1] : -1;

    for (auto& node : updated_node_list) {
        int flag = count_if(node->parents.begin(), node->parents.end(), [](Node* parent){ 
            return parent->is_scheduled != 2; 
        });
        dependencyReadiness[node->id - 1] = flag;
        if (node->id == next_node_id) {
            sequenceReadiness[node->id - 1] = 0;
        }
    }

    // Add nodes to the stack if they meet the readiness criteria
    for (auto& node : updated_node_list) {
        auto stack_it = find(stack.begin(), stack.end(), node);
        if (dependencyReadiness[node->id - 1] == 0 && sequenceReadiness[node->id - 1] == 0 
            && node->is_scheduled != 2 && stack_it == stack.end()) {
            stack.push_back(node);
        }
    }
}

vector<Node*> kernel_algorithm(vector<Node*>& updated_node_list, const vector<vector<int>>& updated_seq) {
    /**
    Kernel algorithm

    :param updated_node_list: node list
    :param updated_seq: current core sequence: [core1_seq, core2_seq, core3_seq, cloud_seq], each one is a list of nodes
    */

    vector<int> localCoreReadyTimes, cloudStageReadyTimes, dependencyReadiness, sequenceReadiness;
    vector<Node*> stack;
    tie(localCoreReadyTimes, cloudStageReadyTimes, dependencyReadiness, sequenceReadiness, stack) = initialize_kernel(updated_node_list, updated_seq);

    // Process nodes until the stack is empty
    while (stack.size()!=0) {
        Node* currentNode = stack.back();
        stack.pop_back();  // Pop the last node from the stack
        currentNode->is_scheduled = 2;  // Mark the node as scheduled
        calculate_and_schedule_node(currentNode, localCoreReadyTimes, cloudStageReadyTimes);
        update_readiness_and_stack(currentNode, updated_node_list, updated_seq, dependencyReadiness, sequenceReadiness, stack);
    }

    // Reset the scheduling status of all nodes after processing
    for (auto& node : updated_node_list) {
        node->is_scheduled = -1;
    }

    return updated_node_list;
}

int main(){
    // Initialize nodes with specific IDs, parents, children, core and cloud speeds

    // Test 1
    // Node node10(10, {7, 4, 2}, {3, 1, 1});
    // Node node9(9, {5, 3, 2}, {3, 1, 1});
    // Node node8(8, {6, 4, 2}, {3, 1, 1});
    // Node node7(7, {8, 5, 3}, {3, 1, 1});
    // Node node6(6, {7, 6, 4}, {3, 1, 1});
    // Node node5(5, {5, 4, 2}, {3, 1, 1});
    // Node node4(4, {7, 5, 3}, {3, 1, 1});
    // Node node3(3, {6, 5, 4}, {3, 1, 1});
    // Node node2(2, {8, 6, 5}, {3, 1, 1});
    // Node node1(1, {9, 7, 5}, {3, 1, 1});
    // node1.children = {&node2, &node3, &node4, &node5, &node6};
    // node2.parents = {&node1}; node2.children = {&node8, &node9};
    // node3.parents = {&node1}; node3.children = {&node7};
    // node4.parents = {&node1}; node4.children = {&node8, &node9};
    // node5.parents = {&node1}; node5.children = {&node9};
    // node6.parents = {&node1}; node6.children = {&node8};
    // node7.parents = {&node3}; node7.children = {&node10};
    // node8.parents = {&node2, &node4, &node6}; node8.children = {&node10};
    // node9.parents = {&node2, &node4, &node5}; node9.children = {&node10};
    // node10.parents = {&node7, &node8, &node9};

    // vector<Node*> node_list = {&node1, &node2, &node3, &node4, &node5, &node6, &node7, &node8, &node9, &node10};

    // Test 2
    // Node node10(10, {7, 4, 2}, {3, 1, 1});
    // Node node9(9, {5, 3, 2}, {3, 1, 1});
    // Node node8(8, {6, 4, 2}, {3, 1, 1});
    // Node node7(7, {8, 5, 3}, {3, 1, 1});
    // Node node6(6, {7, 6, 4}, {3, 1, 1});
    // Node node5(5, {5, 4, 2}, {3, 1, 1});
    // Node node4(4, {7, 5, 3}, {3, 1, 1});
    // Node node3(3, {6, 5, 4}, {3, 1, 1});
    // Node node2(2, {8, 6, 5}, {3, 1, 1});
    // Node node1(1, {9, 7, 5}, {3, 1, 1});

    // node1.parents = {}; node1.children = {&node2, &node3, &node4};
    // node2.parents = {&node1};node2.children = {&node5, &node7};
    // node3.parents = {&node1};node3.children = {&node7, &node8};
    // node4.parents = {&node1};node4.children = {&node7, &node8};
    // node5.parents = {&node2};node5.children = {&node6};
    // node6.parents = {&node5};node6.children = {&node10};
    // node7.parents = {&node2, &node3, &node4};node7.children = {&node9, &node10};
    // node8.parents = {&node3, &node4};node8.children = {&node9};
    // node9.parents = {&node7, &node8};node9.children = {&node10};
    // node10.parents = {&node6, &node7, &node9};node10.children = {};

    // vector<Node*> node_list = {&node1, &node2, &node3, &node4, &node5, &node6, &node7, &node8, &node9, &node10};

    // Test 3
    // Node node20(20, {12, 5, 4}, {3, 1, 1});
    // Node node19(19, {10, 5, 3}, {3, 1, 1});
    // Node node18(18, {13, 9, 2}, {3, 1, 1});
    // Node node17(17, {9, 3, 3}, {3, 1, 1});
    // Node node16(16, {9, 7, 3}, {3, 1, 1});
    // Node node15(15, {13, 4, 2}, {3, 1, 1});
    // Node node14(14, {12, 11, 4}, {3, 1, 1});
    // Node node13(13, {11, 3, 2}, {3, 1, 1});
    // Node node12(12, {12, 8, 4}, {3, 1, 1});
    // Node node11(11, {12, 3, 3}, {3, 1, 1});
    // Node node10(10, {7, 4, 2}, {3, 1, 1});
    // Node node9(9, {5, 3, 2}, {3, 1, 1});
    // Node node8(8, {6, 4, 2}, {3, 1, 1});
    // Node node7(7, {8, 5, 3}, {3, 1, 1});
    // Node node6(6, {7, 6, 4}, {3, 1, 1});
    // Node node5(5, {5, 4, 2}, {3, 1, 1});
    // Node node4(4, {7, 5, 3}, {3, 1, 1});
    // Node node3(3, {6, 5, 4}, {3, 1, 1});
    // Node node2(2, {8, 6, 5}, {3, 1, 1});
    // Node node1(1, {9, 7, 5}, {3, 1, 1});
    // node1.parents = {};node1.children = {&node2, &node3, &node4, &node5, &node6};
    // node2.parents = {&node1};node2.children = {&node7};
    // node3.parents = {&node1};node3.children = {&node7, &node8};
    // node4.parents = {&node1};node4.children = {&node8, &node9};
    // node5.parents = {&node1};node5.children = {&node9, &node10};
    // node6.parents = {&node1};node6.children = {&node10, &node11};
    // node7.parents = {&node2, &node3};node7.children = {&node12};
    // node8.parents = {&node3, &node4};node8.children = {&node12, &node13};
    // node9.parents = {&node4, &node5};node9.children = {&node13, &node14};
    // node10.parents = {&node5, &node6};node10.children = {&node11, &node15};
    // node11.parents = {&node6, &node10};node11.children = {&node15, &node16};
    // node12.parents = {&node7, &node8};node12.children = {&node17};
    // node13.parents = {&node8, &node9};node13.children = {&node17, &node18};
    // node14.parents = {&node9, &node10};node14.children = {&node18, &node19};
    // node15.parents = {&node10, &node11};node15.children = {&node19};
    // node16.parents = {&node11};node16.children = {&node19};
    // node17.parents = {&node12, &node13};node17.children = {&node20};
    // node18.parents = {&node13, &node14};node18.children = {&node20};
    // node19.parents = {&node14, &node15,&node16};node19.children = {&node20};
    // node20.parents = {&node17, &node18,&node19};node20.children = {};

    // vector<Node*> node_list = {&node1, &node2, &node3, &node4, &node5, &node6, &node7, &node8, &node9, &node10, &node11, &node12, &node13, &node14, &node15, &node16, &node17, &node18, &node19, &node20};

    // Test 4
    // Node node20(20, {12, 5, 4}, {3, 1, 1});
    // Node node19(19, {10, 5, 3}, {3, 1, 1});
    // Node node18(18, {13, 9, 2}, {3, 1, 1});
    // Node node17(17, {9, 3, 3}, {3, 1, 1});
    // Node node16(16, {9, 7, 3}, {3, 1, 1});
    // Node node15(15, {13, 4, 2}, {3, 1, 1});
    // Node node14(14, {12, 11, 4}, {3, 1, 1});
    // Node node13(13, {11, 3, 2}, {3, 1, 1});
    // Node node12(12, {12, 8, 4}, {3, 1, 1});
    // Node node11(11, {12, 3, 3}, {3, 1, 1});
    // Node node10(10, {7, 4, 2}, {3, 1, 1});
    // Node node9(9, {5, 3, 2}, {3, 1, 1});
    // Node node8(8, {6, 4, 2}, {3, 1, 1});
    // Node node7(7, {8, 5, 3}, {3, 1, 1});
    // Node node6(6, {7, 6, 4}, {3, 1, 1});
    // Node node5(5, {5, 4, 2}, {3, 1, 1});
    // Node node4(4, {7, 5, 3}, {3, 1, 1});
    // Node node3(3, {6, 5, 4}, {3, 1, 1});
    // Node node2(2, {8, 6, 5}, {3, 1, 1});
    // Node node1(1, {9, 7, 5}, {3, 1, 1});
    // node1.parents = {}; node1.children = {&node7};
    // node2.parents = {}; node2.children = {&node7, &node8};
    // node3.parents = {}; node3.children = {&node7, &node8};
    // node4.parents = {}; node4.children = {&node8, &node9};
    // node5.parents = {}; node5.children = {&node9, &node10};
    // node6.parents = {}; node6.children = {&node10, &node11};
    // node7.parents = {&node1, &node2, &node3}; node7.children = {&node12};
    // node8.parents = {&node3, &node4}; node8.children = {&node12, &node13};
    // node9.parents = {&node4, &node5}; node9.children = {&node13, &node14};
    // node10.parents = {&node5, &node6}; node10.children = {&node11, &node15};
    // node11.parents = {&node6, &node10}; node11.children = {&node15, &node16};
    // node12.parents = {&node7, &node8}; node12.children = {&node17};
    // node13.parents = {&node8, &node9}; node13.children = {&node17, &node18};
    // node14.parents = {&node9, &node10}; node14.children = {&node18, &node19};
    // node15.parents = {&node10, &node11}; node15.children = {&node19};
    // node16.parents = {&node11}; node16.children = {&node19};
    // node17.parents = {&node12, &node13}; node17.children = {&node20};
    // node18.parents = {&node13, &node14}; node18.children = {&node20};
    // node19.parents = {&node14, &node15, &node16}; node19.children = {&node20};
    // node20.parents = {&node17, &node18, &node19}; node20.children = {};

    // vector<Node*> node_list = {&node1, &node2, &node3, &node4, &node5, &node6, &node7, &node8, &node9, &node10, &node11, &node12, &node13, &node14, &node15, &node16, &node17, &node18, &node19, &node20};
    
    // Test 5
    Node node20(20, {12, 5, 4}, {3, 1, 1});
    Node node19(19, {10, 5, 3}, {3, 1, 1});
    Node node18(18, {13, 9, 2}, {3, 1, 1});
    Node node17(17, {9, 3, 3}, {3, 1, 1});
    Node node16(16, {9, 7, 3}, {3, 1, 1});
    Node node15(15, {13, 4, 2}, {3, 1, 1});
    Node node14(14, {12, 11, 4}, {3, 1, 1});
    Node node13(13, {11, 3, 2}, {3, 1, 1});
    Node node12(12, {12, 8, 4}, {3, 1, 1});
    Node node11(11, {12, 3, 3}, {3, 1, 1});
    Node node10(10, {7, 4, 2}, {3, 1, 1});
    Node node9(9, {5, 3, 2}, {3, 1, 1});
    Node node8(8, {6, 4, 2}, {3, 1, 1});
    Node node7(7, {8, 5, 3}, {3, 1, 1});
    Node node6(6, {7, 6, 4}, {3, 1, 1});
    Node node5(5, {5, 4, 2}, {3, 1, 1});
    Node node4(4, {7, 5, 3}, {3, 1, 1});
    Node node3(3, {6, 5, 4}, {3, 1, 1});
    Node node2(2, {8, 6, 5}, {3, 1, 1});
    Node node1(1, {9, 7, 5}, {3, 1, 1});
    node1.parents = {}; node1.children = {&node7};
    node2.parents = {}; node2.children = {&node7, &node8};
    node3.parents = {}; node3.children = {&node7, &node8};
    node4.parents = {}; node4.children = {&node8, &node9};
    node5.parents = {}; node5.children = {&node9, &node10};
    node6.parents = {}; node6.children = {&node10, &node11};
    node7.parents = {&node1, &node2, &node3}; node7.children = {&node12};
    node8.parents = {&node3, &node4}; node8.children = {&node12, &node13};
    node9.parents = {&node4, &node5}; node9.children = {&node13, &node14};
    node10.parents = {&node5, &node6}; node10.children = {&node11, &node15};
    node11.parents = {&node6, &node10}; node11.children = {&node15, &node16};
    node12.parents = {&node7, &node8}; node12.children = {&node17};
    node13.parents = {&node8, &node9}; node13.children = {&node17, &node18};
    node14.parents = {&node9, &node10}; node14.children = {&node18, &node19};
    node15.parents = {&node10, &node11}; node15.children = {&node19};
    node16.parents = {&node11}; node16.children = {&node19};
    node17.parents = {&node12, &node13}; node17.children = {};
    node18.parents = {&node13, &node14}; node18.children = {};
    node19.parents = {&node14, &node15, &node16}; node19.children = {};
    node20.parents = {&node12}; node20.children = {};

    vector<Node*> node_list = {&node1, &node2, &node3, &node4, &node5, &node6, &node7, &node8, &node9, &node10, &node11, &node12, &node13, &node14, &node15, &node16, &node17, &node18, &node19, &node20};


    auto start = chrono::high_resolution_clock::now();

    // Initial scheduling
    primary_assignment(node_list);
    task_prioritizing(node_list);
    auto sequence = execution_unit_selection(node_list);

    // Plot for initial scheduling
    vector<TaskForPlotting> tasksForPlottingInitial;
    for (const auto& node : node_list) {
        TaskForPlotting task;
        task.node_id = node->id;
        task.assignment = node->assignment + 1;
        task.is_core = node->is_core;

        if (!node->is_core) {
            task.cloud_start_time = node->cloud_ready_time;
            task.cloud_finish_time = node->cloud_ready_time + node->cloud_speed[1];
            task.ws_start_time = node->wireless_sending_ready_time;
            task.ws_finish_time = node->wireless_sending_ready_time + node->cloud_speed[0];
            task.wr_start_time = node->wireless_recieving_ready_time;
            task.wr_finish_time = node->wireless_recieving_ready_time + node->cloud_speed[2];
        } else {
            task.local_start_time = node->start_time[node->assignment];
            task.local_finish_time = node->start_time[node->assignment] + node->core_speed[node->assignment];
        }
        tasksForPlottingInitial.push_back(task);
    }

    // Total time and energy at the end of initial scheduling
    double T_init_pre_kernel = total_T(node_list);
    double T_init = T_init_pre_kernel;
    double E_init_pre_kernel = total_E(node_list, {1, 2, 4, 0.5});
    double E_init = E_init_pre_kernel;

    cout << "INITIAL TIME: " << T_init_pre_kernel << endl;
    cout << "INITIAL ENERGY: " << E_init_pre_kernel << endl;

    for (const auto& task : tasksForPlottingInitial) {
        task.print();
    }

    ///////////////////////////////////////
    // start outer loop
    ///////////////////////////////////////

    int iter_num = 0;
    // Set a maximum time constraint for the schedule
    // 1.5 * T_initial
    double T_max_constraint = T_init_pre_kernel * 1.5;

    while (iter_num < 100) {
        // Start of an optimization iteration. The loop will run for a maximum of 100 iterations.
        cout << string(80, '-') << endl;  // Print a separator line
        cout << "iter: " << iter_num << endl;

        // Calculate and print the total time and energy at the start of this iteration
        double T_init = total_T(node_list);
        double E_init = total_E(node_list, {1, 2, 4, 0.5});
        cout << "initial time: " << T_init << endl;
        cout << "initial energy: " << E_init << endl;
        cout << string(80, '-') << endl;

        // Initialize migration choices for each node
        vector<vector<int>> migeff_ratio_choice(node_list.size(), vector<int>(4, 0));
        for (size_t i = 0; i < node_list.size(); ++i) {
            // If the node is currently assigned to the cloud
            if (node_list[i]->assignment == 3) {
                // Mark all resources (4 in total) as possible migration targets
                fill(migeff_ratio_choice[i].begin(), migeff_ratio_choice[i].end(), 1);
            } else {
                // For nodes not on the cloud
                // Initially mark no resource as a target
                // Mark the current resource as a target
                migeff_ratio_choice[i][node_list[i]->assignment] = 1;
            }
        }
        // Initialize a table to store the results (time and energy) of each potential migration
        vector<vector<pair<double, double>>> result_table(node_list.size(), vector<pair<double, double>>(4, make_pair(-1.0, -1.0)));
        for (size_t n = 0; n < migeff_ratio_choice.size(); ++n) { // Iterate over each node.
            auto& nth_row = migeff_ratio_choice[n];
            for (size_t k = 0; k < nth_row.size(); ++k) { // Iterate over each possible resource.
                if (nth_row[k] == 1) {
                    continue; // Skip if the node is already assigned to this resource.
                }

                // Create copies of the current sequence and node list for testing the migration.
                auto seq_copy = sequence; 
                vector<Node*> nodes_copy = deep_copy_node_list(node_list);

                // Apply the migration and run the kernel scheduling algorithm.
                seq_copy = new_sequence(nodes_copy, n + 1, k, seq_copy);
                kernel_algorithm(nodes_copy, seq_copy);

                // Calculate and store the total time and energy for this migration scenario.
                double current_T = total_T(nodes_copy);
                double current_E = total_E(nodes_copy);

                result_table[n][k] = make_pair(current_T, current_E);
                for (Node* node : nodes_copy) {
                    delete node;
                }
            }
        }

        // Initialize variables to track the best migration found in this iteration.
        int n_best = -1, k_best = -1;
        double T_best = T_init, E_best = E_init;
        double eff_ratio_best = -1;
        
        // Find the optimal migration option based on an efficiency ratio.
        for (size_t i = 0; i < result_table.size(); ++i) {
            for (size_t j = 0; j < result_table[i].size(); ++j) {
                auto val = result_table[i][j];
                if (val == make_pair(-1.0, -1.0) || val.first > T_max_constraint) {
                    continue; // Skip invalid or infeasible migrations.
                }

                // Calculate the efficiency ratio for the current migration.
                double eff_ratio = (E_best - val.second) / (abs(val.first - T_best) + 0.00005);
                if (eff_ratio > eff_ratio_best) { // If this migration is more efficient, update the best values.
                    eff_ratio_best = eff_ratio;
                    n_best = i;
                    k_best = j;
                }
            }
        }

        // Check if a better migration option was found.
        if (n_best == -1 && k_best == -1) {
            break; // Exit the loop if no better option is found.
        }

        // Apply the best migration found
        n_best += 1;
        k_best += 1;
        T_best = result_table[n_best - 1][k_best - 1].first;
        E_best = result_table[n_best - 1][k_best - 1].second; 
        cout << "\ncurrent migration: task: " << n_best << ", k: " << k_best
        << ", total time: " << T_best << ", total energy: " << E_best << endl;

        // Update the task sequence to reflect the best migration found
        cout << "\nupdate after current outer loop" << endl;
        sequence = new_sequence(node_list, n_best, k_best - 1, sequence);
        kernel_algorithm(node_list, sequence);

        // Print the updated sequence and calculate the new total time and energy
        for (const auto& s : sequence) {
            cout << '[';
            for (const auto& i : s) {
                cout << i << ' ';
            }
            cout << ']' << endl;
        }
        double T_current = total_T(node_list);
        double E_current = total_E(node_list, {1, 2, 4, 0.5});

        // Calculate the difference in energy from the initial state
        double E_diff = E_init - E_current;
        double T_diff = abs(T_current - T_init);

        // Increment the iteration counter
        iter_num += 1;

        // Print the current total time and energy after the migration
        cout << "\npost migration time: " << T_current << endl;
        cout << "post migration energy: " << E_current << endl;

        // Break the loop if the energy difference is minimal, indicating little to no improvement
        if (E_diff <= 1) {
            break;
        }
    }

    auto elapsed = chrono::duration_cast<chrono::milliseconds>(
        chrono::high_resolution_clock::now() - start).count();

    vector<TaskForPlotting> tasksForPlottingFinal;

    cout << "\n\nRESCHEDULING FINISHED\n\n";

    for (const auto& node : node_list) {
        TaskForPlotting task;
        task.node_id = node->id;
        task.assignment = node->assignment + 1;
        task.is_core = node->is_core;

        if (node->is_core) {
            task.local_start_time = node->start_time[node->assignment];
            task.local_finish_time = node->start_time[node->assignment] + node->core_speed[node->assignment];
        } else {
            task.cloud_start_time = node->cloud_ready_time;
            task.cloud_finish_time = node->cloud_ready_time + node->cloud_speed[1];
            task.ws_start_time = node->start_time[3];
            task.ws_finish_time = node->start_time[3] + node->cloud_speed[0];
            task.wr_start_time = node->wireless_recieving_ready_time;
            task.wr_finish_time = node->wireless_recieving_ready_time + node->cloud_speed[2];
        }

        tasksForPlottingFinal.push_back(task);
    }

    // Printing tasks for plotting
    for (const auto& task : tasksForPlottingFinal) {
        task.print();
    }

    cout << "\nTime to run on machine: " << elapsed << " milliseconds" << endl;
    cout << "final sequence: " << endl;
    for (const auto& s : sequence) {
        cout << "[";
        for (const auto& i : s) {
            cout << i << " ";
        }
        cout << "]" << endl;
    }

    double T_final = total_T(node_list);
    double E_final = total_E(node_list, {1, 2, 4, 0.5});

    cout << "\nINITIAL TIME: " << T_init_pre_kernel << "\nINITIAL ENERGY: " << E_init_pre_kernel << "\n\n";
    cout << "FINAL TIME: " << T_final << "\nFINAL ENERGY: " << E_final << endl;

    return 0;
}