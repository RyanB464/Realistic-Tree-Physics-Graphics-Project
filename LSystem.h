#pragma once

#include <string>
#include <vector>

struct Branch;
struct Leaf;

namespace LSystem {
    // Generate an L-system string from an axiom and iteration count
    std::string generate(const std::string& axiom, int iterations);

    // Build branches and leaves from an L-system string.
    // - branches and leaves are appended to by this function.
    // - baseLength and baseRadius are the starting geometry parameters.
    void buildTree(const std::string& seq,
                   std::vector<Branch>& branches,
                   std::vector<Leaf>& leaves,
                   float baseLength,
                   float baseRadius);
}
