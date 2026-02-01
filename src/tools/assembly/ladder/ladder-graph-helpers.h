// ladder-graph-helpers.h - Graph Traversal and Connection Helper Functions
// Compiles graph-based ladder logic (nodes + connections) to Siemens-style STL
//
// Copyright (c) 2026 J.Vovk
//
// This file is part of VovkPLCRuntime.
//
// VovkPLCRuntime is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// VovkPLCRuntime is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with VovkPLCRuntime.  If not, see <https://www.gnu.org/licenses/>.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#ifdef __WASM__

#include "ladder-types.h"
#include "ladder-node-types.h"

// ============================================================================
// Graph Helper Functions
// These are utility functions for navigating the ladder graph structure
// ============================================================================

// Find a node by its ID, returns index or -1 if not found
inline int ladderFindNodeById(const LadderGraphNode* nodes, int node_count, const char* nodeId) {
    for (int i = 0; i < node_count; i++) {
        if (ladderStrEqI(nodes[i].id, nodeId)) return i;
    }
    return -1;
}

// Check if a node has any incoming connections
inline bool ladderNodeHasInputs(
    const LadderGraphNode* nodes, int node_count,
    const LadderConnection* connections, int connection_count,
    int nodeIdx
) {
    if (nodeIdx < 0 || nodeIdx >= node_count) return false;
    for (int c = 0; c < connection_count; c++) {
        for (int d = 0; d < connections[c].dest_count; d++) {
            if (ladderStrEqI(connections[c].destinations[d], nodes[nodeIdx].id)) {
                return true;
            }
        }
    }
    return false;
}

// Check if a node has any outgoing connections
inline bool ladderNodeHasOutputs(
    const LadderGraphNode* nodes, int node_count,
    const LadderConnection* connections, int connection_count,
    int nodeIdx
) {
    if (nodeIdx < 0 || nodeIdx >= node_count) return false;
    for (int c = 0; c < connection_count; c++) {
        for (int s = 0; s < connections[c].source_count; s++) {
            if (ladderStrEqI(connections[c].sources[s], nodes[nodeIdx].id)) {
                return true;
            }
        }
    }
    return false;
}

// Find all connections where this node is a destination (inputs to this node)
inline void ladderGetInputConnections(
    const LadderConnection* connections, int connection_count,
    const char* nodeId, int* connIndices, int& count, int maxCount
) {
    count = 0;
    for (int c = 0; c < connection_count && count < maxCount; c++) {
        for (int d = 0; d < connections[c].dest_count; d++) {
            if (ladderStrEqI(connections[c].destinations[d], nodeId)) {
                connIndices[count++] = c;
                break;
            }
        }
    }
}

// Find all connections where this node is a source (outputs from this node)
inline void ladderGetOutputConnections(
    const LadderConnection* connections, int connection_count,
    const char* nodeId, int* connIndices, int& count, int maxCount
) {
    count = 0;
    for (int c = 0; c < connection_count && count < maxCount; c++) {
        for (int s = 0; s < connections[c].source_count; s++) {
            if (ladderStrEqI(connections[c].sources[s], nodeId)) {
                connIndices[count++] = c;
                break;
            }
        }
    }
}

// Get all source node indices feeding into a given node
inline void ladderGetSourceNodes(
    const LadderGraphNode* nodes, int node_count,
    const LadderConnection* connections, int connection_count,
    const char* nodeId, int* nodeIndices, int& count, int maxCount
) {
    count = 0;
    int connIndices[LADDER_MAX_CONNECTIONS];
    int connCount;
    ladderGetInputConnections(connections, connection_count, nodeId, connIndices, connCount, LADDER_MAX_CONNECTIONS);

    for (int c = 0; c < connCount; c++) {
        const LadderConnection& conn = connections[connIndices[c]];
        for (int s = 0; s < conn.source_count && count < maxCount; s++) {
            int idx = ladderFindNodeById(nodes, node_count, conn.sources[s]);
            if (idx >= 0) {
                // Check if already added
                bool found = false;
                for (int i = 0; i < count; i++) {
                    if (nodeIndices[i] == idx) { found = true; break; }
                }
                if (!found) nodeIndices[count++] = idx;
            }
        }
    }
}

// Get all destination node indices this node feeds into
inline void ladderGetDestNodes(
    const LadderGraphNode* nodes, int node_count,
    const LadderConnection* connections, int connection_count,
    const char* nodeId, int* nodeIndices, int& count, int maxCount
) {
    count = 0;
    int connIndices[LADDER_MAX_CONNECTIONS];
    int connCount;
    ladderGetOutputConnections(connections, connection_count, nodeId, connIndices, connCount, LADDER_MAX_CONNECTIONS);

    for (int c = 0; c < connCount; c++) {
        const LadderConnection& conn = connections[connIndices[c]];
        for (int d = 0; d < conn.dest_count && count < maxCount; d++) {
            int idx = ladderFindNodeById(nodes, node_count, conn.destinations[d]);
            if (idx >= 0) {
                bool found = false;
                for (int i = 0; i < count; i++) {
                    if (nodeIndices[i] == idx) { found = true; break; }
                }
                if (!found) nodeIndices[count++] = idx;
            }
        }
    }
}

// Sort nodes by x,y coordinates (left-to-right, top-to-bottom)
inline void ladderSortNodesByPosition(LadderGraphNode* nodes, int node_count) {
    // Simple bubble sort for embedded compatibility
    for (int i = 0; i < node_count - 1; i++) {
        for (int j = 0; j < node_count - i - 1; j++) {
            bool swap = false;
            if (nodes[j].x > nodes[j + 1].x) {
                swap = true;
            } else if (nodes[j].x == nodes[j + 1].x && nodes[j].y > nodes[j + 1].y) {
                swap = true;
            }
            if (swap) {
                LadderGraphNode tmp = nodes[j];
                nodes[j] = nodes[j + 1];
                nodes[j + 1] = tmp;
            }
        }
    }

    // Assign order indices
    for (int i = 0; i < node_count; i++) {
        nodes[i].order_index = i;
    }
}

// Find starting nodes (nodes with no input connections)
inline void ladderFindStartNodes(
    const LadderGraphNode* nodes, int node_count,
    const LadderConnection* connections, int connection_count,
    int* startNodes, int& count, int maxCount
) {
    count = 0;
    for (int i = 0; i < node_count && count < maxCount; i++) {
        int inputConns[LADDER_MAX_CONNECTIONS];
        int inputCount;
        ladderGetInputConnections(connections, connection_count, nodes[i].id, inputConns, inputCount, LADDER_MAX_CONNECTIONS);
        if (inputCount == 0) {
            startNodes[count++] = i;
        }
    }
}

// Find ending nodes (nodes with no output connections)
inline void ladderFindEndNodes(
    const LadderGraphNode* nodes, int node_count,
    const LadderConnection* connections, int connection_count,
    int* endNodes, int& count, int maxCount
) {
    count = 0;
    for (int i = 0; i < node_count && count < maxCount; i++) {
        int outputConns[LADDER_MAX_CONNECTIONS];
        int outputCount;
        ladderGetOutputConnections(connections, connection_count, nodes[i].id, outputConns, outputCount, LADDER_MAX_CONNECTIONS);
        if (outputCount == 0) {
            endNodes[count++] = i;
        }
    }
}

// Find all output nodes (coils, operation blocks)
inline void ladderFindOutputNodes(
    const LadderGraphNode* nodes, int node_count,
    int* outputNodes, int& count, int maxCount
) {
    count = 0;
    for (int i = 0; i < node_count && count < maxCount; i++) {
        if (ladderIsOutputNode(nodes[i].type)) {
            outputNodes[count++] = i;
        }
    }
}

// Sort source indices by y position (top to bottom)
inline void ladderSortSourcesByPosition(const LadderGraphNode* nodes, int* sourceIndices, int sourceCount) {
    // Simple bubble sort
    for (int i = 0; i < sourceCount - 1; i++) {
        for (int j = 0; j < sourceCount - i - 1; j++) {
            int yA = nodes[sourceIndices[j]].y;
            int yB = nodes[sourceIndices[j + 1]].y;
            // If same y, sort by x
            if (yA > yB || (yA == yB && nodes[sourceIndices[j]].x > nodes[sourceIndices[j + 1]].x)) {
                int tmp = sourceIndices[j];
                sourceIndices[j] = sourceIndices[j + 1];
                sourceIndices[j + 1] = tmp;
            }
        }
    }
}

#endif // __WASM__
