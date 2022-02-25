OPENQASM 2.0; // for testing clifford circuit - without measurements
include "qelib1.inc";

qreg q[2];		// create a quantum register with 2 qubits
creg c[2]; 		// create a classical register with 2 bits
h q[0];  		// perform Hadamard gate on one qubit
cx q[0],q[1];  	// perform control-not gate on both qubits