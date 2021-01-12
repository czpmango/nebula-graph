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
    auto* currentRoot = subplan.root;
    auto* agg = Aggregate::make(gctx->qctx,
                                currentRoot,
                                std::move(gctx->groupKeys_),
                                std::move(gctx->groupItems_));
    agg->setColNames(std::vector<std::string>(gctx->outputColumnNames_));
    if (gctx->needGenProject_) {
        // rewrite Expr which has inner aggExpr and push it up to Project.
        auto *project = Project::make(gctx->qctx, agg, gctx->projCols_);
        project->setInputVar(agg->outputVar());
        project->setColNames(gctx->projOutputColumnNames_);
        subplan.root = project;
    } else {
        subplan.root = agg;
    }
    subplan.tail = agg;

    return Status::OK();
}
}  // namespace graph
}  // namespace nebula
