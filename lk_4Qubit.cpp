 //===----------------------------------------------------------------------===//
// INTEL CONFIDENTIAL
//
// Copyright 2021-2022 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//===----------------------------------------------------------------------===//

/// Quantum intrinsic function definitions
#include <clang/Quantum/quintrinsics.h>

/// Quantum Runtime Library APIs
#include <quantum.hpp>

#define M_PI 3.14159265358979323846

/// Define our quantum resources
const int N = 2; // handle for number of qubits
qbit q[N];       // register of qubits to operate on
cbit c[N];       // register of measurement readouts

/// prepare each of the qubits in q to the |0> state
quantum_kernel void prepare_all() {
    PrepZ(q[0]);
    PrepZ(q[1]);
}

/// measure each qubit in q and store each measurement outcome in c
quantum_kernel void measure_all() {
    MeasZ(q[0], c[0]);
    MeasZ(q[1], c[1]);
}

quantum_kernel void gates(double angle1, double angle2) {
    H(q[0]);
    CNOT(q[0], q[1]);
    RY(q[0], angle1);
    RY(q[1], angle2);
}


quantum_kernel double getCor(iqsdk::QssMap<double> probability_map) {
    double total_probability(0);
    for (auto const & key_value : probability_map) {
        total_probability += key_value.second;
    }
    return 2 * total_probability - 1;
}

int main() {
    /// Setup quantum device with N qubits and noiseless simulation
    
    iqsdk::IqsConfig iqs_config(N, "noiseless");
    iqsdk::FullStateSimulator iqs_device(iqs_config);
    if (iqsdk::QRT_ERROR_SUCCESS != iqs_device.ready()) {
        return 1;
    }
    
    
    // QBit reference wrapper
    std::vector<std::reference_wrapper<qbit>> qids;
    for (int id = 0; id < N ; ++id) {
        qids.push_back(std::ref(q[id]));
    }
    
    // Angles for violating Bell's Inequality
    double a1 = 0;
    double b1 = M_PI/8;
    double a2 = M_PI/4;
    double b2 = 3*M_PI/8;
    
    // E/Correlation values, aka Average of A(θ,λ)*B(Φ,λ)
    double cor1 = 0;
    double cor2 = 0;
    double cor3 = 0;
    double cor4 = 0;
    
    
    
    // |00> and |11> state basis put in a vector (essentially a list)
    // These are used to tell the program which probabilities of the entangled system to output
    // Thus, this makes it output the probability of measuring |00> and of measuring |11>.
    iqsdk::QssIndex state_a("|00>");
    iqsdk::QssIndex state_b("|11>");
    std::vector<iqsdk::QssIndex> bases;
    bases.push_back(state_a);
    bases.push_back(state_b);
    
    
    iqsdk::QssMap<double> probability_map;

    // Calculates Correlations, aka E(a,b), E(a, b'), E(a, b'), E(a', b').

    prepare_all(); // Prepares all qbits in Z direction
    gates(a1, b1); // Applies quantum gates, with angles a1 and b1 for rotations of q0 and q1
    probability_map = iqs_device.getProbabilities(qids, bases); // Makes probability map of |00> and |11>
    iqs_device.displayProbabilities(probability_map); // Outputs these probabilities
    cor1 = getCor(probability_map); // uses getCor to calculate correlation w/ this map
    
    prepare_all();
    gates(a1, b2);
    probability_map = iqs_device.getProbabilities(qids, bases);
    iqs_device.displayProbabilities(probability_map);
    cor2 = getCor(probability_map);
    
    prepare_all();
    gates(a2, b1);
    probability_map = iqs_device.getProbabilities(qids, bases);
    iqs_device.displayProbabilities(probability_map);
    cor3 = getCor(probability_map);
    
    prepare_all();
    gates(a2, b2);
    probability_map = iqs_device.getProbabilities(qids, bases);
    iqs_device.displayProbabilities(probability_map);
    cor4 = getCor(probability_map);
    
    // Outputs results
    std::cout << "E(a,b)=" << cor1 << ", E(a',b)=" << cor2 << ", E(a,b')=" << cor3 << ", E(a',b')=" << cor4 << std::endl;
    std::cout << "S=E(a,b)-E(a',b)+E(a,b')+E(a',b')" << std::endl;
    std::cout << "If |S|>2, QM predicts violation of Bell's Inequality:" << std::endl;
    std::cout << "S=" << cor1 - cor2 + cor3 + cor4 << std::endl;
    
    
    return 0;
}
