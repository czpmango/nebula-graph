/* Copyright (c) 2020 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#include "planner/match/GroupClausePlanner.h"

#include "planner/Query.h"

namespace nebula {
namespace graph {
StatusOr<SubPlan> GroupClausePlanner::transform(CypherClauseContextBase* clauseCtx) {
    if (clauseCtx->kind != CypherClauseKind::kGroup) {
        return Status::Error("Not a valid context for GroupClausePlanner.");
    }
    auto* groupCtx = static_cast<GroupClauseContext*>(clauseCtx);

    SubPlan groupPlan;
    NG_RETURN_IF_ERROR(buildGroup(groupCtx, groupPlan));
    return groupPlan;
}

Status GroupClausePlanner::buildGroup(GroupClauseContext* gctx, SubPlan& subplan) {
    UNUSED(gctx);
    UNUSED(subplan);
    // auto* currentRoot = subplan.root;
    // auto* group = Sort::make(octx->qctx, currentRoot, octx->indexedOrderFactors);
    // subplan.root = group;
    // subplan.tail = group;
    return Status::OK();
}
}  // namespace graph
}  // namespace nebula
