/* Copyright (c) 2020 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#ifndef EXECUTOR_QUERY_FILTEREXECUTOR_H_
#define EXECUTOR_QUERY_FILTEREXECUTOR_H_

#include "executor/Executor.h"

namespace nebula {
namespace graph {

class FilterExecutor final : public Executor {
public:
    FilterExecutor(const PlanNode *node, QueryContext *qctx)
        : Executor("FilterExecutor", node, qctx) {}

    folly::Future<Status> execute() override;
};

}   // namespace graph
}   // namespace nebula

#endif   // EXECUTOR_QUERY_FILTEREXECUTOR_H_
