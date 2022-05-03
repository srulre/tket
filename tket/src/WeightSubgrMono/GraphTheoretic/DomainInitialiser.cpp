// Copyright 2019-2022 Cambridge Quantum Computing
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "WeightSubgrMono/GraphTheoretic/DomainInitialiser.hpp"

#include <algorithm>

#include "Utils/Assert.hpp"
#include "WeightSubgrMono/Common/GeneralUtils.hpp"
#include "WeightSubgrMono/Common/SetIntersection.hpp"
#include "WeightSubgrMono/GraphTheoretic/FilterUtils.hpp"
#include "WeightSubgrMono/GraphTheoretic/NearNeighboursData.hpp"
#include "WeightSubgrMono/GraphTheoretic/NeighboursData.hpp"

namespace tket {
namespace WeightedSubgraphMonomorphism {

bool DomainInitialiser::full_initialisation(
    PossibleAssignments& possible_assignments,
    const NeighboursData& pattern_neighbours_data,
    NearNeighboursData& pattern_near_neighbours_data,
    const NeighboursData& target_neighbours_data,
    NearNeighboursData& target_near_neighbours_data, unsigned max_path_length) {
  return degree_sequence_initialisation(
             possible_assignments, pattern_neighbours_data,
             target_neighbours_data) &&

         distance_counts_reduction(
             possible_assignments, pattern_neighbours_data,
             pattern_near_neighbours_data, target_neighbours_data,
             target_near_neighbours_data, max_path_length);
}

bool DomainInitialiser::degree_sequence_initialisation(
    PossibleAssignments& possible_assignments,
    const NeighboursData& pattern_neighbours_data,
    const NeighboursData& target_neighbours_data) {
  possible_assignments.clear();
  for (const auto& entry : pattern_neighbours_data.get_map()) {
    possible_assignments[entry.first];
  }

  // Get the target degree sequences.

  // FIRST: the target vertex;
  // SECOND: its degree sequence.
  typedef std::pair<VertexWSM, std::vector<std::size_t>> DegSeqData;
  std::vector<DegSeqData> target_degree_sequences;

  const auto& target_map = target_neighbours_data.get_map();
  target_degree_sequences.reserve(target_map.size());
  for (const auto& t_entry : target_map) {
    const VertexWSM tv = t_entry.first;
    target_degree_sequences.emplace_back(
        tv, target_neighbours_data.get_sorted_degree_sequence_expensive(tv));
  }

  // Sort by decreasing sequence length.
  std::sort(
      target_degree_sequences.begin(), target_degree_sequences.end(),
      [](const DegSeqData& lhs, const DegSeqData& rhs) -> bool {
        return lhs.second.size() > rhs.second.size() ||
               // This is not crucial, it just ensures exact behaviour for
               // nonstable sorts:
               (lhs.second.size() == rhs.second.size() &&
                lhs.first < rhs.first);
      });

  std::vector<std::size_t> pattern_sequence;
  for (auto& entry : possible_assignments) {
    const auto& pv = entry.first;
    auto& domain = entry.second;
    pattern_sequence =
        pattern_neighbours_data.get_sorted_degree_sequence_expensive(pv);

    // Now, which target vertices have compatible degree sequences?
    for (const DegSeqData& deg_seq_data : target_degree_sequences) {
      const auto& target_sequence = deg_seq_data.second;
      if (target_sequence.size() < pattern_sequence.size()) {
        break;
      }
      if (FilterUtils::compatible_sorted_degree_sequences(
              pattern_sequence, target_sequence)) {
        domain.insert(deg_seq_data.first);
      }
    }
    if (domain.empty()) {
      return false;
    }
  }
  return true;
}

bool DomainInitialiser::distance_counts_reduction(
    PossibleAssignments& possible_assignments,
    const NeighboursData& pattern_neighbours_data,
    NearNeighboursData& pattern_near_neighbours_data,
    const NeighboursData& target_neighbours_data,
    NearNeighboursData& target_near_neighbours_data, unsigned max_path_length) {
  if (max_path_length <= 1) {
    // Neighbour counts are already included in degree sequences.
    return true;
  }
  // Easier to build them all up, then erase them later in a separate pass.
  // KEY: PV  VALUE:
  // std::map<VertexWSM, std::vector<VertexWSM>> impossible_assignments;
  std::vector<VertexWSM> tv_to_erase;

  for (auto& entry : possible_assignments) {
    const VertexWSM& pv = entry.first;
    auto& domain = entry.second;
    tv_to_erase.clear();
    for (VertexWSM tv : domain) {
      // If f is a valid monomorphism then
      //    dist(PV, PV')=d   ==>   dist(f(PV), f(PV')) <= d.
      // Now TV=f(PV) is the value to be checked;
      // so for each d, just count how many PV' are at that exact distance d,
      // and check that there are at least that many TV' with
      // dist(TV, TV') <= d.
      for (unsigned distance = 2; distance <= max_path_length; ++distance) {
        const auto number_of_pv_at_distance_d =
            pattern_near_neighbours_data.get_vertices_at_distance(pv, distance)
                .size();

        if (number_of_pv_at_distance_d > 0) {
          const auto number_of_tv_at_distance_le_d =
              target_near_neighbours_data.get_n_vertices_at_max_distance(
                  tv, distance);
          if (number_of_pv_at_distance_d > number_of_tv_at_distance_le_d) {
            tv_to_erase.push_back(tv);
            break;
          }
        }
      }
    }
    if (tv_to_erase.size() == domain.size()) {
      return false;
    }
    for (VertexWSM tv : tv_to_erase) {
      TKET_ASSERT(domain.erase(tv) == 1);
    }
  }
  return true;
}

}  // namespace WeightedSubgraphMonomorphism
}  // namespace tket
