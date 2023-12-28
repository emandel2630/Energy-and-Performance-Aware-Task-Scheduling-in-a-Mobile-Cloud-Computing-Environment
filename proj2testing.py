"""
The algorithm below was created in the following paper:
Energy and Performance-Aware Task Scheduling in a Mobile Cloud Computing 
Environment

By:
Xue Lin, Yanzhi Wang, Qing Xie, Massoud Pedram 
Department of Electrical Engineering 
University of Southern California 

Implementation of the algorithm:
[E]than Mandel

"""


import matplotlib.pyplot as plt
from copy import deepcopy
import time
import sys

# define node class
class Node(object):
    def __init__(self, id, parents, children, core_speed, cloud_speed, assignment = -2, local_ready_time = -1, wireless_sending_ready_time = -1, cloud_ready_time = -1, wireless_recieving_ready_time = -1):
        self.id = id # node id
        self.parents = parents # list of Nodes
        self.children = children # list of Nodes
        self.core_speed = core_speed # list: [9, 7, 5] for core1, core2 and core3
        self.cloud_speed = cloud_speed # list [3, 1, 1] cloud speed
        self.remote_execution_time= sum(cloud_speed) #Eq 12
        self.local_finish_time = 0 # local finish time, inf at start
        self.ft = 0 # general finish time
        self.wireless_sending_finish_time = 0 #wireless sending finish time
        self.cloud_finish_time = 0 #cloud finish time
        self.wireless_recieving_finish_time = 0 #wireless recieving finish time
        self.local_ready_time = -1 # local ready time
        self.wireless_sending_ready_time = -1 # cloud ready time
        self.cloud_ready_time = -1 #cloud ready time
        self.wireless_recieving_ready_time = -1 #wireless recieving ready time 
        self.priority_score = None
        self.assignment = -2 # 0 (core), 1 (core), 2 (core), 3 (cloud)
        self.is_core = False #Is the task occuring on a core or on cloud
        self.start_time = [-1, -1, -1, -1] # start time for core1, core2, core3, cloud
        self.is_scheduled = None #Has the task been schedueled

    
    def print_info(self):
        print(f"NODE ID: {self.id }")
        print(f"Assignment: {self.assignment +1}")
        print(f"local READY time: {self.local_ready_time}")
        print(f"wireless sending READY time: {self.wireless_sending_ready_time}")
        print(f"cloud READY time: {self.cloud_ready_time}")
        print(f"wireless recieving READY time: {self.wireless_recieving_ready_time}")
        print(f"START time: {self.start_time[self.assignment]}")
        print(f"local FINISH time: {self.local_finish_time}")
        print(f"wireless sending FINISH time: {self.wireless_sending_finish_time}")
        print(f"cloud FINISH time: {self.cloud_finish_time}")
        print(f"wireless recieving FINISH time: {self.wireless_recieving_finish_time}")
        print()


def total_T(nodes):
    """compute the total time"""
    total_t = 0
    for node in nodes:
        if len(node.children) == 0:
            total_t = max(node.local_finish_time, node.wireless_recieving_finish_time)
    return total_t

def total_E(nodes, core_cloud_power=[1, 2, 4, 0.5]):
    """compute total energy
        core_cloud_power: [1, 2, 4, 0.5] for core1, core2, core3, cloud sending
    """
    total_energy = 0
    for node in nodes:
        if node.is_core == True:
            current_node_e = node.core_speed[node.assignment] * core_cloud_power[node.assignment]
            total_energy += current_node_e
        if node.is_core == False:
            current_node_e = node.cloud_speed[0] * core_cloud_power[3]
            total_energy += current_node_e
    return total_energy

def primary_assignment(nodes):
    # Assuming nodes is a list of Node objects
    for i,node in enumerate(nodes):
        t_l_min = min(node.core_speed)
        # Classify tasks: value of node.assignment 1:local 0:cloud
        if t_l_min > node.remote_execution_time: #EQ 11
            node.is_core = True #Local
        else:
            node.is_core = False #Cloud

def calculate_all_priorities(task_graph, weights):
    """
    Calculate priorities for all tasks in the task graph.
    :param task_graph: A list of tasks (nodes) representing the task graph.
    :param weights: A list of weights, one for each task, used in priority calculation.
    :return: A dictionary mapping each task's ID to its calculated priority.
    """
    priority_cache  = {}  # Dictionary to store calculated priorities to avoid recalculating.
    for task in task_graph:
        # Calculate priority for each task using a recursive helper function.
        calculate_priority(task, task_graph, weights, priority_cache )
    return priority_cache   # Return the dictionary of priorities.

def calculate_priority(task, task_graph, weights, priority_cache ):
    """
    Recursive helper function to calculate the priority of a task.
    :param task: The current task for which priority is being calculated.
    :param task_graph: The overall task graph.
    :param weights: A list of weights for priority calculation.
    :param priority_cache : Dictionary to store and retrieve previously calculated priorities.
    :return: The priority score of the given task.
    """
    # Check if the task's priority has already been calculated.
    if task in priority_cache :
        return priority_cache [task.id]

    # Base case: If the task has no children, it's an exit task.
    if task.children == []:
        priority_cache [task.id] = weights[task.id-1]
        return weights[task.id-1]

    # Recursive case: Calculate task priority based on its successors.
    max_successor_priority = max(calculate_priority(successor, task_graph, weights, priority_cache ) for successor in task.children)
    task_priority = weights[task.id-1] + max_successor_priority

    # Store the calculated priority in priority_cache  and return it.
    priority_cache [task.id] = task_priority
    return task_priority

def task_prioritizing(nodes):
    """
    Assign priority scores to tasks based on their characteristics and position in the task graph.
    :param nodes: A list of Node objects representing tasks.
    """
    n = len(nodes)
    w = [0] * n  # Initialize a list of weights for each node.
    # Determine weights based on whether a node is a core or cloud task.
    for i, node in enumerate(nodes):
        if node.is_core == True:
            w[i] = node.remote_execution_time  # Use remote execution time for core nodes.
        else:
            w[i] = sum(node.core_speed) / len(node.core_speed)  # Average core speed for cloud nodes.

    # Reverse the nodes list to start priority calculation from the end of the task graph.
    nodes.reverse()

    # Calculate priorities for all nodes.
    priorities = calculate_all_priorities(nodes, w)

    # Reverse the nodes list back to its original order.
    nodes.reverse()

    # Update the priority_score attribute in each Node object.
    for i, node in enumerate(nodes):
        node.priority_score = priorities[node.id]


def execution_unit_selection(nodes):
    k = 3  # Number of cores available for task scheduling.
    n= len(nodes) #Number of nodes for iteration

    # Initialize sequences for each core and the cloud.
    core1_seq = []
    core2_seq = []
    core3_seq = []
    cloud_seq = []

    # Track the earliest ready time for each core and the cloud.
    coreEarliestReady = [0] * (k+1)  # +1 for including the cloud.

    # Prepare a list of nodes with their priority scores and IDs.
    node_priority_list  = []
    for node in node_list:  # Assuming node_list is a global or previously defined list of nodes.
        node_priority_list.append((node.priority_score, node.id))

    # Sort the list of tuples by priority score.
    node_priority_list .sort()

    # Extract node IDs from the sorted list, now ordered by priority.
    pri_n = [item[1] for item in node_priority_list ] #Prio list with node id

    # Schedule each node based on priority.
    for a in range(n-1, -1, -1):  # Iterate in reverse order.
        i = pri_n[a]-1  # Convert ID to index.
        node = nodes[i]

        # Calculate ready times and finish times for each node.
        if not node.parents:  # If the node has no parents, it can start immediately.
            min_load_core = coreEarliestReady.index(min(coreEarliestReady))
            
            # Schedule the parentless node on the earliest available resource
            node.local_ready_time = coreEarliestReady[min_load_core]
            node.wireless_sending_ready_time = coreEarliestReady[min_load_core]
            node.wireless_sending_finish_time = node.wireless_sending_ready_time + node.cloud_speed[0]
            node.cloud_ready_time = node.wireless_sending_finish_time
            coreEarliestReady[min_load_core] = node.cloud_ready_time
        else:  # If the node has parents, calculate its ready time based on their finish times.
            # Calculations for local and cloud ready times.
            max_j_l = max([max(parent.local_finish_time, parent.wireless_recieving_finish_time) for parent in node.parents], default=0)
            node.local_ready_time = max_j_l

            max_j_ws = max([max(parent.local_finish_time, parent.wireless_recieving_finish_time) for parent in node.parents], default=0)
            node.wireless_sending_ready_time = max_j_ws  
            node.wireless_sending_finish_time = max(node.wireless_sending_ready_time, coreEarliestReady[3]) + node.cloud_speed[0]

            max_j_c = max([(parent.wireless_recieving_finish_time - node.cloud_speed[2]) for parent in node.parents], default=0)
            node.cloud_ready_time = max(node.wireless_sending_finish_time, max_j_c)
            

        # Determine whether to schedule the node on a core or in the cloud.
        if node.is_core:
            # Scheduling for a node assigned to the cloud.
            node.wireless_recieving_ready_time = node.cloud_ready_time + node.cloud_speed[1]
            node.wireless_recieving_finish_time = node.wireless_recieving_ready_time  + node.cloud_speed[2]
            node.ft = node.wireless_recieving_finish_time
            node.local_finish_time = 0
            coreEarliestReady[3] = node.wireless_sending_finish_time
            node.start_time[3] = node.wireless_sending_ready_time
            node.assignment = 3  # Assign to cloud
            node.is_core = False
            node.is_scheduled = 1
        else:
            # Find the most suitable core for scheduling.
            finish_time = float('inf')
            index = -1
            for j in range(k):
                ready_time = max(node.local_ready_time, coreEarliestReady[j])
                if finish_time > ready_time + node.core_speed[j]:
                    finish_time = ready_time + node.core_speed[j]
                    index = j
            node.local_ready_time = finish_time - node.core_speed[index]
            node.start_time[index] = node.local_ready_time
            node.local_finish_time = finish_time
            node.wireless_recieving_ready_time = node.cloud_ready_time + node.cloud_speed[1]
            node.wireless_recieving_finish_time = node.wireless_recieving_ready_time  + node.cloud_speed[2]

            # Decide whether to schedule the node on the selected core or in the cloud.
            if node.local_finish_time <= node.wireless_recieving_finish_time:
                node.ft = node.local_finish_time
                node.start_time[index] = node.local_ready_time
                node.wireless_recieving_finish_time = 0
                coreEarliestReady[index] = node.ft
                node.assignment = index
                node.is_core = True
                node.is_scheduled = 1
            else:
                node.ft = node.wireless_recieving_finish_time
                node.local_finish_time = 0
                coreEarliestReady[3] = node.ft
                node.start_time[3] = node.wireless_sending_ready_time
                node.assignment = 3  # Assign to cloud
                node.is_core = False
                node.is_scheduled = 1

        # Append the node ID to the appropriate sequence based on its assignment.
        if node.assignment == 0:
            core1_seq.append(node.id)
        elif node.assignment == 1:
            core2_seq.append(node.id)
        elif node.assignment == 2:
            core3_seq.append(node.id)
        elif node.assignment == 3:
            cloud_seq.append(node.id)

    # Compile the final sequences for all cores and the cloud.
    seq = [core1_seq, core2_seq, core3_seq, cloud_seq]
    return seq


def new_sequence(nodes, targetNodeId, targetLocation, seq):
    """
    Compute a new scheduling sequence by migrating a target node to a new location (core or cloud).
    :param nodes: List of all nodes (tasks) in the system.
    :param targetNodeId: ID of the target node to be migrated.
    :param targetLocation: The destination location for migration, represented as an index (0-3 corresponds to core1, core2, core3, cloud).
    :param seq: The current scheduling sequence for all cores and the cloud, each as a list of node IDs.
    :return: The updated scheduling sequence after migration.
    """
    # Create a dictionary to map node IDs to their index in the node list for quick access.
    nodeIdToIndexMap = {}  # {key: node ID, value: index in nodes list}
    temp_id = 0
    for node in nodes:
        nodeIdToIndexMap[node.id] = temp_id
        temp_id += 1
        # Identify the target node based on the provided target ID.
        if node.id == targetNodeId:
            target_node = node

    # Determine the ready time of the target node based on its current assignment (core or cloud).
    if target_node.is_core:  # If the target node is currently assigned to a core.
        target_node_rt = target_node.local_ready_time
    else:  # If the target node is currently assigned to the cloud.
        target_node_rt = target_node.wireless_sending_ready_time

    # Remove the target node from its original sequence.
    seq[target_node.assignment].remove(target_node.id)

    # Prepare to insert the target node into the new sequence.
    s_new = seq[targetLocation]  # The new sequence where the node is to be migrated.
    s_new_prim = []  # A temporary list to hold the new sequence with the target node.
    flag = False
    for _node_id in s_new:
        node = nodes[nodeIdToIndexMap[_node_id]]
        # Add nodes to the new sequence maintaining the order based on their start times.
        if node.start_time[targetLocation] < target_node_rt:
            s_new_prim.append(node.id)
        if node.start_time[targetLocation] >= target_node_rt and not flag:
            s_new_prim.append(target_node.id)
            flag = True
        if node.start_time[targetLocation] >= target_node_rt and flag:
            s_new_prim.append(node.id)
    if not flag:
        # If the target node has not been added, append it at the end.
        s_new_prim.append(target_node.id)

    # Update the sequence with the new order.
    seq[targetLocation] = s_new_prim

    # Update the assignment of the target node to the new location.
    target_node.assignment = targetLocation
    # Update whether the target node is on a core or the cloud based on the new location.
    if targetLocation == 3:  # If the new location is the cloud.
        target_node.is_core = False
    else:  # If the new location is a core.
        target_node.is_core = True

    return seq

def initialize_kernel(updated_node_list, updated_seq):
    """
    helper function for kernel algorithm
    :param updated_node_list: node list
    :param updated_seq: current core sequence: [core1_seq, core2_seq, core3_seq, cloud_seq], each one is a list of nodes
    """

    # Initialize the ready times for local cores and the cloud.
    localCoreReadyTimes = [0, 0, 0]
    cloudStageReadyTimes = [0, 0, 0]

    # Initialize arrays to track the readiness of each node for scheduling.
    dependencyReadiness = [-1]*len(updated_node_list)  # -1 indicates not ready. Index matches node ID.
    sequenceReadiness = [-1]*len(updated_node_list)  # Similar to dependencyReadiness but for a different readiness condition.
    dependencyReadiness[updated_node_list[0].id - 1] = 0  # The first node is initially ready.
    for each_seq in updated_seq:
        if len(each_seq) > 0:
            sequenceReadiness[each_seq[0] - 1] = 0  # The first node in each sequence is initially ready.

    # Create a dictionary mapping node IDs to their index in the node list.
    node_index = {}  
    temp_id = 0
    for node in updated_node_list:
        node_index[node.id] = temp_id
        # Initialize ready times for different stages for each node.
        node.local_ready_time = node.wireless_sending_ready_time = node.cloud_ready_time = node.wireless_recieving_ready_time = -1
        temp_id += 1

    # Initialize a stack for processing nodes in LIFO order.
    stack = []
    stack.append(updated_node_list[0])  # Start with the first node.

    return localCoreReadyTimes,cloudStageReadyTimes,dependencyReadiness,sequenceReadiness,stack

def calculate_and_schedule_node(currentNode, localCoreReadyTimes, cloudStageReadyTimes):
    """
    Helper function for the kernel algorithm. This function calculates the ready time for a node and schedules it either on a local core or on the cloud, updating the necessary finish times and source readiness.

    :param node: The node to be scheduled. This should be an object with attributes like 'is_core', 'parents', 'assignment', 'core_speed', etc.
    :param localCoreReadyTimes: A list representing the readiness times of the local cores. It is updated based on the node's scheduling.
    :param cloudStageReadyTimes: A list representing the readiness times at different stages of cloud processing. It is updated based on the node's scheduling.
    """
    # Calculate local ready time for local tasks.
    if currentNode.is_core == True:
        if len(currentNode.parents) == 0:
            currentNode.local_ready_time = 0  # Ready time is 0 if no parents.
        else:
            # Calculate ready time based on the finish time of the parent nodes.
            for parent in currentNode.parents:
                p_ft = max(parent.local_finish_time, parent.wireless_recieving_finish_time)
                if p_ft > currentNode.local_ready_time:
                    currentNode.local_ready_time = p_ft

    # Schedule the node on its assigned core or cloud.
    if currentNode.assignment in [0, 1, 2]:  # If assigned to a local core.
        currentNode.start_time = [-1, -1, -1, -1]
        core_index = currentNode.assignment
        currentNode.start_time[core_index] = max(localCoreReadyTimes[core_index], currentNode.local_ready_time)
        currentNode.local_finish_time = currentNode.start_time[core_index] + currentNode.core_speed[core_index]
        # Reset other finish times as they are not applicable for local tasks.
        currentNode.wireless_sending_finish_time = currentNode.cloud_finish_time = currentNode.wireless_recieving_finish_time = -1
        localCoreReadyTimes[core_index] = currentNode.local_finish_time  # Update the core's ready time.

    if currentNode.assignment == 3:  # If assigned to the cloud.
        # Calculate ready and finish times for each stage of cloud processing.
        # Sending stage:
        if len(currentNode.parents) == 0:
            currentNode.wireless_sending_ready_time = 0
        else:
            for parent in currentNode.parents:
                p_ws = max(parent.local_finish_time, parent.wireless_sending_finish_time)
                if p_ws > currentNode.wireless_sending_ready_time:
                    currentNode.wireless_sending_ready_time = p_ws
        currentNode.wireless_sending_finish_time = max(cloudStageReadyTimes[0], currentNode.wireless_sending_ready_time) + currentNode.cloud_speed[0]
        currentNode.start_time[3] = max(cloudStageReadyTimes[0], currentNode.wireless_sending_ready_time)
        cloudStageReadyTimes[0] = currentNode.wireless_sending_finish_time

        # Cloud processing stage:
        p_max_ft_c = 0
        for parent in currentNode.parents:
            if parent.cloud_finish_time > p_max_ft_c:
                p_max_ft_c = parent.cloud_finish_time
        currentNode.cloud_ready_time = max(currentNode.wireless_sending_finish_time, p_max_ft_c)
        currentNode.cloud_finish_time = max(cloudStageReadyTimes[1], currentNode.cloud_ready_time) + currentNode.cloud_speed[1]
        cloudStageReadyTimes[1] = currentNode.cloud_finish_time

        # Receiving stage:
        currentNode.wireless_recieving_ready_time = currentNode.cloud_finish_time
        currentNode.wireless_recieving_finish_time = max(cloudStageReadyTimes[2], currentNode.wireless_recieving_ready_time) + currentNode.cloud_speed[2]
        currentNode.local_finish_time = -1  # Reset local finish time as it's not applicable for cloud tasks.
        cloudStageReadyTimes[2] = currentNode.wireless_recieving_finish_time


def update_readiness_and_stack(currentNode, updated_node_list, updated_seq, dependencyReadiness, sequenceReadiness, stack):
    """
    Helper function for the kernel algorithm. This function updates the readiness of all nodes based on the scheduling of the current node and adds nodes to a stack if they meet certain readiness criteria. It modifies the readiness arrays and the stack based on the current state of the nodes.

    :param currentNode: The current node whose scheduling influences the readiness of other nodes. This should be an object with attributes like 'id', 'assignment', etc.
    :param updated_node_list: List of all nodes in the algorithm. Each node should have attributes such as 'id', 'parents', 'is_scheduled', etc.
    :param updated_seq: Current core sequence, a list of lists, where each inner list represents a sequence of nodes for a specific core or the cloud.
    :param dependencyReadiness: Readiness array indicating the first condition of readiness for each node. This array is updated in this function.
    :param sequenceReadiness: Readiness array indicating the second condition of readiness for each node. This array is updated in this function.
    :param stack: Stack (list) to which nodes are added if they meet the readiness criteria. It is updated with nodes ready for processing.
    """

    # Update readiness of other nodes based on the current node's scheduling.
    corresponding_seq = updated_seq[currentNode.assignment]  # Get the sequence corresponding to the current node's assignment.
    currentNode_index = corresponding_seq.index(currentNode.id)  # Find index of the current node in its sequence.
    next_node_id = corresponding_seq[currentNode_index + 1] if currentNode_index != len(corresponding_seq) - 1 else -1

    for node in updated_node_list:
        flag = sum(parent.is_scheduled != "kernel_scheduled" for parent in node.parents)
        dependencyReadiness[node.id - 1] = flag
        if node.id == next_node_id:
            sequenceReadiness[node.id - 1] = 0

    # Add nodes to the stack if they meet the readiness criteria.
    for node in updated_node_list:
        if dependencyReadiness[node.id - 1] == 0 and sequenceReadiness[node.id - 1] == 0 and node.is_scheduled != "kernel_scheduled" and node not in stack:
            stack.append(node)



def kernel_algorithm(updated_node_list, updated_seq):
    """
    kernel algorithm
    :param updated_node_list: node list
    :param updated_seq: current core sequence: [core1_seq, core2_seq, core3_seq, cloud_seq], each one is a list of nodes
    """

    localCoreReadyTimes,cloudStageReadyTimes,dependencyReadiness,sequenceReadiness,stack = initialize_kernel(updated_node_list,updated_seq)
  
    # Process nodes until the stack is empty.
    while len(stack) != 0:
        currentNode = stack.pop()  # Pop the last node from the stack.
        currentNode.is_scheduled = "kernel_scheduled"  # Mark the node as scheduled.
        calculate_and_schedule_node(currentNode, localCoreReadyTimes, cloudStageReadyTimes)
        update_readiness_and_stack(currentNode, updated_node_list, updated_seq, dependencyReadiness, sequenceReadiness, stack)
        
    # Reset the scheduling status of all nodes after processing.
    for node in updated_node_list:
        node.is_scheduled = None
    
    return updated_node_list


def draw_schedule(tasks):
    fig, ax = plt.subplots(figsize=(15, 8))
    
    # Define the y-axis for different cores and cloud processes
    y_positions = {
        'Core 1': 6,
        'Core 2': 5,
        'Core 3': 4,
        'Wireless Sending': 3,
        'Cloud': 2,
        'Wireless Receiving': 1
    }

    AssignmentToName = {1:'Core 1', 2: "Core 2", 3: "Core 3", 4: "Wireless Sending", 5: "Cloud", 6: "Wireless Receiving"}
    
    maxFinishTime = 0
    for task in tasks:
        if(task['assignment']<4):
            maxFinishTime = max(task['local finish_time'],maxFinishTime)

            start = task['local start_time']
            duration = task['local finish_time']-task['local start_time']
            bar = AssignmentToName[task['assignment']]
            y_pos = y_positions[bar]
        
            # Draw the task bar
            ax.broken_barh([(start, duration)], (y_pos - 0.4, 0.8), facecolors='blue',edgecolor='red', linewidth=2)
            
            # Annotate the task number in the middle of the bar
            ax.text(start + duration / 2, y_pos, str(task['node id']), ha='center', va='center',color='red')
        else:
            maxFinishTime = max(task['cloud finish_time'],maxFinishTime)
            cloudStart = task['cloud start_time']
            cloudDuration = task['cloud finish_time']-task['cloud start_time']
            y_pos = y_positions[AssignmentToName[5]]
            ax.broken_barh([(cloudStart, cloudDuration)], (y_pos - 0.4, 0.8), facecolors='blue',edgecolor='red', linewidth=2)
            ax.text(cloudStart + cloudDuration / 2, y_pos, str(task['node id']), ha='center', va='center',color='red')
            wsStart = task['ws start_time']
            wsDuration = task['ws finish_time']-task['ws start_time']
            y_pos = y_positions[AssignmentToName[4]]
            ax.broken_barh([(wsStart, wsDuration)], (y_pos - 0.4, 0.8), facecolors='blue',edgecolor='red', linewidth=2)
            ax.text(wsStart + wsDuration / 2, y_pos, str(task['node id']), ha='center', va='center',color='red')
            wrStart = task['wr start_time']
            wrDuration = task['wr finish_time']-task['wr start_time']
            y_pos = y_positions[AssignmentToName[6]]
            ax.broken_barh([(wrStart, wrDuration)], (y_pos - 0.4, 0.8), facecolors='blue',edgecolor='red', linewidth=2)
            ax.text(wrStart + wrDuration / 2, y_pos, str(task['node id']), ha='center', va='center',color='red')
        
    # Set the yticks to be the names of the cores and cloud processes
    ax.set_yticks(list(y_positions.values()))
    ax.set_yticklabels(list(y_positions.keys()))
    
    # Set labels and grid
    ax.set_xlabel('Time')
    ax.grid(True)
    ax.set_xticks(range(0, maxFinishTime +2))  
    ax.set_xlim(0, maxFinishTime +2)

    
    # Show the plot
    plt.show()

if __name__ == '__main__':
    # Initialize nodes with specific IDs, parents, children, core and cloud speeds.

    #Test 1
    node10 = Node(id=10, parents=None, children=[], core_speed=[7, 4, 2], cloud_speed=[3, 1, 1])
    node9 = Node(id=9, parents=None, children=[node10], core_speed=[5, 3, 2], cloud_speed=[3, 1, 1])
    node8 = Node(id=8, parents=None, children=[node10], core_speed=[6, 4, 2], cloud_speed=[3, 1, 1])
    node7 = Node(id=7, parents=None, children=[node10], core_speed=[8, 5, 3], cloud_speed=[3, 1, 1])
    node6 = Node(id=6, parents=None, children=[node8], core_speed=[7, 6, 4], cloud_speed=[3, 1, 1])
    node5 = Node(id=5, parents=None, children=[node9], core_speed=[5, 4, 2], cloud_speed=[3, 1, 1])
    node4 = Node(id=4, parents=None, children=[node8, node9], core_speed=[7, 5, 3], cloud_speed=[3, 1, 1])
    node3 = Node(id=3, parents=None, children=[node7], core_speed=[6, 5, 4], cloud_speed=[3, 1, 1])
    node2 = Node(id=2, parents=None, children=[node8, node9], core_speed=[8, 6, 5], cloud_speed=[3, 1, 1])
    node1 = Node(id=1, parents=None, children=[node2, node3, node4, node5, node6], core_speed=[9, 7, 5], cloud_speed=[3, 1, 1])
    node1.parents = []
    node2.parents = [node1]
    node3.parents = [node1]
    node4.parents = [node1]
    node5.parents = [node1]
    node6.parents = [node1]
    node7.parents = [node3]
    node8.parents = [node2, node4, node6]
    node9.parents = [node2, node4, node5]
    node10.parents = [node7, node8, node9]
    node_list = [node1, node2, node3, node4, node5, node6, node7, node8, node9, node10]
    

    #Test 2
    
    # node10 = Node(id=10, parents=None, children=[], core_speed=[7, 4, 2], cloud_speed=[3, 1, 1])
    # node9 = Node(id=9, parents=None, children=[node10], core_speed=[5, 3, 2], cloud_speed=[3, 1, 1])
    # node8 = Node(id=8, parents=None, children=[node9], core_speed=[6, 4, 2], cloud_speed=[3, 1, 1])
    # node7 = Node(id=7, parents=None, children=[node9,node10], core_speed=[8, 5, 3], cloud_speed=[3, 1, 1])
    # node6 = Node(id=6, parents=None, children=[node10], core_speed=[7, 6, 4], cloud_speed=[3, 1, 1])
    # node5 = Node(id=5, parents=None, children=[node6], core_speed=[5, 4, 2], cloud_speed=[3, 1, 1])
    # node4 = Node(id=4, parents=None, children=[node7,node8], core_speed=[7, 5, 3], cloud_speed=[3, 1, 1])
    # node3 = Node(id=3, parents=None, children=[node7, node8], core_speed=[6, 5, 4], cloud_speed=[3, 1, 1])
    # node2 = Node(id=2, parents=None, children=[node5,node7], core_speed=[8, 6, 5], cloud_speed=[3, 1, 1])
    # node1 = Node(id=1, parents=None, children=[node2, node3, node4], core_speed=[9, 7, 5], cloud_speed=[3, 1, 1])
    # node1.parents = []
    # node2.parents = [node1]
    # node3.parents = [node1]
    # node4.parents = [node1]
    # node5.parents = [node2]
    # node6.parents = [node5]
    # node7.parents = [node2,node3,node4]
    # node8.parents = [node3, node4]
    # node9.parents = [node7,node8]
    # node10.parents = [node6, node7, node9]
    # node_list = [node1, node2, node3, node4, node5, node6, node7, node8, node9, node10]

    
    #Test 3
    # node20 = Node(id=20, parents=None, children=[], core_speed=[12, 5, 4], cloud_speed=[3, 1, 1])
    # node19 = Node(id=19, parents=None, children=[node20], core_speed=[10, 5, 3], cloud_speed=[3, 1, 1])
    # node18 = Node(id=18, parents=None, children=[node20], core_speed=[13, 9, 2], cloud_speed=[3, 1, 1])
    # node17 = Node(id=17, parents=None, children=[node20], core_speed=[9, 3, 3], cloud_speed=[3, 1, 1])
    # node16 = Node(id=16, parents=None, children=[node19], core_speed=[9, 7, 3], cloud_speed=[3, 1, 1])
    # node15 = Node(id=15, parents=None, children=[node19], core_speed=[13, 4, 2], cloud_speed=[3, 1, 1])
    # node14 = Node(id=14, parents=None, children=[node18, node19], core_speed=[12, 11, 4], cloud_speed=[3, 1, 1])
    # node13 = Node(id=13, parents=None, children=[node17, node18], core_speed=[11, 3, 2], cloud_speed=[3, 1, 1])
    # node12 = Node(id=12, parents=None, children=[node17], core_speed=[12, 8, 4], cloud_speed=[3, 1, 1])
    # node11 = Node(id=11, parents=None, children=[node15, node16], core_speed=[12, 3, 3], cloud_speed=[3, 1, 1])
    # node10 = Node(id=10, parents=None, children=[node11,node15], core_speed=[7, 4, 2], cloud_speed=[3, 1, 1])
    # node9 = Node(id=9, parents=None, children=[node13,node14], core_speed=[5, 3, 2], cloud_speed=[3, 1, 1])
    # node8 = Node(id=8, parents=None, children=[node12,node13], core_speed=[6, 4, 2], cloud_speed=[3, 1, 1])
    # node7 = Node(id=7, parents=None, children=[node12], core_speed=[8, 5, 3], cloud_speed=[3, 1, 1])
    # node6 = Node(id=6, parents=None, children=[node10,node11], core_speed=[7, 6, 4], cloud_speed=[3, 1, 1])
    # node5 = Node(id=5, parents=None, children=[node9,node10], core_speed=[5, 4, 2], cloud_speed=[3, 1, 1])
    # node4 = Node(id=4, parents=None, children=[node8,node9], core_speed=[7, 5, 3], cloud_speed=[3, 1, 1])
    # node3 = Node(id=3, parents=None, children=[node7, node8], core_speed=[6, 5, 4], cloud_speed=[3, 1, 1])
    # node2 = Node(id=2, parents=None, children=[node7], core_speed=[8, 6, 5], cloud_speed=[3, 1, 1])
    # node1 = Node(id=1, parents=None, children=[node2, node3, node4,node5,node6], core_speed=[9, 7, 5], cloud_speed=[3, 1, 1])
    # node1.parents = []
    # node2.parents = [node1]
    # node3.parents = [node1]
    # node4.parents = [node1]
    # node5.parents = [node1]
    # node6.parents = [node1]
    # node7.parents = [node2,node3]
    # node8.parents = [node3, node4]
    # node9.parents = [node4,node5]
    # node10.parents = [node5, node6]
    # node11.parents = [node6, node10]
    # node12.parents = [node7, node8]
    # node13.parents = [node8, node9]
    # node14.parents = [node9, node10]
    # node15.parents = [node10, node11]
    # node16.parents = [node11]
    # node17.parents = [node12, node13]
    # node18.parents = [node13, node14]
    # node19.parents = [node14, node15,node16]
    # node20.parents = [node17, node18,node19]

    # node_list = [node1, node2, node3, node4, node5, node6, node7, node8, node9, node10,node11,node12,node13,node14,node15,node16,node17,node18,node19,node20]

    #Test 4
    # node20 = Node(id=20, parents=None, children=[], core_speed=[12, 5, 4], cloud_speed=[3, 1, 1])
    # node19 = Node(id=19, parents=None, children=[node20], core_speed=[10, 5, 3], cloud_speed=[3, 1, 1])
    # node18 = Node(id=18, parents=None, children=[node20], core_speed=[13, 9, 2], cloud_speed=[3, 1, 1])
    # node17 = Node(id=17, parents=None, children=[node20], core_speed=[9, 3, 3], cloud_speed=[3, 1, 1])
    # node16 = Node(id=16, parents=None, children=[node19], core_speed=[9, 7, 3], cloud_speed=[3, 1, 1])
    # node15 = Node(id=15, parents=None, children=[node19], core_speed=[13, 4, 2], cloud_speed=[3, 1, 1])
    # node14 = Node(id=14, parents=None, children=[node18, node19], core_speed=[12, 11, 4], cloud_speed=[3, 1, 1])
    # node13 = Node(id=13, parents=None, children=[node17, node18], core_speed=[11, 3, 2], cloud_speed=[3, 1, 1])
    # node12 = Node(id=12, parents=None, children=[node17], core_speed=[12, 8, 4], cloud_speed=[3, 1, 1])
    # node11 = Node(id=11, parents=None, children=[node15, node16], core_speed=[12, 3, 3], cloud_speed=[3, 1, 1])
    # node10 = Node(id=10, parents=None, children=[node11,node15], core_speed=[7, 4, 2], cloud_speed=[3, 1, 1])
    # node9 = Node(id=9, parents=None, children=[node13,node14], core_speed=[5, 3, 2], cloud_speed=[3, 1, 1])
    # node8 = Node(id=8, parents=None, children=[node12,node13], core_speed=[6, 4, 2], cloud_speed=[3, 1, 1])
    # node7 = Node(id=7, parents=None, children=[node12], core_speed=[8, 5, 3], cloud_speed=[3, 1, 1])
    # node6 = Node(id=6, parents=None, children=[node10,node11], core_speed=[7, 6, 4], cloud_speed=[3, 1, 1])
    # node5 = Node(id=5, parents=None, children=[node9,node10], core_speed=[5, 4, 2], cloud_speed=[3, 1, 1])
    # node4 = Node(id=4, parents=None, children=[node8,node9], core_speed=[7, 5, 3], cloud_speed=[3, 1, 1])
    # node3 = Node(id=3, parents=None, children=[node7, node8], core_speed=[6, 5, 4], cloud_speed=[3, 1, 1])
    # node2 = Node(id=2, parents=None, children=[node7,node8], core_speed=[8, 6, 5], cloud_speed=[3, 1, 1])
    # node1 = Node(id=1, parents=None, children=[node7], core_speed=[9, 7, 5], cloud_speed=[3, 1, 1])
    # node1.parents = []
    # node2.parents = []
    # node3.parents = []
    # node4.parents = []
    # node5.parents = []
    # node6.parents = []
    # node7.parents = [node1,node2,node3]
    # node8.parents = [node3, node4]
    # node9.parents = [node4,node5]
    # node10.parents = [node5, node6]
    # node11.parents = [node6, node10]
    # node12.parents = [node7, node8]
    # node13.parents = [node8, node9]
    # node14.parents = [node9, node10]
    # node15.parents = [node10, node11]
    # node16.parents = [node11]
    # node17.parents = [node12, node13]
    # node18.parents = [node13, node14]
    # node19.parents = [node14, node15,node16]
    # node20.parents = [node17, node18,node19]

    # node_list = [node1, node2, node3, node4, node5, node6, node7, node8, node9, node10,node11,node12,node13,node14,node15,node16,node17,node18,node19,node20]
    
    # Test 5
    # node20 = Node(id=20, parents=None, children=[], core_speed=[12, 5, 4], cloud_speed=[3, 1, 1])
    # node19 = Node(id=19, parents=None, children=[], core_speed=[10, 5, 3], cloud_speed=[3, 1, 1])
    # node18 = Node(id=18, parents=None, children=[], core_speed=[13, 9, 2], cloud_speed=[3, 1, 1])
    # node17 = Node(id=17, parents=None, children=[], core_speed=[9, 3, 3], cloud_speed=[3, 1, 1])
    # node16 = Node(id=16, parents=None, children=[node19], core_speed=[9, 7, 3], cloud_speed=[3, 1, 1])
    # node15 = Node(id=15, parents=None, children=[node19], core_speed=[13, 4, 2], cloud_speed=[3, 1, 1])
    # node14 = Node(id=14, parents=None, children=[node18, node19], core_speed=[12, 11, 4], cloud_speed=[3, 1, 1])
    # node13 = Node(id=13, parents=None, children=[node17, node18], core_speed=[11, 3, 2], cloud_speed=[3, 1, 1])
    # node12 = Node(id=12, parents=None, children=[node17], core_speed=[12, 8, 4], cloud_speed=[3, 1, 1])
    # node11 = Node(id=11, parents=None, children=[node15, node16], core_speed=[12, 3, 3], cloud_speed=[3, 1, 1])
    # node10 = Node(id=10, parents=None, children=[node11,node15], core_speed=[7, 4, 2], cloud_speed=[3, 1, 1])
    # node9 = Node(id=9, parents=None, children=[node13,node14], core_speed=[5, 3, 2], cloud_speed=[3, 1, 1])
    # node8 = Node(id=8, parents=None, children=[node12,node13], core_speed=[6, 4, 2], cloud_speed=[3, 1, 1])
    # node7 = Node(id=7, parents=None, children=[node12], core_speed=[8, 5, 3], cloud_speed=[3, 1, 1])
    # node6 = Node(id=6, parents=None, children=[node10,node11], core_speed=[7, 6, 4], cloud_speed=[3, 1, 1])
    # node5 = Node(id=5, parents=None, children=[node9,node10], core_speed=[5, 4, 2], cloud_speed=[3, 1, 1])
    # node4 = Node(id=4, parents=None, children=[node8,node9], core_speed=[7, 5, 3], cloud_speed=[3, 1, 1])
    # node3 = Node(id=3, parents=None, children=[node7, node8], core_speed=[6, 5, 4], cloud_speed=[3, 1, 1])
    # node2 = Node(id=2, parents=None, children=[node7,node8], core_speed=[8, 6, 5], cloud_speed=[3, 1, 1])
    # node1 = Node(id=1, parents=None, children=[node7], core_speed=[9, 7, 5], cloud_speed=[3, 1, 1])
    # node1.parents = []
    # node2.parents = []
    # node3.parents = []
    # node4.parents = []
    # node5.parents = []
    # node6.parents = []
    # node7.parents = [node1,node2,node3]
    # node8.parents = [node3, node4]
    # node9.parents = [node4,node5]
    # node10.parents = [node5, node6]
    # node11.parents = [node6, node10]
    # node12.parents = [node7, node8]
    # node13.parents = [node8, node9]
    # node14.parents = [node9, node10]
    # node15.parents = [node10, node11]
    # node16.parents = [node11]
    # node17.parents = [node12, node13]
    # node18.parents = [node13, node14]
    # node19.parents = [node14, node15,node16]
    # node20.parents = [node12]

    # node_list = [node1, node2, node3, node4, node5, node6, node7, node8, node9, node10,node11,node12,node13,node14,node15,node16,node17,node18,node19,node20]
    
    start = time.perf_counter()

    #initial scheduling 
    primary_assignment(node_list)
    task_prioritizing(node_list)
    sequence = execution_unit_selection(node_list)

    #Plot for initial scheduling 
    tasksForPlotting = []
    for node in node_list:
        if node.is_core ==0:
            tasksForPlotting.append({"node id":node.id, 
                                     "assignment": node.assignment+1, 
                                     "cloud start_time": node.cloud_ready_time, 
                                     "cloud finish_time": node.cloud_ready_time+node.cloud_speed[1],
                                     "ws start_time": node.wireless_sending_ready_time,
                                     "ws finish_time": node.wireless_sending_ready_time+node.cloud_speed[0], 
                                     "wr start_time": node.wireless_recieving_ready_time, 
                                     "wr finish_time": node.wireless_recieving_ready_time+node.cloud_speed[2]})
        else:
            tasksForPlotting.append({"node id":node.id, "assignment": node.assignment +1, "local start_time": node.start_time[node.assignment], "local finish_time": node.start_time[node.assignment]+node.core_speed[node.assignment]})



    
    #Total time and energy at the end of initial scheduling 
    T_init_pre_kernel = total_T(node_list)
    T_init= T_init_pre_kernel
    E_init_pre_kernel = total_E(node_list, [1, 2, 4, 0.5])
    E_init= E_init_pre_kernel
    print("INITIAL TIME: ", T_init_pre_kernel)
    print("INITIAL ENERGY:", E_init_pre_kernel)

    for task in tasksForPlotting:
        print(task)
    
    draw_schedule(tasksForPlotting)

    #############################################
    # start outer loop
    #############################################
    iter_num = 0
    while iter_num < 100:
        # Start of an optimization iteration. The loop will run for a maximum of 100 iterations.
        print("-----" * 20)
        print("iter: ", iter_num)  # Print the current iteration number.
        
        # Calculate and print the total time and energy at the start of this iteration.
        T_init = total_T(node_list)
        E_init = total_E(node_list, [1, 2, 4, 0.5])
        print(f"initial time: {T_init}")
        print(f"initial energy: {E_init}")
        print("-----" * 20)

        # Initialize migration choices for each node.
        migeff_ratio_choice = [[] for i in range(len(node_list))]
        for i in range(len(node_list)):
            if node_list[i].assignment == 3:  # If the node is currently assigned to the cloud.
                current_row_id = node_list[i].id - 1
                current_row_value = [1] * 4  # Mark all resources (4 in total) as possible migration targets.
                migeff_ratio_choice[current_row_id] = current_row_value
            else:  # For nodes not on the cloud.
                current_row_id = node_list[i].id - 1
                current_row_value = [0] * 4  # Initially mark no resource as a target.
                current_row_value[node_list[i].assignment] = 1  # Mark the current resource as a target.
                migeff_ratio_choice[current_row_id] = current_row_value

        # Set a maximum time constraint for the schedule.
        # 1.5 * T_initial
        T_max_constraint = T_init_pre_kernel *1.5 

        # Initialize a table to store the results (time and energy) of each potential migration.
        result_table = [[(-1, -1) for j in range(4)] for i in range(len(node_list))]

        # Evaluate each potential migration for every node.
        for n in range(len(migeff_ratio_choice)):  # Iterate over each node.
            nth_row = migeff_ratio_choice[n]
            for k in range(len(nth_row)):  # Iterate over each possible resource.
                if nth_row[k] == 1:
                    continue  # Skip if the node is already assigned to this resource.
                
                # Create copies of the current sequence and node list for testing the migration.
                seq_copy = deepcopy(sequence)
                nodes_copy = deepcopy(node_list)
                
                # Apply the migration and run the kernel scheduling algorithm.
                seq_copy = new_sequence(nodes_copy, n+1, k, seq_copy)

                kernel_algorithm(nodes_copy, seq_copy)

                # Calculate and store the total time and energy for this migration scenario.
                current_T = total_T(nodes_copy)
                current_E = total_E(nodes_copy)
                result_table[n][k] = (current_T, current_E)

        # Initialize variables to track the best migration found in this iteration.
        n_best = -1
        k_best = -1
        T_best = T_init
        E_best = E_init
        eff_ratio_best = -1

        # Find the optimal migration option based on an efficiency ratio.
        for i in range(len(result_table)):
            for j in range(len(result_table[i])):
                val = result_table[i][j]
                if val == (-1, -1) or val[0] > T_max_constraint:
                    continue  # Skip invalid or infeasible migrations.

                # Calculate the efficiency ratio for the current migration.
                eff_ratio = (E_best - val[1]) / abs(val[0] - T_best + 0.00005) #Prevents division by 0
                if eff_ratio > eff_ratio_best:  # If this migration is more efficient, update the best values.
                    eff_ratio_best = eff_ratio
                    n_best = i
                    k_best = j

        # Check if a better migration option was found.
        if n_best == -1 and k_best == -1:
            break  # Exit the loop if no better option is found.

        # Apply the best migration found.
        n_best += 1
        k_best += 1
        T_best, E_best = result_table[n_best-1][k_best-1]
        print("\ncurrent migration: task:{}, k: {}, total time: {}, total energy: {}".format(n_best, k_best, T_best, E_best))

        # Update the task sequence to reflect the best migration found.
        print("\nupdate after current outer loop")
        sequence = new_sequence(node_list, n_best, k_best-1, sequence)

        kernel_algorithm(node_list, sequence)

        # Print the updated sequence and calculate the new total time and energy.
        for s in sequence:
            print([i for i in s])
        T_current = total_T(node_list)
        E_current = total_E(node_list, [1, 2, 4, 0.5])

        # Calculate the difference in energy from the initial state.
        E_diff = E_init - E_current
        T_diff = abs(T_current - T_init)

        # Increment the iteration counter.
        iter_num += 1

        # Print the current total time and energy after the migration.
        print(f"\npost migration time: {T_current} ")
        print(f"post migration energy: {E_current}" )

        # Break the loop if the energy difference is minimal, indicating little to no improvement.
        if E_diff <= 1:
            break


    tasksForPlotting = []

    print("\n\nRESCHEDULING FINISHED\n\n")


    for node in node_list:
        if node.is_core == True:
            tasksForPlotting.append({"node id":node.id, "assignment": node.assignment + 1, "local start_time": node.start_time[node.assignment], "local finish_time": node.start_time[node.assignment]+node.core_speed[node.assignment]})
        else:
            tasksForPlotting.append({"node id":node.id, 
                                     "assignment": node.assignment + 1, 
                                     "cloud start_time": node.cloud_ready_time, 
                                     "cloud finish_time": node.cloud_ready_time+node.cloud_speed[1],
                                     "ws start_time": node.start_time[3],
                                     "ws finish_time": node.start_time[3]+node.cloud_speed[0], 
                                     "wr start_time": node.wireless_recieving_ready_time, 
                                     "wr finish_time": node.wireless_recieving_ready_time+node.cloud_speed[2]})
                                     
    
    elapsed = (time.perf_counter() - start)

    for task in tasksForPlotting:
        print(task)

    print("\nTime to run on machine:", elapsed)
    print("final sequence: ")
    for s in sequence:
        print([i for i in s])

    T_final = total_T(node_list)
    E_final = total_E(node_list, [1, 2, 4, 0.5])
    print("\nINITIAL TIME: {}\nINITIAL ENERGY: {}\n".format(T_init_pre_kernel, E_init_pre_kernel))
    print("FINAL TIME: {}\nFINAL ENERGY: {}".format(T_final, E_final))


draw_schedule(tasksForPlotting)