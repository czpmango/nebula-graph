/* Copyright (c) 2020 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#include "planner/match/ReturnClausePlanner.h"

#include "planner/Query.h"
#include "planner/match/MatchSolver.h"
#include "planner/match/OrderByClausePlanner.h"
#include "planner/match/PaginationPlanner.h"
#include "planner/match/GroupClausePlanner.h"
#include "planner/match/SegmentsConnector.h"
#include "visitor/RewriteMatchLabelVisitor.h"

namespace nebula {
namespace graph {
StatusOr<SubPlan> ReturnClausePlanner::transform(CypherClauseContextBase* clauseCtx) {
    if (clauseCtx->kind != CypherClauseKind::kReturn) {
        return Status::Error("Not a valid context for ReturnClausePlanner.");
    }
    auto* returnClauseCtx = static_cast<ReturnClauseContext*>(clauseCtx);

    SubPlan returnPlan;
    NG_RETURN_IF_ERROR(buildReturn(returnClauseCtx, returnPlan));
    return returnPlan;
}

Status ReturnClausePlanner::buildReturn(ReturnClauseContext* rctx, SubPlan& subPlan) {
    std::vector<std::string> colNames;
    PlanNode* current = nullptr;

    for (auto* col : rctx->yieldColumns->columns()) {
        if (col->alias() != nullptr) {
            colNames.emplace_back(*col->alias());
        } else {
            colNames.emplace_back(col->expr()->toString());
        }
    }

    auto* groupCtx = rctx->group.get();
    if (groupCtx) {
        auto groupPlan = std::make_unique<GroupClausePlanner>()->transform(groupCtx);
        NG_RETURN_IF_ERROR(groupPlan);
        auto plan = std::move(groupPlan).value();
        subPlan.tail = plan.tail;
        current = plan.root;
    } else {
        auto* project = Project::make(rctx->qctx,
                                      nullptr,
                                      const_cast<YieldColumns*>(rctx->yieldColumns));
        project->setColNames(std::move(colNames));
        subPlan.tail = project;
        current = project;
    }

    if (rctx->distinct) {
        auto* dedup = Dedup::make(rctx->qctx, current);
        dedup->setInputVar(current->outputVar());
        dedup->setColNames(current->colNames());
        current = dedup;
    }

    subPlan.root = current;

    if (rctx->order != nullptr) {
        auto orderPlan = std::make_unique<OrderByClausePlanner>()->transform(rctx->order.get());
        NG_RETURN_IF_ERROR(orderPlan);
        auto plan = std::move(orderPlan).value();
        SegmentsConnector::addInput(plan.tail, subPlan.root, true);
        subPlan.root = plan.root;
    }

    if (rctx->pagination != nullptr &&
        (rctx->pagination->skip != 0 ||
         rctx->pagination->limit != std::numeric_limits<int64_t>::max())) {
        auto paginationPlan =
            std::make_unique<PaginationPlanner>()->transform(rctx->pagination.get());
        NG_RETURN_IF_ERROR(paginationPlan);
        auto plan = std::move(paginationPlan).value();
        SegmentsConnector::addInput(plan.tail, subPlan.root, true);
        subPlan.root = plan.root;
    }

    VLOG(1) << "return root: " << subPlan.root->outputVar()
            << " colNames: " << folly::join(",", subPlan.root->colNames());

    return Status::OK();
}

}  // namespace graph
}  // namespace nebula
