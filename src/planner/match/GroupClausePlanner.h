/* Copyright (c) 2020 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#ifndef PLANNER_MATCH_GROUPCLAUSEPLANNER_H_
#define PLANNER_MATCH_GROUPCLAUSEPLANNER_H_

#include "planner/match/CypherClausePlanner.h"

namespace nebula {
namespace graph {
/*
 * The GroupClausePlanner was designed to generate plan for agg in Return clause;
 */
class GroupClausePlanner final : public CypherClausePlanner {
public:
    GroupClausePlanner() = default;

    StatusOr<SubPlan> transform(CypherClauseContextBase* clauseCtx) override;

    Status buildGroup(GroupClauseContext* gctx, SubPlan& subplan);
};
}  // namespace graph
}  // namespace nebula
#endif  // PLANNER_MATCH_GROUPCLAUSEPLANNER_H_
