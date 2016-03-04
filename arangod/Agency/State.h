////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2016 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Kaveh Vahedipour
////////////////////////////////////////////////////////////////////////////////

#ifndef __ARANGODB_CONSENSUS_STATE__
#define __ARANGODB_CONSENSUS_STATE__


#include "AgencyCommon.h"
#include "State.h"

#include <Basics/Thread.h>
#include <Cluster/ClusterComm.h>
#include <velocypack/vpack.h>

#include <cstdint>
#include <functional>


//using namespace arangodb::velocypack;

class Slice {};

namespace arangodb {
namespace consensus {

class Agent;

/**
 * @brief State replica
 */
class State {
  
public:
  
  /**
   * @brief Default constructor
   */
  State ();
  
  /**
   * @brief Default Destructor
   */
  virtual ~State();
  
  /**
   * @brief Append log entry
   */
  void append (query_t const& query);

  /**
   * @brief Log entries (leader)
   */
  std::vector<index_t> log (query_t const& query, term_t term, id_t lid);

  /**
   * @brief Log entry follower
   */
  void log (query_t const& query, index_t, term_t term, id_t lid);

  /**
   * @brief Find entry at index with term
   */
  bool findit (index_t index, term_t term);

  /**
   * @brief Collect all from index on
   */
  collect_ret_t collectFrom (index_t index);

  /**
   * @brief log entry at index i
   */
  log_t const& operator[](index_t) const;

  /**
   * @brief last log entry
   */
  log_t const& lastLog () const;
  
private:
  
  /**
   * @brief Save currentTerm, votedFor, log entries
   */
  bool save (arangodb::velocypack::Builder const&);

  /**
   * @brief Load persisted data from above or start with empty log
   */
  bool load ();

  mutable arangodb::Mutex _logLock;          /**< @brief Mutex for modifying _log */
  std::vector<log_t> _log;          /**< @brief  State entries */
  
};

}}

#endif
