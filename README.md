# Provably secure public key encryption with keyword search for data outsourcing in cloud environments 

Cite the following article to use this codebase in your work.

[Provably secure public key encryption with keyword search for data outsourcing in cloud environments](https://www.sciencedirect.com/science/article/pii/S1383762123000553)

## Instructions to run project

### Build Project
- Install `cmake` on your system if not already installed.
- Run `cmake -S . -B cmake-build-debug`
- Run `cmake --build cmake-build-debug`
- Create a directory `temp` inside the project root.

### Run Project
Run `cd cmake-build-debug`

#### Server command
`./server`

#### Data Owner command
`./data_owner`

#### Data Consumer command
`./data_consumer`

## NOTE:
The data owner and data consumer files needs to be run twice when executing the project for the first time.
This is required in order to generated the keys. For simplicity we have not used a key distribution server and instead are using 
the `temp` directory to share the keys with one another.