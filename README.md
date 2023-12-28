# Energy and Performance-Aware Task Scheduling Algorithm Implementation (Final Project for EECE 7205)

## Overview
This script is an implementation of the task scheduling algorithm described in the paper "Energy and Performance-Aware Task Scheduling in a Mobile Cloud Computing Environment" by Xue Lin, Yanzhi Wang, Qing Xie, and Massoud Pedram from the Department of Electrical Engineering at the University of Southern California. The implementation was undertaken by Ethan Mandel.

## Description
The algorithm aims to efficiently schedule tasks in a mobile cloud computing environment by balancing the dual objectives of minimizing energy consumption and execution time. It does so by leveraging a model that accounts for both local and cloud processing capabilities and times, as well as energy costs associated with task execution in these environments.

## Implementation Details
The script defines a `Node` class representing a task with attributes like ID, parent and child tasks, core and cloud processing speeds, and scheduling times. The main algorithm involves initializing the tasks, setting priorities, and then iteratively scheduling these tasks on either local cores or cloud based on a series of calculated metrics aiming for an optimal balance of energy and performance.

### Key Functions:
- `primary_assignment()`: Classifies tasks into local or cloud execution based on initial conditions.
- `task_prioritizing()`: Assigns priority scores to tasks.
- `execution_unit_selection()`: Schedules tasks on either local cores or cloud based on their priorities.
- `kernel_algorithm()`: Performs scheduling adjustments to improve energy and performance metrics.
- `draw_schedule()`: Visualizes the task scheduling on cores and cloud.

## Usage
1. Ensure you have Python and necessary libraries (`matplotlib` for visualization) installed.
2. Define the tasks (nodes) by instantiating `Node` objects with appropriate parameters.
3. Call `primary_assignment()`, `task_prioritizing()`, and `execution_unit_selection()` with the list of nodes.
4. To optimize the initial schedule, run `kernel_algorithm()`.
5. Visualize the schedule using `draw_schedule()`.

## Note
- The script is meant to be a direct translation of the scheduling strategy as proposed in the cited paper, and efficiency or scalability for large-scale systems isn't guaranteed.
- The user is encouraged to adjust parameters and extend the model to fit specific needs or constraints of different scheduling environments.

## Credits
- **Algorithm Concept**: Xue Lin, Yanzhi Wang, Qing Xie, Massoud Pedram
- **Implementation**: Ethan Mandel
