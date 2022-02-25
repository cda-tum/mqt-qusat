OPENQASM 2.0; // for testing clifford circuits - without measurements
include "qelib1.inc"; //https://github.com/AgentANAKIN/Simon-s-Algorithm/blob/master/simon.qasm
qreg q[6];
creg c[6];
// This initializes 6 quantum registers and 6 classical registers.

h q[0];
h q[1];
h q[2];
// The first 3 qubits are put into superposition states.

//barrier q;
cx q[2], q[4];
x q[3];
cx q[2], q[3];
ccx q[0], q[1], q[3];
x q[0];
x q[1];
ccx q[0], q[1], q[3];
x q[0];
x q[1];
x q[3];
// This applies the secret structure: s=110.
